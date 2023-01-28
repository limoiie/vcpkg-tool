#include <vcpkg/base/checks.h>
#include <vcpkg/base/messages.h>
#include <vcpkg/base/parse.h>
#include <vcpkg/base/util.h>

#include <vcpkg/packagespec.h>
#include <vcpkg/paragraphparser.h>

namespace vcpkg
{
    static constexpr StringLiteral SEP_TRIPLET_COMPILE_TRIPLET = "_";

    std::string FeatureSpec::to_string() const
    {
        std::string ret;
        this->to_string(ret);
        return ret;
    }
    void FeatureSpec::to_string(std::string& out) const
    {
        if (feature().empty()) return spec().to_string(out);
        Strings::append(out, port(), '[', feature(), "]:", spec().qualifier());
    }

    void FullPackageSpec::expand_fspecs_to(std::vector<FeatureSpec>& out) const
    {
        for (auto&& feature : features)
        {
            out.emplace_back(package_spec, feature);
        }
    }

    const std::string& PackageSpec::name() const { return this->m_name; }

    Optional<bin2sth::CompileTriplet> const& PackageSpec::compile_triplet() const { return this->m_compile_triplet; }

    Triplet PackageSpec::triplet() const { return this->m_triplet; }

    std::string PackageSpec::qualifier() const
    {
        std::string qualifier = this->triplet().to_string();
        if (auto const p_compile_triplet = this->m_compile_triplet.get())
        {
            qualifier.append(SEP_TRIPLET_COMPILE_TRIPLET).append(p_compile_triplet->to_string());
        }
        return qualifier;
    }

    std::string PackageSpec::dir() const { return Strings::format("%s_%s", this->m_name, this->qualifier()); }

    std::string PackageSpec::to_string() const { return Strings::format("%s:%s", this->name(), this->qualifier()); }
    void PackageSpec::to_string(std::string& s) const { Strings::append(s, this->name(), ':', this->qualifier()); }

    bool operator==(const PackageSpec& left, const PackageSpec& right)
    {
        return left.name() == right.name() && left.triplet() == right.triplet() &&
               left.compile_triplet() == right.compile_triplet();
    }

    DECLARE_AND_REGISTER_MESSAGE(IllegalPlatformSpec,
                                 (),
                                 "",
                                 "Error: Platform qualifier is not allowed in this context");
    DECLARE_AND_REGISTER_MESSAGE(IllegalFeatures, (), "", "Error: List of features is not allowed in this contect");

    static InternalFeatureSet normalize_feature_list(View<std::string> fs, ImplicitDefault id)
    {
        InternalFeatureSet ret;
        bool core = false;
        for (auto&& f : fs)
        {
            if (f == "core")
            {
                core = true;
            }
            ret.emplace_back(f);
        }

        if (!core)
        {
            ret.emplace_back("core");
            if (id == ImplicitDefault::YES)
            {
                ret.emplace_back("default");
            }
        }
        return ret;
    }

    ExpectedS<FullPackageSpec> ParsedQualifiedSpecifier::to_full_spec(Triplet default_triplet,
                                                                      Optional<bin2sth::CompileTriplet> ct,
                                                                      ImplicitDefault id) const
    {
        if (platform)
        {
            return {msg::format(msgIllegalPlatformSpec).data(), expected_right_tag};
        }

        const Triplet t = triplet ? Triplet::from_canonical_name(*triplet.get()) : default_triplet;
        const View<std::string> fs = !features.get() ? View<std::string>{} : *features.get();
        return FullPackageSpec{{name, t, ct}, normalize_feature_list(fs, id)};
    }

    ExpectedS<PackageSpec> ParsedQualifiedSpecifier::to_package_spec(Triplet default_triplet,
                                                                     Optional<bin2sth::CompileTriplet> ct) const
    {
        if (platform)
        {
            return {msg::format(msgIllegalPlatformSpec).data(), expected_right_tag};
        }
        if (features)
        {
            return {msg::format(msgIllegalFeatures).data(), expected_right_tag};
        }

        const Triplet t = triplet ? Triplet::from_canonical_name(*triplet.get()) : default_triplet;
        return PackageSpec{name, t, ct};
    }

    static bool is_package_name_char(char32_t ch)
    {
        return Parse::ParserBase::is_lower_alpha(ch) || Parse::ParserBase::is_ascii_digit(ch) || ch == '-';
    }

    static bool is_feature_name_char(char32_t ch)
    {
        // TODO: we do not intend underscores to be valid, however there is currently a feature using them
        // (libwebp[vwebp_sdl]).
        // TODO: we need to rename this feature, then remove underscores from this list.
        return is_package_name_char(ch) || ch == '_';
    }

    static bool is_compile_triplet_char(char32_t ch)
    {
        // example: clang-9.0.1_O1_O2
        return Parse::ParserBase::is_alphanumdash(ch) || ch == '_' || ch == '.';
    }

    ExpectedS<ParsedQualifiedSpecifier> parse_qualified_specifier(StringView input)
    {
        auto parser = Parse::ParserBase(input, "<unknown>");
        auto maybe_pqs = parse_qualified_specifier(parser);
        if (!parser.at_eof()) parser.add_error("expected eof");
        if (auto e = parser.get_error()) return e->format();
        return std::move(maybe_pqs).value_or_exit(VCPKG_LINE_INFO);
    }

    Optional<std::string> parse_feature_name(Parse::ParserBase& parser)
    {
        using Parse::ParserBase;
        auto ret = parser.match_zero_or_more(is_feature_name_char).to_string();
        auto ch = parser.cur();

        // ignores the feature name vwebp_sdl as a back-compat thing
        const bool has_underscore = std::find(ret.begin(), ret.end(), '_') != ret.end() && ret != "vwebp_sdl";
        if (has_underscore || ParserBase::is_upper_alpha(ch))
        {
            parser.add_error("invalid character in feature name (must be lowercase, digits, '-')");
            return nullopt;
        }

        if (ret == "default")
        {
            parser.add_error("'default' is a reserved feature name");
            return nullopt;
        }

        if (ret.empty())
        {
            parser.add_error("expected feature name (must be lowercase, digits, '-')");
            return nullopt;
        }
        return ret;
    }
    Optional<std::string> parse_package_name(Parse::ParserBase& parser)
    {
        using Parse::ParserBase;
        auto ret = parser.match_zero_or_more(is_package_name_char).to_string();
        auto ch = parser.cur();
        if (ParserBase::is_upper_alpha(ch) || ch == '_')
        {
            parser.add_error("invalid character in package name (must be lowercase, digits, '-')");
            return nullopt;
        }
        if (ret.empty())
        {
            parser.add_error("expected package name (must be lowercase, digits, '-')");
            return nullopt;
        }
        return ret;
    }

    Optional<ParsedQualifiedSpecifier> parse_qualified_specifier(Parse::ParserBase& parser)
    {
        using Parse::ParserBase;
        ParsedQualifiedSpecifier ret;
        auto name = parse_package_name(parser);
        if (auto n = name.get())
            ret.name = std::move(*n);
        else
            return nullopt;
        auto ch = parser.cur();
        if (ch == '[')
        {
            std::vector<std::string> features;
            do
            {
                parser.next();
                parser.skip_tabs_spaces();
                if (parser.cur() == '*')
                {
                    features.push_back("*");
                    parser.next();
                }
                else
                {
                    auto feature = parse_feature_name(parser);
                    if (auto f = feature.get())
                        features.push_back(std::move(*f));
                    else
                        return nullopt;
                }
                auto skipped_space = parser.skip_tabs_spaces();
                ch = parser.cur();
                if (ch == ']')
                {
                    ch = parser.next();
                    break;
                }
                else if (ch == ',')
                {
                    continue;
                }
                else
                {
                    if (skipped_space.size() > 0 || Parse::ParserBase::is_lineend(parser.cur()))
                        parser.add_error("expected ',' or ']' in feature list");
                    else
                        parser.add_error("invalid character in feature name (must be lowercase, digits, '-', or '*')");
                    return nullopt;
                }
            } while (true);
            ret.features = std::move(features);
        }
        if (ch == ':')
        {
            parser.next();
            ret.triplet = parser.match_zero_or_more(is_package_name_char).to_string();
            if (ret.triplet.get()->empty())
            {
                parser.add_error("expected triplet name (must be lowercase, digits, '-')");
                return nullopt;
            }
            if (parser.cur() == SEP_TRIPLET_COMPILE_TRIPLET[0])
            {
                parser.next();
                ret.compile_triplet = parser.match_zero_or_more(is_compile_triplet_char).to_string();
                if (ret.compile_triplet.get()->empty())
                {
                    parser.add_error("expected compile-triplet name (must be lowercase, digits, '-', '_')");
                    return nullopt;
                }
            }
        }
        parser.skip_tabs_spaces();
        ch = parser.cur();
        if (ch == '(')
        {
            auto loc = parser.cur_loc();
            std::string platform_string;
            int depth = 1;
            while (depth > 0 && (ch = parser.next()) != 0)
            {
                if (ch == '(') ++depth;
                if (ch == ')') --depth;
            }
            if (depth > 0)
            {
                parser.add_error("unmatched open braces in platform specifier", loc);
                return nullopt;
            }
            platform_string.append((++loc.it).pointer_to_current(), parser.it().pointer_to_current());
            auto platform_opt = PlatformExpression::parse_platform_expression(
                platform_string, PlatformExpression::MultipleBinaryOperators::Allow);
            if (auto platform = platform_opt.get())
            {
                ret.platform = std::move(*platform);
            }
            else
            {
                parser.add_error(platform_opt.error(), loc);
            }
            parser.next();
        }
        // This makes the behavior of the parser more consistent -- otherwise, it will skip tabs and spaces only if
        // there isn't a qualifier.
        parser.skip_tabs_spaces();
        return ret;
    }

    Optional<std::pair<Triplet, Optional<bin2sth::CompileTriplet>>> deserialize_qualifier(
        const ParsedQualifiedSpecifier& p)
    {
        if (!p.triplet.has_value()) return nullopt;

        auto triplet = Triplet::from_canonical_name(std::string(*p.triplet.get()));
        auto compile_triplet =
            (p.compile_triplet && !p.compile_triplet.get()->empty())
                ? bin2sth::CompileTriplet::from_canonical_name(std::string(*p.compile_triplet.get()), triplet)
                : nullopt;
        return std::make_pair(std::move(triplet), std::move(compile_triplet));
    }

    bool operator==(const DependencyConstraint& lhs, const DependencyConstraint& rhs)
    {
        if (lhs.type != rhs.type) return false;
        if (lhs.value != rhs.value) return false;
        return lhs.port_version == rhs.port_version;
    }

    Optional<Version> DependencyConstraint::try_get_minimum_version() const
    {
        if (type == VersionConstraintKind::None)
        {
            return nullopt;
        }

        return Version{
            value,
            port_version,
        };
    }

    FullPackageSpec Dependency::to_full_spec(Triplet target,
                                             Triplet host_triplet,
                                             Optional<bin2sth::CompileTriplet> compile_triplet,
                                             ImplicitDefault id) const
    {
        return FullPackageSpec{{name, host ? host_triplet : target, host ? compile_triplet : nullopt},
                               normalize_feature_list(features, id)};
    }

    bool operator==(const Dependency& lhs, const Dependency& rhs)
    {
        if (lhs.name != rhs.name) return false;
        if (lhs.features != rhs.features) return false;
        if (!structurally_equal(lhs.platform, rhs.platform)) return false;
        if (lhs.extra_info != rhs.extra_info) return false;
        if (lhs.constraint != rhs.constraint) return false;
        if (lhs.host != rhs.host) return false;

        return true;
    }
    bool operator!=(const Dependency& lhs, const Dependency& rhs);

    bool operator==(const DependencyOverride& lhs, const DependencyOverride& rhs)
    {
        if (lhs.version_scheme != rhs.version_scheme) return false;
        if (lhs.port_version != rhs.port_version) return false;
        if (lhs.name != rhs.name) return false;
        if (lhs.version != rhs.version) return false;
        return lhs.extra_info == rhs.extra_info;
    }
    bool operator!=(const DependencyOverride& lhs, const DependencyOverride& rhs);
}

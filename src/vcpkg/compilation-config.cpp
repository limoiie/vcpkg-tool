#include <vcpkg/compilation-config.h>

namespace vcpkg::bin2sth
{
    static std::string deref_or(std::unique_ptr<std::string> const& p, StringLiteral default_)
    {
        return p && p->size() ? *p : default_;
    }

    CompilationConfig::CompilationConfig(std::string compiler_tag,
                                         std::string optimization_tag,
                                         std::string obfuscation_tag,
                                         Triplet triplet)
        : compiler_tag(std::move(compiler_tag))
        , optimization_tag(std::move(optimization_tag))
        , obfuscation_tag(std::move(obfuscation_tag))
        , triplet(std::move(triplet))
    {
    }

    CompilationConfig::CompilationConfig(std::unique_ptr<std::string> const& compiler_tag,
                                         std::unique_ptr<std::string> const& optimization_tag,
                                         std::unique_ptr<std::string> const& obfuscation_tag,
                                         Triplet triplet)
        : compiler_tag(deref_or(compiler_tag, DEFAULT_COMPILER_TAG))
        , optimization_tag(deref_or(optimization_tag, DEFAULT_OPTIMIZATION_TAG))
        , obfuscation_tag(deref_or(obfuscation_tag, DEFAULT_OBFUSCATION_TAG))
        , triplet(triplet)
    {
    }

    Optional<CompilationConfig> CompilationConfig::from_canonical_name(const std::string& canonical_name,
                                                                       Triplet triplet)
    {
        if (canonical_name.empty()) return nullopt;
        auto parts = Strings::split(canonical_name, '_');
        Checks::check_exit(VCPKG_LINE_INFO, parts.size() == 3);
        return CompilationConfig{std::move(parts[0]), std::move(parts[1]), std::move(parts[2]), std::move(triplet)};
    }

    std::string CompilationConfig::to_string() const { return canonical_name(); }

    void CompilationConfig::to_string(std::string& str) const { str.append(to_string()); }

    std::string CompilationConfig::canonical_name() const
    {
        return Strings::format("%s_%s_%s", compiler_tag, optimization_tag, obfuscation_tag);
    }

    void CompilationConfig::with_triplet(Triplet new_triplet) { this->triplet = std::move(new_triplet); }

    size_t CompilationConfig::hash_code() const
    {
        auto const fn_str_hash = std::hash<std::string>();
        size_t hash = 17;
        hash = hash * 31 + fn_str_hash(compiler_tag);
        hash = hash * 31 + fn_str_hash(optimization_tag);
        hash = hash * 31 + fn_str_hash(obfuscation_tag);
        hash = hash * 31 + std::hash<Triplet>()(triplet);
        return hash;
    }

    bool operator==(CompilationConfig const& left, CompilationConfig const& right)
    {
        return left.hash_code() == right.hash_code();
    }
}
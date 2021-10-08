#include <vcpkg/compile-triplet.h>
#include <vcpkg/vcpkgcmdarguments.h>

namespace vcpkg::bin2sth
{
    static std::string deref_or(std::unique_ptr<std::string> const& p, StringLiteral default_)
    {
        return p && p->size() ? *p : default_;
    }

    CompileTriplet::CompileTriplet(std::string compiler_tag, std::string optimization_tag, std::string obfuscation_tag)
        : compiler_tag(std::move(compiler_tag))
        , optimization_tag(std::move(optimization_tag))
        , obfuscation_tag(std::move(obfuscation_tag))
    {
    }

    CompileTriplet::CompileTriplet(std::unique_ptr<std::string> const& compiler_tag,
                                   std::unique_ptr<std::string> const& optimization_tag,
                                   std::unique_ptr<std::string> const& obfuscation_tag)
        : compiler_tag(deref_or(compiler_tag, DEFAULT_COMPILER_TAG))
        , optimization_tag(deref_or(optimization_tag, DEFAULT_OPTIMIZATION_TAG))
        , obfuscation_tag(deref_or(obfuscation_tag, DEFAULT_OBFUSCATION_TAG))
    {
    }

    Optional<CompileTriplet> CompileTriplet::from_canonical_name(const std::string& canonical_name)
    {
        if (canonical_name.empty()) return nullopt;
        auto parts = Strings::split(canonical_name, '_');
        Checks::check_exit(VCPKG_LINE_INFO, parts.size() == 3);
        return CompileTriplet{std::move(parts[0]), std::move(parts[1]), std::move(parts[2])};
    }

    std::string CompileTriplet::to_string() const { return canonical_name(); }

    void CompileTriplet::to_string(std::string& str) const { str.append(to_string()); }

    std::string CompileTriplet::canonical_name() const
    {
        return Strings::format("%s_%s_%s", compiler_tag, optimization_tag, obfuscation_tag);
    }

    size_t CompileTriplet::hash_code() const
    {
        auto const fn_str_hash = std::hash<std::string>();
        size_t hash = 17;
        hash = hash * 31 + fn_str_hash(compiler_tag);
        hash = hash * 31 + fn_str_hash(optimization_tag);
        hash = hash * 31 + fn_str_hash(obfuscation_tag);
        return hash;
    }

    bool operator==(CompileTriplet const& left, CompileTriplet const& right)
    {
        return left.hash_code() == right.hash_code();
    }

    Optional<CompileTriplet> default_compile_triplet(vcpkg::VcpkgCmdArguments const& args)
    {
        Optional<CompileTriplet> default_compile_triplet = nullopt;
        if (args.bin2sth_enabled())
        {
            auto const compile_triplet = args.bin2sth_compile_triplet ? *args.bin2sth_compile_triplet : "";
            default_compile_triplet = CompileTriplet::from_canonical_name(compile_triplet);
        }
        return default_compile_triplet;
    }
}
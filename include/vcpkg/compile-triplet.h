#pragma once

#include <vcpkg/triplet.h>

namespace vcpkg::bin2sth
{
    constexpr StringLiteral DEFAULT_COMPILER_TAG = "default";
    constexpr StringLiteral DEFAULT_OPTIMIZATION_TAG = "";
    constexpr StringLiteral DEFAULT_OBFUSCATION_TAG = "NONE";

    struct CompileTriplet
    {
    public:
        CompileTriplet(std::string compiler_tag,
                       std::string optimization_tag,
                       std::string obfuscation_tag,
                       Triplet triplet);

        CompileTriplet(std::unique_ptr<std::string> const& compiler_tag,
                       std::unique_ptr<std::string> const& optimization_tag,
                       std::unique_ptr<std::string> const& obfuscation_tag,
                       Triplet triplet);

        static Optional<CompileTriplet> from_canonical_name(std::string const& canonical_name, Triplet triplet);

        void with_triplet(Triplet new_triplet);

        std::string to_string() const;
        void to_string(std::string&) const;
        std::string canonical_name() const;

        size_t hash_code() const;

        bool operator<(const CompileTriplet& other) const { return canonical_name() < other.canonical_name(); }

        std::string compiler_tag;
        std::string optimization_tag;
        std::string obfuscation_tag;

        Triplet triplet;
    };

    bool operator==(const CompileTriplet& left, const CompileTriplet& right);
    inline bool operator!=(const CompileTriplet& left, const CompileTriplet& right) { return !(left == right); }

    Optional<bin2sth::CompileTriplet> default_compile_triplet(VcpkgCmdArguments const& args, Triplet default_triplet);

}

namespace std
{
    template<>
    struct hash<vcpkg::bin2sth::CompileTriplet>
    {
        size_t operator()(const vcpkg::bin2sth::CompileTriplet& obj) const { return obj.hash_code(); }
    };
}
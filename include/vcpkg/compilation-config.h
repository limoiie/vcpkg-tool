#pragma once

#include <vcpkg/triplet.h>

namespace vcpkg::bin2sth
{
    constexpr StringLiteral DEFAULT_COMPILER_TAG = "default";
    constexpr StringLiteral DEFAULT_OPTIMIZATION_TAG = "";
    constexpr StringLiteral DEFAULT_OBFUSCATION_TAG = "NONE";

    struct CompilationConfig
    {
    public:
        CompilationConfig(std::string compiler_tag,
                          std::string optimization_tag,
                          std::string obfuscation_tag,
                          Triplet triplet);

        CompilationConfig(std::unique_ptr<std::string> const& compiler_tag,
                          std::unique_ptr<std::string> const& optimization_tag,
                          std::unique_ptr<std::string> const& obfuscation_tag,
                          Triplet triplet);

        static Optional<CompilationConfig> from_canonical_name(std::string const& canonical_name, Triplet triplet);

        void with_triplet(Triplet new_triplet);

        std::string to_string() const;
        void to_string(std::string&) const;
        std::string canonical_name() const;

        size_t hash_code() const;

        bool operator<(const CompilationConfig& other) const { return canonical_name() < other.canonical_name(); }

        std::string compiler_tag;
        std::string optimization_tag;
        std::string obfuscation_tag;

        Triplet triplet;
    };

    bool operator==(const CompilationConfig& left, const CompilationConfig& right);
    inline bool operator!=(const CompilationConfig& left, const CompilationConfig& right) { return !(left == right); }

}

namespace std
{
    template<>
    struct hash<vcpkg::bin2sth::CompilationConfig>
    {
        size_t operator()(const vcpkg::bin2sth::CompilationConfig& obj) const { return obj.hash_code(); }
    };
}
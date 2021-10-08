#pragma once

#include <vcpkg/compile-triplet.h>

#include <map>

namespace vcpkg
{
    struct Triplet;
    struct CompilerInfo;
    struct VcpkgCmdArguments;
    struct VcpkgPaths;
}

namespace vcpkg::bin2sth
{
    struct CompileTriplet;

    struct ConfigFlags
    {
        std::string name;
        std::string flags;

        size_t hash_code() const;
    };

    struct CompilationFlags
    {
        CompilationFlags(CompilerInfo const& compiler_info, ConfigFlags&& optimization, ConfigFlags&& obfuscation);

        std::string c_compiler_full_path() const;
        std::string cxx_compiler_full_path() const;

        std::string make_cxx_flags() const;
        std::string make_c_flags() const;

    private:
        CompilerInfo const& m_compiler_info;
        ConfigFlags m_optimization;
        ConfigFlags m_obfuscation;
    };

    struct CompilationFlagsFactory
    {
        CompilationFlagsFactory(const std::map<std::string, CompilerInfo>& compilers);

        CompilationFlags interpret(const CompileTriplet& config) const;

    private:
        const std::map<std::string, CompilerInfo>& m_compilers;
    };
}

namespace std
{
    template<>
    struct hash<vcpkg::bin2sth::ConfigFlags>
    {
        size_t operator()(vcpkg::bin2sth::ConfigFlags const& obj) const { return obj.hash_code(); }
    };
}
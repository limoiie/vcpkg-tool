#pragma once

#include <vcpkg/compilation-config.h>

#include <map>

namespace vcpkg
{
    struct Triplet;
    struct VcpkgCmdArguments;
    struct VcpkgPaths;
}

namespace vcpkg::bin2sth
{
    struct CompilationConfig;

    struct CompilerInfo
    {
        std::string name;
        std::string version;
        std::string c_full_path;
        std::string cxx_full_path;

        size_t hash_code() const;
        std::string nickname() const { return std::string(name).append("-").append(version); }
    };

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

    struct CompilationConfigFactory
    {
        CompilationConfigFactory(Filesystem& filesystem, Path const& compiler_config_dir);

        CompilationFlags make_flags_from_config(CompilationConfig const& config) const;

    private:
        std::map<std::string, CompilerInfo> m_compilers;
    };

    Optional<bin2sth::CompilationConfig> default_compilation_config(VcpkgCmdArguments const& args,
                                                                    Triplet default_triplet);
}

namespace std
{
    template<>
    struct hash<vcpkg::bin2sth::CompilerInfo>
    {
        size_t operator()(vcpkg::bin2sth::CompilerInfo const& obj) const { return obj.hash_code(); }
    };

    template<>
    struct hash<vcpkg::bin2sth::ConfigFlags>
    {
        size_t operator()(vcpkg::bin2sth::ConfigFlags const& obj) const { return obj.hash_code(); }
    };
}
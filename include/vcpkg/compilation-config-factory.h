#pragma once

#include <vcpkg/triplet.h>

namespace vcpkg::bin2sth
{
    struct CompilationConfig;
    struct CompilerInfo
    {
        std::string name;
        std::string version;
        std::string c_full_path;
        std::string cxx_full_path;

        std::string nickname() const { return std::string(name).append("-").append(version); }
    };

    struct ConfigFlags
    {
        std::string name;
        std::string flags;

        std::string nickname() const { return name; }
    };

    struct CompilationConfigFactory
    {
        CompilationConfigFactory(Filesystem& filesystem, Path const& compiler_config_dir);

        std::unique_ptr<CompilationConfig> create_config(std::unique_ptr<std::string> const& compiler,
                                                         std::unique_ptr<std::string> const& optimization,
                                                         std::unique_ptr<std::string> const& obfuscation,
                                                         Triplet const& triplet) const;

    private:
        std::map<std::string, CompilerInfo> m_compilers;
    };
}
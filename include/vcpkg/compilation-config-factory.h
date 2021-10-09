#pragma once

#include <vcpkg/compilation-config.h>

namespace vcpkg
{
    struct Triplet;
}

namespace vcpkg::bin2sth
{
    struct CompilationConfig;

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
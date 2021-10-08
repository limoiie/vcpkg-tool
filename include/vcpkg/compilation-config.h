#pragma once

#include <vcpkg/triplet.h>

namespace vcpkg::bin2sth
{
    struct CompilerInfo;
    struct ConfigFlags;

    struct CompilationConfig
    {
    public:
        CompilationConfig(CompilerInfo const& compiler,
                          ConfigFlags optimization_flags,
                          ConfigFlags obfuscation_flags,
                          Triplet triplet);

        std::string c_compiler_full_path() const;
        std::string cxx_compiler_full_path() const;

        std::string make_cxx_flags() const;
        std::string make_c_flags() const;

        std::string to_string() const;
        void to_string(std::string&) const;

    private:
        CompilerInfo const& compiler_info;
        ConfigFlags optimization;
        ConfigFlags obfuscation;

        Triplet triplet;
    };

}
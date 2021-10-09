#include <vcpkg/compilation-config.h>

namespace vcpkg::bin2sth
{
    CompilationConfig::CompilationConfig(CompilerInfo const& compiler_info,
                                         ConfigFlags optimization,
                                         ConfigFlags obfuscation,
                                         Triplet triplet)
        : compiler_info(compiler_info)
        , optimization(std::move(optimization))
        , obfuscation(std::move(obfuscation))
        , triplet(triplet)
    {
        // TODO: verify if the compiler supports obfuscation or not
        // TODO: verify if the compiler is good with the target architecture
    }

    std::string CompilationConfig::to_string() const
    {
        return std::string(triplet.to_string())
            .append("_")
            .append(compiler_info.nickname())
            .append("_")
            .append(optimization.nickname())
            .append("_")
            .append(obfuscation.nickname());
    }

    void CompilationConfig::to_string(std::string& str) const { str.assign(to_string()); }

    std::string CompilationConfig::c_compiler_full_path() const { return compiler_info.c_full_path; }
    std::string CompilationConfig::cxx_compiler_full_path() const { return compiler_info.cxx_full_path; }

    std::string CompilationConfig::make_cxx_flags() const
    {
        return std::string(optimization.flags).append(" ").append(obfuscation.flags);
    }

    std::string CompilationConfig::make_c_flags() const
    {
        return std::string(optimization.flags).append(" ").append(obfuscation.flags);
    }
}
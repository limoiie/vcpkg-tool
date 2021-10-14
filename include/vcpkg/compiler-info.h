#pragma once

#include <pch.h>

namespace vcpkg
{
    struct Filesystem;

    struct CompilerInfo
    {
        std::string name;
        std::string version;
        std::string c_full_path;
        std::string cxx_full_path;

        static CompilerInfo load_compiler_info(Filesystem& filesystem, const Path& compiler_info_path);

        size_t hash_code() const;
        std::string nickname() const { return std::string(name).append("-").append(version); }
    };
}

namespace std
{
    template<>
    struct hash<vcpkg::CompilerInfo>
    {
        size_t operator()(vcpkg::CompilerInfo const& obj) const { return obj.hash_code(); }
    };

}
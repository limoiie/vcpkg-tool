#pragma once

#include <pch.h>

namespace vcpkg
{
    struct Filesystem;

    struct CompilerInfo
    {
        std::string name;
        std::string version;
        Path c_full_path;
        Path cxx_full_path;

        static CompilerInfo load(const Filesystem& filesystem, const Path& path);
        static void dump(Filesystem& filesystem, const Path& path, const CompilerInfo& instance);

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
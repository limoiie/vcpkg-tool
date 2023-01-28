#pragma once

#include <vcpkg/base/files.h>

#include <vcpkg/binaryparagraph.h>
#include <vcpkg/packagespec.h>
#include <vcpkg/triplet.h>

namespace vcpkg
{
    struct InstalledPaths
    {
        explicit InstalledPaths(Path&& root) : m_root(std::move(root)) { }

        const Path& root() const { return m_root; }
        Path listfile_path(const BinaryParagraph& pgh) const;

        Path vcpkg_dir() const { return m_root / "vcpkg"; }
        Path vcpkg_dir_status_file() const { return vcpkg_dir() / "status"; }
        Path vcpkg_dir_info() const { return vcpkg_dir() / "info"; }
        Path vcpkg_dir_updates() const { return vcpkg_dir() / "updates"; }
        Path lockfile_path() const { return vcpkg_dir() / "vcpkg-lock.json"; }
        Path triplet_dir(Triplet t, Optional<bin2sth::CompileTriplet> ct) const { return triplet_dir({"", t, ct}); }
        Path triplet_dir(const PackageSpec& p) const { return m_root / p.qualifier(); }
        Path share_dir(const PackageSpec& p) const { return triplet_dir(p) / "share" / p.name(); }
        Path usage_file(const PackageSpec& p) const { return share_dir(p) / "usage"; }
        Path vcpkg_port_config_cmake(const PackageSpec& p) const { return share_dir(p) / "vcpkg-port-config.cmake"; }

    private:
        Path m_root;
    };

}
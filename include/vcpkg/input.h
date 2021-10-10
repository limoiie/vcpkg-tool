#pragma once

#include <vcpkg/packagespec.h>

namespace vcpkg::Input
{
    PackageSpec check_and_get_package_spec(std::string&& spec_string,
                                           Triplet default_triplet,
                                           Optional<bin2sth::CompilationConfig> const& default_compilation_config,
                                           CStringView example_text);
    FullPackageSpec check_and_get_full_package_spec(std::string&& spec_string,
                                                    Triplet default_triplet,
                                                    Optional<bin2sth::CompilationConfig> const& compilation_config,
                                                    CStringView example_text);

    void check_triplet(Triplet t, const VcpkgPaths& paths);
}

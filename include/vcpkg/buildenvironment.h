#pragma once

#include <vcpkg/fwd/vcpkgpaths.h>

#include <vcpkg/base/system.process.h>

#include <vcpkg/compilation-config.h>

#include <string>
#include <vector>

namespace vcpkg
{
    Command make_cmake_cmd(const VcpkgPaths& paths,
                           const Path& cmake_script,
                           std::vector<CMakeVariable>&& pass_variables,
                           Optional<bin2sth::CompilationConfig> const& compilation_config=nullopt);
}

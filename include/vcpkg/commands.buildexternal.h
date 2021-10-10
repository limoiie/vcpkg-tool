#pragma once

#include <vcpkg/commands.interface.h>

namespace vcpkg::Commands::BuildExternal
{
    void perform_and_exit(const VcpkgCmdArguments& args,
                          const VcpkgPaths& paths,
                          Triplet default_triplet,
                          Triplet host_triplet,
                          Optional<bin2sth::CompilationConfig>&& default_compilation_config);

    struct BuildExternalCommand : TripletCommand
    {
        virtual void perform_and_exit(const VcpkgCmdArguments& args,
                                      const VcpkgPaths& paths,
                                      Triplet default_triplet,
                                      Triplet host_triplet,
                                      Optional<bin2sth::CompilationConfig>&& default_compilation_config) const override;
    };
}

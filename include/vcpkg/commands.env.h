#pragma once

#include <vcpkg/commands.interface.h>

namespace vcpkg::Commands::Env
{
    extern const CommandStructure COMMAND_STRUCTURE;
    void perform_and_exit(const VcpkgCmdArguments& args,
                          const VcpkgPaths& paths,
                          Triplet default_triplet,
                          Triplet host_triplet,
                          Optional<bin2sth::CompileTriplet>&& compile_triplet);

    struct EnvCommand : TripletCommand
    {
        virtual void perform_and_exit(const VcpkgCmdArguments& args,
                                      const VcpkgPaths& paths,
                                      Triplet default_triplet,
                                      Triplet host_triplet,
                                      Optional<bin2sth::CompileTriplet>&& default_compile_triplet) const override;
    };
}

#pragma once

#include <vcpkg/commands.interface.h>

namespace vcpkg::Export
{
    extern const CommandStructure COMMAND_STRUCTURE;

    void perform_and_exit(const VcpkgCmdArguments& args,
                          const VcpkgPaths& paths,
                          Triplet default_triplet,
                          Optional<bin2sth::CompileTriplet>&& compile_triplet);

    void export_integration_files(const Path& raw_exported_dir_path, const VcpkgPaths& paths);

    struct ExportCommand : Commands::TripletCommand
    {
        virtual void perform_and_exit(const VcpkgCmdArguments& args,
                                      const VcpkgPaths& paths,
                                      Triplet default_triplet,
                                      Triplet host_triplet,
                                      Optional<bin2sth::CompileTriplet>&& default_compile_triplet) const override;
    };
}

#include <vcpkg/binarycaching.h>
#include <vcpkg/build.h>
#include <vcpkg/cmakevars.h>
#include <vcpkg/commands.buildexternal.h>
#include <vcpkg/help.h>
#include <vcpkg/input.h>
#include <vcpkg/portfileprovider.h>
#include <vcpkg/vcpkgcmdarguments.h>

namespace vcpkg::Commands::BuildExternal
{
    const CommandStructure COMMAND_STRUCTURE = {
        create_example_string(R"(build-external zlib2 C:\path\to\dir\with\controlfile\)"),
        2,
        2,
        {},
        nullptr,
    };

    void perform_and_exit(const VcpkgCmdArguments& args,
                          const VcpkgPaths& paths,
                          Triplet default_triplet,
                          Triplet host_triplet,
                          Optional<bin2sth::CompileTriplet>&& default_compile_triplet)
    {
        const ParsedArguments options = args.parse_arguments(COMMAND_STRUCTURE);

        BinaryCache binary_cache{args, paths};

        const FullPackageSpec spec = Input::check_and_get_full_package_spec(std::string(args.command_arguments.at(0)),
                                                                            default_triplet,
                                                                            default_compile_triplet,
                                                                            COMMAND_STRUCTURE.example_text, paths);

        auto overlays = args.overlay_ports;
        overlays.insert(overlays.begin(), args.command_arguments.at(1));

        PortFileProvider::PathsPortFileProvider provider(paths, overlays);
        auto maybe_scfl = provider.get_control_file(spec.package_spec.name());

        Checks::check_maybe_upgrade(
            VCPKG_LINE_INFO, maybe_scfl.has_value(), "could not load control file for %s", spec.package_spec.name());

        Build::Command::perform_and_exit_ex(args,
                                            spec,
                                            host_triplet,
                                            maybe_scfl.value_or_exit(VCPKG_LINE_INFO),
                                            provider,
                                            binary_cache,
                                            Build::null_build_logs_recorder(),
                                            paths);
    }

    void BuildExternalCommand::perform_and_exit(const VcpkgCmdArguments& args,
                                                const VcpkgPaths& paths,
                                                Triplet default_triplet,
                                                Triplet host_triplet,
                                                Optional<bin2sth::CompileTriplet>&& default_compile_triplet) const
    {
        BuildExternal::perform_and_exit(args, paths, default_triplet, host_triplet, std::move(default_compile_triplet));
    }
}

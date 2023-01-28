#include <vcpkg/base/system.print.h>

#include <vcpkg/commands.list-compilers.h>
#include <vcpkg/compiler-info.h>
#include <vcpkg/vcpkgcmdarguments.h>
#include <vcpkg/vcpkgpaths.h>

namespace vcpkg::Commands::ListCompilers
{
    namespace ArgFields
    {
        static constexpr StringLiteral OPTION_DETAILED = "detail";
    }

    static void do_print_nicknames(const std::vector<std::string>& nicknames, Optional<std::string>&& pat_opt)
    {
        auto const pat = pat_opt.value_or("");
        for (auto const& name : nicknames)
        {
            if (!Strings::case_insensitive_ascii_contains(name, pat)) continue;
            print2(name, '\n');
        }
    }

    static void do_print(const std::map<std::string, CompilerInfo>& compilers, Optional<std::string>&& pat_opt)
    {
        auto const pat = pat_opt.value_or("");
        for (auto const& pair : compilers)
        {
            if (!Strings::case_insensitive_ascii_contains(pair.first, pat)) continue;
            auto const& compiler = pair.second;
            vcpkg::printf("%-12s %-16s %-32s %s\n",
                          compiler.name,
                          compiler.version,
                          compiler.c_full_path,
                          compiler.cxx_full_path);
        }
    }

    static constexpr std::array<CommandSwitch, 1> LIST_COMPILERS_SWITCHES = {
        {{ArgFields::OPTION_DETAILED, "Show detailed compiler info"}}};

    const CommandStructure COMMAND_STRUCTURE = {
        Strings::format(
            "The argument should be a substring to search for, or no argument to display all registered compilers.\n%s",
            create_example_string("list-compilers tag")),
        0,
        1,
        {LIST_COMPILERS_SWITCHES, {}},
        nullptr,
    };

    void perform_and_exit(const VcpkgCmdArguments& args, const VcpkgPaths& paths)
    {
        Optional<std::string> pat = nullopt;
        if (!args.command_arguments.empty())
        {
            pat = args.command_arguments[0];
        }

        auto const options = args.parse_arguments(COMMAND_STRUCTURE);
        auto const enable_detailed = Util::Sets::contains(options.switches, ArgFields::OPTION_DETAILED.to_string());
        if (enable_detailed)
        {
            auto const& compilers = paths.get_available_compilers();
            do_print(compilers, std::move(pat));
        }
        else
        {
            auto const& nicknames = paths.get_available_compiler_nicknames();
            do_print_nicknames(nicknames, std::move(pat));
        }
        Checks::exit_success(VCPKG_LINE_INFO);
    }

    void ListCompilersCommand::perform_and_exit(const VcpkgCmdArguments& args, const VcpkgPaths& paths) const
    {
        ListCompilers::perform_and_exit(args, paths);
    }
}
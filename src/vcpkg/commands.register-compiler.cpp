#include <vcpkg/base/system.debug.h>
#include <vcpkg/base/system.print.h>
#include <vcpkg/base/system.process.h>

#include <vcpkg/commands.register-compiler.h>
#include <vcpkg/compiler-info.h>
#include <vcpkg/vcpkgcmdarguments.h>
#include <vcpkg/vcpkgpaths.h>

namespace vcpkg::Commands::RegisterCompiler
{
    namespace ArgFields
    {
        static constexpr StringLiteral OPTION_ARG_NAME = "name";
        static constexpr StringLiteral OPTION_ARG_VERSION = "ver";
        static constexpr StringLiteral ARG_C_COMPILER = "c";
        static constexpr StringLiteral ARG_CXX_COMPILER = "cxx";
    }

    static constexpr std::array<CommandSetting, 4> REGISTER_COMPILER_SETTINGS = {
        {{ArgFields::OPTION_ARG_NAME, "Compiler short name"},
         {ArgFields::OPTION_ARG_VERSION, "Compiler version"},
         {ArgFields::ARG_C_COMPILER, "Full path to c compiler"},
         {ArgFields::ARG_CXX_COMPILER, "Full path to cxx compiler"}},
    };

    const CommandStructure COMMAND_STRUCTURE = {
        Strings::format(
            "The argument 'c' and 'cxx' are necessary, while 'name' and 'ver' are optional with default value guessed "
            "from 'c'.",
            create_example_string("register-compilers [--name ..] [--ver ..] --c </path/to/c> --cxx </path/to/cxx>")),
        0,
        4,
        {{}, REGISTER_COMPILER_SETTINGS},
        nullptr,
    };

    namespace details
    {
        static CompilerInfo parse_arguments(const VcpkgCmdArguments& args)
        {
            auto const required_arguments = {ArgFields::ARG_C_COMPILER, ArgFields::ARG_CXX_COMPILER};
            auto const parsed_args = args.parse_arguments(COMMAND_STRUCTURE);

            for (auto const& required : required_arguments)
                if (parsed_args.settings.find(required) == parsed_args.settings.end())
                    Checks::exit_with_message(VCPKG_LINE_INFO, Strings::format("Missing required arg: %s", required));

            auto const value_or = [&parsed_args](auto const& key,
                                                 const Optional<std::string>& default_value = nullopt) {
                if (parsed_args.settings.find(key) == parsed_args.settings.end())
                {
                    if (auto const* p_default_value = default_value.get())
                    {
                        return *p_default_value;
                    }
                    Checks::unreachable(VCPKG_LINE_INFO);
                }
                return parsed_args.settings.at(key);
            };

            return {value_or(ArgFields::OPTION_ARG_NAME, ""),
                    value_or(ArgFields::OPTION_ARG_VERSION, ""),
                    value_or(ArgFields::ARG_C_COMPILER),
                    value_or(ArgFields::ARG_CXX_COMPILER)};
        }

        static Optional<std::string> query_version_by_command(const Path& compiler_path)
        {
            auto const out = cmd_execute_and_capture_output(Command(compiler_path).raw_arg("--version"));
            if (out.exit_code)
            {
                Debug::print("Failed to execute compiler --version: ", out.output, '\n');
                return nullopt;
            }

            static const std::regex version_rex(R"/([0-9]+\.[0-9]+\.[0-9]+)/");
            std::smatch match;
            if (std::regex_search(out.output, match, version_rex))
            {
                return match[0];
            }
            return nullopt;
        }

        static constexpr StringView SEP_REX = "-";

        static constexpr StringView NAME_SEP_REX = R"([.\-])";
        static constexpr StringView VERSION_SEP_REX = R"(.)";

        static constexpr StringView C_NAME_PART_REX = R"([a-zA-Z][^.\-]*)";
        static constexpr StringView C_VERSION_PART_REX = R"([0-9]+)";

        static constexpr StringView C_NAME_REX = Strings::regex::join<NAME_SEP_REX, C_NAME_PART_REX>;
        static constexpr StringView C_VERSION_REX = Strings::regex::join<VERSION_SEP_REX, C_VERSION_PART_REX>;

        static std::pair<std::string, std::string> query_name_version(const Path& compiler_path)
        {
            using namespace Strings::regex;

            std::pair<std::string, std::string> res;

            static constexpr auto NAME_VERSION_REX = exact<cap<C_NAME_REX>, opt<SEP_REX, cap<C_VERSION_REX>>>;
            static const std::regex name_ver_rex(NAME_VERSION_REX.data());

            std::smatch match;
            std::string compiler_filename = compiler_path.filename().to_string();
            if (std::regex_match(compiler_filename, match, name_ver_rex))
            {
                res.first = match[1].str();
                res.second = match[2].str();
            }

            // prefer the version from command output (detailed) than the one from name (abbrev)
            if (auto const* p_version = query_version_by_command(compiler_path).get())
            {
                res.second = *p_version;
            }
            return res;
        }

        static bool is_valid_name(const std::string& compiler_name)
        {
            static const std::regex name_rex(Strings::regex::exact<C_NAME_REX>.data());
            return std::regex_match(compiler_name, name_rex);
        }

        static bool is_valid_version(const std::string& compiler_version)
        {
            static const std::regex version_rex(Strings::regex::exact<C_VERSION_REX>.data());
            return std::regex_match(compiler_version, version_rex);
        }

        static void validate_compiler_info(CompilerInfo& compiler_info, const Filesystem& fs)
        {
            if (!fs.is_regular_file(compiler_info.c_full_path))
            {
                auto const err_msg = Strings::format("Invalid c compiler path: %s\n", compiler_info.c_full_path);
                Checks::exit_with_message(VCPKG_LINE_INFO, err_msg);
            }

            if (!fs.is_regular_file(compiler_info.cxx_full_path))
            {
                auto const err_msg = Strings::format("Invalid cxx compiler path: %s\n", compiler_info.cxx_full_path);
                Checks::exit_with_message(VCPKG_LINE_INFO, err_msg);
            }

            if (compiler_info.name.empty() || compiler_info.version.empty())
            {
                auto const name_ver = query_name_version(compiler_info.c_full_path);
                if (compiler_info.name.empty()) compiler_info.name = name_ver.first;
                if (compiler_info.version.empty()) compiler_info.version = name_ver.second;
            }

            if (!is_valid_name(compiler_info.name))
            {
                auto const err_msg = Strings::format("Invalid name (%s): %s\n", C_NAME_REX, compiler_info.name);
                Checks::exit_with_message(VCPKG_LINE_INFO, err_msg);
            }

            if (!is_valid_version(compiler_info.version))
            {
                auto const err_msg =
                    Strings::format("Invalid version (%s): '%s'\n", C_VERSION_REX, compiler_info.version);
                Checks::exit_with_message(VCPKG_LINE_INFO, err_msg);
            }
        }
    }

    void perform_and_exit(const VcpkgCmdArguments& args, const VcpkgPaths& paths)
    {
        auto& fs = paths.get_filesystem();

        auto compiler_info = details::parse_arguments(args);
        details::validate_compiler_info(compiler_info, fs);

        auto compiler_info_filename = Strings::format("%s-%s.json", compiler_info.name, compiler_info.version);
        auto compiler_info_path = paths.vcpkg_bin2sth_compiler_config_dir / compiler_info_filename;

        if (fs.exists(compiler_info_path, VCPKG_LINE_INFO))
            Checks::exit_with_message(VCPKG_LINE_INFO, Strings::format("Already exists: %s\n", compiler_info_path));

        fs.create_directories(paths.vcpkg_bin2sth_compiler_config_dir, VCPKG_LINE_INFO);
        CompilerInfo::dump(fs, compiler_info_path, compiler_info);

        print2(Strings::format(
            "Succeed to register compiler '%s' at path '%s'\n", compiler_info.nickname(), compiler_info_path));
        Checks::exit_success(VCPKG_LINE_INFO);
    }

    void RegisterCompilerCommand::perform_and_exit(const VcpkgCmdArguments& args, const VcpkgPaths& paths) const
    {
        RegisterCompiler::perform_and_exit(args, paths);
    }
}

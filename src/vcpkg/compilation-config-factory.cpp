#include <vcpkg/base/json.h>
#include <vcpkg/base/jsonreader.h>
#include <vcpkg/base/util.h>

#include <vcpkg/compilation-config-factory.h>
#include <vcpkg/compilation-config.h>
#include <vcpkg/triplet.h>

namespace vcpkg::bin2sth
{
    namespace details
    {
        static CompilerInfo load_compiler_info(Filesystem& filesystem, Path const& compiler_info_path);

        static ConfigFlags parse_optimization_flags(CompilerInfo const& compiler_info,
                                                    std::unique_ptr<std::string> const& optimization);

        static ConfigFlags parse_obfuscation_flags(CompilerInfo const& compiler_info,
                                                   std::unique_ptr<std::string> const& obfuscation);
    }

    CompilationConfigFactory::CompilationConfigFactory(Filesystem& filesystem, Path const& compiler_config_dir)
    {
        for (auto const& compiler_config_path :
             filesystem.get_regular_files_recursive(compiler_config_dir, VCPKG_LINE_INFO))
        {
            auto compiler_info = details::load_compiler_info(filesystem, compiler_config_path);
            m_compilers.emplace(compiler_info.nickname(), std::move(compiler_info));
        }
    }

    std::unique_ptr<CompilationConfig> CompilationConfigFactory::create_config(
        std::unique_ptr<std::string> const& compiler,
        std::unique_ptr<std::string> const& optimization,
        std::unique_ptr<std::string> const& obfuscation,
        Triplet const& triplet) const
    {
        auto const compiler_nickname = compiler ? *compiler : "default";
        auto const compiler_info_itr = m_compilers.find(compiler_nickname);

        Checks::check_exit(VCPKG_LINE_INFO,
                           compiler_info_itr != m_compilers.end(),
                           std::string("Invalid compiler nickname: ").append(compiler_nickname));

        auto const& compiler_info = compiler_info_itr->second;
        auto optimization_flags = details::parse_optimization_flags(compiler_info, optimization);
        auto obfuscation_flags = details::parse_obfuscation_flags(compiler_info, obfuscation);

        return std::make_unique<CompilationConfig>(
            compiler_info, std::move(optimization_flags), std::move(obfuscation_flags), triplet);
    }

    namespace details
    {
        namespace CompilerInfoFields
        {
            constexpr static StringLiteral VERSION = "version";
            constexpr static StringLiteral NAME = "name";
            constexpr static StringLiteral C_FULL_PATH = "c";
            constexpr static StringLiteral CXX_FULL_PATH = "cxx";
        }

        struct CompilerInfoDeserializer : Json::IDeserializer<CompilerInfo>
        {
            virtual StringView type_name() const override { return "a compiler info"; }

            virtual Span<const StringView> valid_fields() const override
            {
                static const StringView u[] = {
                    CompilerInfoFields::VERSION,
                    CompilerInfoFields::NAME,
                    CompilerInfoFields::C_FULL_PATH,
                    CompilerInfoFields::CXX_FULL_PATH,
                };
                return u;
            }

            virtual Optional<CompilerInfo> visit_object(Json::Reader& r, Json::Object const& obj) override
            {
                using namespace CompilerInfoFields;
                static Json::StringDeserializer string_deserializer{"a string"};
                CompilerInfo compiler_info;
                r.optional_object_field(obj, VERSION, compiler_info.version, string_deserializer);
                r.optional_object_field(obj, NAME, compiler_info.name, string_deserializer);
                r.optional_object_field(obj, C_FULL_PATH, compiler_info.c_full_path, string_deserializer);
                r.optional_object_field(obj, CXX_FULL_PATH, compiler_info.cxx_full_path, string_deserializer);
                return compiler_info;
            }

            static CompilerInfoDeserializer instance;
        };

        CompilerInfoDeserializer CompilerInfoDeserializer::instance;

        CompilerInfo load_compiler_info(Filesystem& filesystem, Path const& compiler_info_path)
        {
            std::error_code ec;
            auto compiler_config_opt = Json::parse_file(filesystem, compiler_info_path, ec);
            if (ec || !compiler_config_opt.has_value())
            {
                Checks::exit_with_message(
                    VCPKG_LINE_INFO, "Failed to parse compiler config at %s:\n%s", compiler_info_path, ec.message());
            }

            auto compiler_config_value = std::move(compiler_config_opt).value_or_exit(VCPKG_LINE_INFO);
            if (!compiler_config_value.first.is_object())
            {
                Checks::exit_with_message(
                    VCPKG_LINE_INFO,
                    "Failed to parse compiler config at %s:\nCompiler Config files must have a top-level object\n",
                    compiler_info_path);
            }

            Json::Reader reader;
            auto compiler_info_opt = reader.visit(compiler_config_value.first, CompilerInfoDeserializer::instance);
            if (reader.errors().size())
            {
                auto const err_message = Strings::join("\n", reader.errors());
                Checks::exit_with_message(
                    VCPKG_LINE_INFO, "Failed to convert compiler info json to instance:\n%s", err_message);
            }

            return std::move(*compiler_info_opt.get());
        }

        ConfigFlags parse_optimization_flags(CompilerInfo const&, std::unique_ptr<std::string> const& optimization)
        {
            constexpr auto DEFAULT_OPTIMIZATION = "O0";
            auto const opt = optimization ? *optimization : DEFAULT_OPTIMIZATION;
            constexpr char const* supported_optimizations[] = {"O0", "O1", "O2", "O3", "Os"};
            for (auto const& supported_opt : supported_optimizations)
            {
                if (supported_opt == *optimization)
                {
                    return {opt, std::string("-").append(supported_opt)};
                }
            }

            Checks::exit_with_message(VCPKG_LINE_INFO, std::string("Error: Invalid optimization: ").append(opt));
        }

        ConfigFlags parse_obfuscation_flags(CompilerInfo const&, std::unique_ptr<std::string> const& obfuscation)
        {
            constexpr auto DEFAULT_OBFUSCATION = "NONE";
            auto const obf = Strings::ascii_to_uppercase(obfuscation ? *obfuscation : DEFAULT_OBFUSCATION);
            if (obf == DEFAULT_OBFUSCATION) return {obf, ""};

            std::string obfuscation_flags;
            if (obf.size() == 2 && std::isalpha(obf[0]) && std::isdigit(obf[1]))
            {
                auto const loop_num = std::to_string(static_cast<unsigned>(obf[1] - '0'));
                switch (obf[0])
                {
                    case 'S': return {obf, std::string("-llvm -sub -llvm -sub-loop=").append(loop_num)};
                    case 'B': return {obf, std::string("-llvm -bcf -llvm -bcf-loop=").append(loop_num)};
                    case 'F': return {obf, std::string("-llvm -fla -llvm -fla-loop=").append(loop_num)};
                }
            }

            Checks::exit_with_message(VCPKG_LINE_INFO, std::string("Error: Invalid obfuscation: ").append(obf));
        }
    }

}
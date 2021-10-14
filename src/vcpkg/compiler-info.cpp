#include <vcpkg/base/json.h>
#include <vcpkg/base/jsonreader.h>
#include <vcpkg/base/lineinfo.h>
#include <vcpkg/base/util.h>

#include <vcpkg/compiler-info.h>

namespace vcpkg
{
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

            virtual Optional<CompilerInfo> visit_object(Json::Reader& r, const Json::Object& obj) override
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

    }

    CompilerInfo CompilerInfo::load_compiler_info(Filesystem& filesystem, const Path& compiler_info_path)
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
        auto compiler_info_opt = reader.visit(compiler_config_value.first, details::CompilerInfoDeserializer::instance);
        if (reader.errors().size())
        {
            auto const err_message = Strings::join("\n", reader.errors());
            Checks::exit_with_message(
                VCPKG_LINE_INFO, "Failed to convert compiler info json to instance:\n%s", err_message);
        }

        return std::move(*compiler_info_opt.get());
    }

    size_t CompilerInfo::hash_code() const
    {
        auto const fn_str_hash = std::hash<std::string>();
        size_t hash = 31;
        hash = hash * 17 + fn_str_hash(name);
        hash = hash * 17 + fn_str_hash(version);
        hash = hash * 17 + fn_str_hash(c_full_path);
        hash = hash * 17 + fn_str_hash(cxx_full_path);
        return hash;
    }

}
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
                using namespace Json;
                static StringDeserializer string_deserializer{"a string"};
                CompilerInfo compiler_info;
                r.optional_object_field(obj, VERSION, compiler_info.version, string_deserializer);
                r.optional_object_field(obj, NAME, compiler_info.name, string_deserializer);
                r.optional_object_field(obj, C_FULL_PATH, compiler_info.c_full_path, PathDeserializer::instance);
                r.optional_object_field(obj, CXX_FULL_PATH, compiler_info.cxx_full_path, PathDeserializer::instance);
                return compiler_info;
            }

            static CompilerInfoDeserializer instance;
        };

        CompilerInfoDeserializer CompilerInfoDeserializer::instance;

        static Json::Object serialize(const CompilerInfo& compiler_info)
        {
            using namespace CompilerInfoFields;
            Json::Object json_obj;
            json_obj.insert(VERSION, Json::Value::string(compiler_info.version));
            json_obj.insert(NAME, Json::Value::string(compiler_info.name));
            json_obj.insert(C_FULL_PATH, Json::Value::string(compiler_info.c_full_path.generic_u8string()));
            json_obj.insert(CXX_FULL_PATH, Json::Value::string(compiler_info.cxx_full_path.generic_u8string()));
            return json_obj;
        }
    }

    CompilerInfo CompilerInfo::load(const Filesystem& filesystem, const Path& path)
    {
        std::error_code ec;
        auto compiler_config_opt = Json::parse_file(filesystem, path, ec);
        if (ec || !compiler_config_opt.has_value())
        {
            Checks::exit_with_message(
                VCPKG_LINE_INFO, "Failed to parse compiler config at %s:\n%s", path, ec.message());
        }

        auto compiler_config_value = std::move(compiler_config_opt).value_or_exit(VCPKG_LINE_INFO);
        if (!compiler_config_value.first.is_object())
        {
            Checks::exit_with_message(
                VCPKG_LINE_INFO,
                "Failed to parse compiler config at %s:\nCompiler Config files must have a top-level object\n",
                path);
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

    void CompilerInfo::dump(Filesystem& filesystem, const Path& path, const CompilerInfo& instance)
    {
        auto serialized = details::serialize(instance);
        Json::dump_file(filesystem, path, serialized, Json::JsonStyle::with_spaces(4), VCPKG_LINE_INFO);
    }

    size_t CompilerInfo::hash_code() const
    {
        auto const fn_str_hash = std::hash<std::string>();
        size_t hash = 31;
        hash = hash * 17 + fn_str_hash(name);
        hash = hash * 17 + fn_str_hash(version);
        hash = hash * 17 + fn_str_hash(c_full_path.generic_u8string());
        hash = hash * 17 + fn_str_hash(cxx_full_path.generic_u8string());
        return hash;
    }

}
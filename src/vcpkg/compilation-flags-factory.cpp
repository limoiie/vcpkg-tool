#include <vcpkg/base/util.h>

#include <vcpkg/compilation-flags-factory.h>
#include <vcpkg/compile-triplet.h>
#include <vcpkg/compiler-info.h>
#include <vcpkg/triplet.h>

namespace vcpkg::bin2sth
{
    namespace details
    {
        static ConfigFlags parse_optimization_flags(CompilerInfo const& compiler_info, std::string const& optimization);

        static ConfigFlags parse_obfuscation_flags(CompilerInfo const& compiler_info, std::string const& obfuscation);
    }

    CompilationFlagsFactory::CompilationFlagsFactory(const std::map<std::string, CompilerInfo>& compilers)
        : m_compilers(compilers)
    {
    }

    CompilationFlags::CompilationFlags(CompilerInfo const& compiler_info,
                                       ConfigFlags&& optimization,
                                       ConfigFlags&& obfuscation)
        : m_compiler_info(compiler_info), m_optimization(std::move(optimization)), m_obfuscation(std::move(obfuscation))
    {
    }

    std::string CompilationFlags::c_compiler_full_path() const
    {
        return m_compiler_info.c_full_path.generic_u8string();
    }
    std::string CompilationFlags::cxx_compiler_full_path() const
    {
        return m_compiler_info.cxx_full_path.generic_u8string();
    }

    std::string CompilationFlags::make_cxx_flags() const
    {
        return std::string(m_optimization.flags).append(" ").append(m_obfuscation.flags);
    }

    std::string CompilationFlags::make_c_flags() const
    {
        return std::string(m_optimization.flags).append(" ").append(m_obfuscation.flags);
    }

    CompilationFlags CompilationFlagsFactory::interpret(const CompileTriplet& config) const
    {
        auto const compiler_info_itr = m_compilers.find(config.compiler_tag);

        Checks::check_exit(VCPKG_LINE_INFO,
                           compiler_info_itr != m_compilers.end(),
                           std::string("Invalid compiler nickname: ").append(config.compiler_tag));

        auto const& compiler_info = compiler_info_itr->second;
        auto optimization_flags = details::parse_optimization_flags(compiler_info, config.optimization_tag);
        auto obfuscation_flags = details::parse_obfuscation_flags(compiler_info, config.obfuscation_tag);

        return CompilationFlags{compiler_info, std::move(optimization_flags), std::move(obfuscation_flags)};
    }

    namespace details
    {
        ConfigFlags parse_optimization_flags(CompilerInfo const&, std::string const& optimization)
        {
            constexpr char const* supported_optimizations[] = {"O0", "O1", "O2", "O3", "Os"};
            for (auto const& supported_opt : supported_optimizations)
            {
                if (supported_opt == optimization)
                {
                    return {optimization, std::string("-").append(supported_opt)};
                }
            }

            Checks::exit_with_message(VCPKG_LINE_INFO,
                                      std::string("Error: Invalid optimization: ").append(optimization));
        }

        ConfigFlags parse_obfuscation_flags(CompilerInfo const&, std::string const& obf)
        {
            if (obf == DEFAULT_OBFUSCATION_TAG) return {obf, ""};

            std::string obfuscation_flags;
            if (obf.size() == 2 && std::isalpha(obf[0]) && std::isdigit(obf[1]))
            {
                auto const loop_num = std::to_string(static_cast<unsigned>(obf[1] - '0'));
                switch (obf[0])
                {
                    case 'S': return {obf, Strings::format("-mllvm -sub -mllvm -sub_loop=%s", loop_num)};
                    case 'B': return {obf, Strings::format("-mllvm -bcf -mllvm -bcf_loop=%s", loop_num)};
                    case 'F': return {obf, Strings::format("-mllvm -fla -mllvm -split -mllvm -split_num=%s", loop_num)};
                }
            }

            Checks::exit_with_message(VCPKG_LINE_INFO, std::string("Error: Invalid obfuscation: ").append(obf));
        }
    }

    size_t ConfigFlags::hash_code() const
    {
        auto const fn_str_hash = std::hash<std::string>();
        size_t hash = 31;
        hash = hash * 17 + fn_str_hash(name);
        hash = hash * 17 + fn_str_hash(flags);
        return hash;
    }
}
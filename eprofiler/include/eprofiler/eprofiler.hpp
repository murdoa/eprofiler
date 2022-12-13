#ifndef EPROFILER_EPROFILER_HPP
#define EPROFILER_EPROFILER_HPP

#include <eprofiler/stringconstant.hpp>

namespace eprofiler {

namespace internal {


template<std::size_t N>
struct EProfilerName {
    char name[N];

    constexpr EProfilerName(const char (&name_)[N]) {
        std::copy_n(name_, N, name);
    }
}; // struct EProfilerName


template<EProfilerName profilerName>
class EProfiler {
public:
    // StringConstant is used to create unique string literal tags for each profiler
    template<class CharT, CharT... Chars>
    struct StringConstant_WithID : public StringConstant<CharT, Chars...> {
        // declared in a generated translation unit
        std::size_t to_id() const noexcept;
    };

    // Convert StringConstant to StringConstant_WithID
    template<class CharT, CharT... Chars>
    static StringConstant_WithID<CharT, Chars...> convert_string_constant(StringConstant<CharT, Chars...> const& sc) noexcept {
        return StringConstant_WithID<CharT, Chars...>{sc};
    }

private:

    // Private functions operating on StringConstant_WithID tags

    template<class CharT, CharT... Chars>
    std::size_t operator[](StringConstant_WithID<CharT, Chars...> const& str) const noexcept {
        return str.to_id();
    }

public:

    constexpr std::string_view name() const noexcept {
        return profilerName.name;
    }

    // Public functions operating on StringConstant tags
    template<class CharT, CharT... Chars>
    std::size_t operator[](StringConstant<CharT, Chars...> const& str) const noexcept {
        return (*this)[convert_string_constant(str)];
    }


}; // class EProfiler



} // namespace internal
} // namespace eprofiler

#define EPROFILER_GET_PROFILER_TYPE(name) eprofiler::internal::EProfiler<eprofiler::internal::EProfilerName{name}>
#define EPROFILER_GET_PROFILER(name) EPROFILER_GET_PROFILER_TYPE(name){}

#endif
#ifndef EPROFILER_EPROFILER_HPP
#define EPROFILER_EPROFILER_HPP

#include <span>
#include <eprofiler/linktimehashtable.hpp>

namespace eprofiler {

namespace detail {

template<std::size_t N>
struct EProfilerName {
    char name[N];

    constexpr EProfilerName(const char (&name_)[N]) {
        std::copy_n(name_, N, name);
    }
}; // struct EProfilerName

} // namespace detail

template<detail::EProfilerName profilerName, class T>
class EProfiler : protected LinkTimeHashTable<EProfiler<profilerName, T>, T> {
public:
    constexpr static std::string_view name() noexcept {
        return profilerName.name;
    }

    // Public functions operating on StringConstant tags
    template<class CharT, CharT... Chars>
    std::size_t operator[](StringConstant<CharT, Chars...> const& str) const noexcept {
        return this->at(str);
    }

    template<class CharT, CharT... Chars>
    std::size_t get_id(StringConstant<CharT, Chars...> const& str) const noexcept {
        return LinkTimeHashTable<EProfiler<profilerName, T>, T>::get_id(str);
    }

}; // class EProfiler


} // namespace eprofiler

#endif
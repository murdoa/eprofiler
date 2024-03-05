#ifndef EPROFILER_EPROFILER_HPP
#define EPROFILER_EPROFILER_HPP

#include <algorithm>
#include <span>

class EProfiler;

#include <eprofiler/linktimehashtable.hpp>

#include "stringconstant.hpp"

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

template<detail::EProfilerName ProfilerName, class IDTypeT, class ValueType>
class EProfiler : protected LinkTimeHashTable<EProfiler<ProfilerName, IDTypeT, ValueType>, IDTypeT, ValueType> {
public:
    using IDType = IDTypeT;

    constexpr static std::string_view name() noexcept {
        return ProfilerName.name;
    }

    // Public functions operating on StringConstant tags
    template<class CharT, CharT... Chars>
    IDType operator[](StringConstant<CharT, Chars...> const& str) const noexcept {
        return this->at(str);
    }

    template<class CharT, CharT... Chars>
    IDType get_id(StringConstant<CharT, Chars...> const& str) const noexcept {
        return LinkTimeHashTable<EProfiler<ProfilerName, IDTypeT, ValueType>, IDTypeT, ValueType>::get_id(str);
    }

}; // class EProfiler


} // namespace eprofiler

#endif
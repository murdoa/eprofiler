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
    using LinkTimeHashTableT = LinkTimeHashTable<EProfiler<ProfilerName, IDTypeT, ValueType>, IDTypeT, ValueType>;

    constexpr static std::string_view name() noexcept {
        return ProfilerName.name;
    }

    static IDType get_offset() noexcept {
        return LinkTimeHashTableT::offset;
    }

    // Public functions operating on StringConstant tags
    template<class CharT, CharT... Chars>
    ValueType& operator[](StringConstant<CharT, Chars...> const& str) const noexcept {
        return this->at(str);
    }

    template<class CharT, CharT... Chars>
    IDType get_id(StringConstant<CharT, Chars...> const& str) const noexcept {
        return LinkTimeHashTableT::get_id(str);
    }

}; // class EProfiler


} // namespace eprofiler

#endif
#ifndef EPROFILER_EPROFILER_HPP
#define EPROFILER_EPROFILER_HPP

#include <algorithm>
#include <concepts>
#include <optional>
#include <span>
#include <tuple>

#include <eprofiler/uniquetype.hpp>
#include <eprofiler/linktimehashtable.hpp>

#include "stringconstant.hpp"

namespace eprofiler {

template<std::size_t N>
struct EProfilerTag {
    char name[N];

    constexpr EProfilerTag(const char (&name_)[N]) {
        std::copy_n(name_, N, name);
    }
}; // struct EProfilerTag

template<EProfilerTag ProfilerTag, std::integral IndexT, class SteadyClock>
class EProfiler : protected LinkTimeHashTable<EProfiler<ProfilerTag, IndexT, typename SteadyClock::time_point>, IndexT, typename SteadyClock::time_point> {
    using LinkTimeHashTableT = LinkTimeHashTable<EProfiler<ProfilerTag, IndexT, typename SteadyClock::time_point>, IndexT, typename SteadyClock::time_point>;
public:
    using index_type = IndexT;
    using time_point = typename SteadyClock::time_point;

    template<class CharT, CharT... Chars>
    static void set_time(StringConstant<CharT, Chars...> const tag) noexcept {
        LinkTimeHashTableT::at(tag) = SteadyClock::now();
    }

    template<class CharT, CharT... Chars>
    static time_point& get_time(StringConstant<CharT, Chars...> const tag) noexcept {
        return LinkTimeHashTableT::at(tag);
    }

    template<class CharT1, CharT1... Chars1, class CharT2, CharT2... Chars2>
    static auto get_duration(StringConstant<CharT1, Chars1...> const start, StringConstant<CharT2, Chars2...> const end) noexcept {
        return LinkTimeHashTableT::at(end) - LinkTimeHashTableT::at(start);
    }

}; // class EProfiler


} // namespace eprofiler

#endif
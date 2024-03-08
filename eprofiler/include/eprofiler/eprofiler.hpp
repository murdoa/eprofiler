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
class EProfiler : protected LinkTimeHashTable<EProfiler<ProfilerTag, IndexT, typename SteadyClock::time_point>, IndexT, std::tuple<std::optional<IndexT>, typename SteadyClock::time_point>> {
    using LinkTimeHashTableT = LinkTimeHashTable<EProfiler<ProfilerTag, IndexT, typename SteadyClock::time_point>, IndexT, std::tuple<std::optional<IndexT>, typename SteadyClock::time_point>>;
public:
    using index_type = IndexT;
    using time_point = typename SteadyClock::time_point;
    using duration = typename SteadyClock::duration;

    template<class CharT, CharT... Chars>
    static void set_time(StringConstant<CharT, Chars...> const tag) noexcept {
        auto& [index, time] = LinkTimeHashTableT::at(tag);
        index = std::optional<IndexT>{};
        time = SteadyClock::now();
    }

    template<class CharT1, CharT1... Chars1, class CharT2, CharT2... Chars2>
    static void set_time(StringConstant<CharT1, Chars1...> const ref, StringConstant<CharT2, Chars2...> const tag) noexcept {
        auto& [index, time] = LinkTimeHashTableT::at(tag);
        index = std::optional<IndexT>(LinkTimeHashTableT::convert_string_constant(ref).to_id());
        time = SteadyClock::now();
    }

    template<class CharT, CharT... Chars>
    static time_point& get_time(StringConstant<CharT, Chars...> const tag) noexcept {
        return std::get<1>(LinkTimeHashTableT::at(tag));
    }

    template<class CharT, CharT... Chars>
    static duration& get_duration(StringConstant<CharT, Chars...> const tag) noexcept {
        auto& [ref_index, time] = LinkTimeHashTableT::at(tag);

        if (ref_index.has_value()) {
            return time - LinkTimeHashTableT::at(ref_index.value());
        } else {
            return duration{};
        }
    }

    static duration get_duration(const IndexT idx) noexcept {
        auto& [ref_index, time] = LinkTimeHashTableT::at(idx);

        if (ref_index.has_value()) {
            return time - std::get<1>(LinkTimeHashTableT::at(ref_index.value()));
        } else {
            return duration{};
        }
    }

    template<class CharT1, CharT1... Chars1, class CharT2, CharT2... Chars2>
    static auto get_duration(StringConstant<CharT1, Chars1...> const start, StringConstant<CharT2, Chars2...> const end) noexcept {
        return std::get<1>(LinkTimeHashTableT::at(end)) - std::get<1>(LinkTimeHashTableT::at(start));
    }

    static std::span<const std::string_view> keys() noexcept {
        return LinkTimeHashTableT::keys;
    }

    static auto& value_store() noexcept {
        return LinkTimeHashTableT::value_store;
    }

    static IndexT offset() noexcept {
        return LinkTimeHashTableT::offset;
    }
}; // class EProfiler


} // namespace eprofiler

#endif
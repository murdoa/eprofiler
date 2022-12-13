#ifndef EPROFILER_STRING_CONSTANT_HPP
#define EPROFILER_STRING_CONSTANT_HPP

#include <string_view>

namespace eprofiler {

template <class CharT, CharT... Chars>
class string_constant {
    constexpr static CharT arr[sizeof...(Chars)] { Chars... };

public:
    constexpr static std::basic_string_view<CharT> value { arr, sizeof...(Chars) };

    // Implicit conversion to std::basic_string_view<CharT>
    constexpr operator std::basic_string_view<CharT>() const noexcept { return value; };
    // Explicit conversion to std::basic_string_view<CharT>
    constexpr std::basic_string_view<CharT> as_string_view() const noexcept { return value; };
};

namespace literals {

    // User-defined literal for string_constant
    template <class T, T... Chars>
    constexpr string_constant<T, Chars...> operator""_sc() {
        return {};
    }

} // namespace literals

} // namespace eprofiler

#endif
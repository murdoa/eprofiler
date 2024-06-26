#ifndef EPROFILER_UNIQUE_TYPE_HPP
#define EPROFILER_UNIQUE_TYPE_HPP

#include <algorithm>
#include <cstdint>

namespace eprofiler {
namespace detail {

// UniqueTypeInfo is helper struct to store __FILE__ and __LINE__ in constexpr context
// This is then used with C++20 literal type template parameters to create unique types
// Cannot use std::source_location because it is not constexpr in C++20
template<std::size_t N>
struct UniqueTypeInfo {
    char fileName[N];
    std::size_t lineNo;

    constexpr UniqueTypeInfo(const char (&fileName_)[N], const std::size_t lineNo_) : lineNo{lineNo_} {
        std::copy_n(fileName_, N, fileName);
    }
};

} // namespace detail

template <detail::UniqueTypeInfo Info>
struct UniqueType {
};

} // namespace eprofiler

// Must use macro on different lines to generate unique types, type must take a UniqueTypeInfo as template parameter
#define EPROFILER_UNIQUE_TYPE_CUSTOM(Type) Type<eprofiler::detail::UniqueTypeInfo{__FILE__, __LINE__}>

// Must use macro on different lines to generate unique types, type must take a UniqueTypeInfo as last template parameter
#define EPROFILER_UNIQUE_TYPE_CUSTOM_TMPL(Type, ...) Type<__VA_ARGS__, eprofiler::detail::UniqueTypeInfo{__FILE__, __LINE__}>

// Must use macro on different lines to generate unique types, type must take a UniqueTypeInfo as template parameter
#define EPROFILER_UNIQUE_TYPE() EPROFILER_UNIQUE_TYPE_CUSTOM(eprofiler::UniqueType)

#endif
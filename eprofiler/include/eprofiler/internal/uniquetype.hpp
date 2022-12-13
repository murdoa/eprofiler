#ifndef EPROFILER_INTERNAL_UNIQUE_TYPE_HPP
#define EPROFILER_INTERNAL_UNIQUE_TYPE_HPP

#include <cstdint>

namespace eprofiler {
namespace internal {

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

template<UniqueTypeInfo UniqueInfo, class T>
struct UniqueTypeWrapper : public T { };

} // namespace internal
} // namespace eprofiler

// Must use macro on different lines to generate unique types
#define EPROFILER_UNIQUE_TYPE(Type) eprofiler::internal::UniqueTypeWrapper<eprofiler::internal::UniqueTypeInfo{__FILE__, __LINE__}, Type>

#endif
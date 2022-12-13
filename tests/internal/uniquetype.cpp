
#include <catch2/catch_test_macros.hpp>

#include <eprofiler/internal/uniquetype.hpp>

template <eprofiler::internal::UniqueTypeInfo UniqueInfo>
struct A { };

TEST_CASE("Test on class with single template param", "[UniqueType]") {
    // Must use macro on different lines to generate unique types
    using t1 = EPROFILER_UNIQUE_TYPE(A);
    using t2 = EPROFILER_UNIQUE_TYPE(A);

    // Require types are unique
    STATIC_REQUIRE(!std::is_same_v<t1, t2>);
}

template <typename U, typename V, eprofiler::internal::UniqueTypeInfo UniqueInfo>
struct B { };

TEST_CASE("Test on class with multiple template params", "[UniqueType]") {
    // Must use macro on different lines to generate unique types
    using t1 = EPROFILER_UNIQUE_TYPE_TMPL(B, int, float);
    using t2 = EPROFILER_UNIQUE_TYPE_TMPL(B, int, float);

    // Require types are unique
    STATIC_REQUIRE(!std::is_same_v<t1, t2>);
}

#include <catch2/catch_test_macros.hpp>

#include <eprofiler/internal/uniquetype.hpp>



TEST_CASE("Test on class with default constructor", "[UniqueType]") {
    struct A {};

    // Must use macro on different lines to generate unique types
    using t1 = EPROFILER_UNIQUE_TYPE(A);
    using t2 = EPROFILER_UNIQUE_TYPE(A);

    // Require types are unique
    STATIC_REQUIRE( !std::is_same_v<t1, t2> );
}
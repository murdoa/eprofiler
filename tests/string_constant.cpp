
#include <catch2/catch_test_macros.hpp>

#include <eprofiler/string_constant.hpp>

using namespace eprofiler::literals;

TEST_CASE("Verify constexpr operation", "[string_constant]") {
    constexpr auto test_constant = "ABC123"_sc;
    STATIC_REQUIRE(test_constant == std::string_view("ABC123"));
    STATIC_REQUIRE(test_constant.as_string_view() == std::string_view("ABC123"));
    STATIC_REQUIRE(test_constant.as_string_view() != std::string_view("ABC"));
}
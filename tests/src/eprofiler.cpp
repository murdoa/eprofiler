
#include <catch2/catch_test_macros.hpp>

#include <eprofiler/eprofiler.hpp>

using namespace eprofiler::literals;

TEST_CASE("Verify EPROFILER_GET_PROFILER", "[EProfiler]") {

    eprofiler::EProfiler<"Profiler1", int> profiler1{};

    // Verify macro returns unqiue types
    STATIC_REQUIRE(std::is_same_v<eprofiler::EProfiler<"Profiler1", int>, eprofiler::EProfiler<"Profiler1", int>>);
    STATIC_REQUIRE(std::is_same_v<eprofiler::EProfiler<"Profiler1", int>, eprofiler::EProfiler<"Profiler1", int>>);
    STATIC_REQUIRE(!std::is_same_v<eprofiler::EProfiler<"Profiler1", int>, eprofiler::EProfiler<"Profiler2", int>>);

    // Verify macro returns EProfiler with correct name
    STATIC_REQUIRE(eprofiler::EProfiler<"Profiler1", int>::name() == "Profiler1");
    STATIC_REQUIRE(eprofiler::EProfiler<"Profiler2", int>::name() == "Profiler2");
}


TEST_CASE("Test StringConstant to ID conversion", "[EProfiler]") {
    const auto profiler1 = eprofiler::EProfiler<"Test-ID-Conversion", int>{};

    REQUIRE( profiler1["0A"_sc] == 0 );
    REQUIRE( profiler1["0B"_sc] == 1 );
}
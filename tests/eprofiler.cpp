
#include <catch2/catch_test_macros.hpp>

#include <eprofiler/eprofiler.hpp>

using namespace eprofiler::literals;

TEST_CASE("Verify EPROFILER_GET_PROFILER", "[EProfiler]") {

    // Verify macro returns unqiue types
    STATIC_REQUIRE(std::is_same_v<EPROFILER_GET_PROFILER_TYPE("Profiler1"), EPROFILER_GET_PROFILER_TYPE("Profiler1")>);
    STATIC_REQUIRE(!std::is_same_v<EPROFILER_GET_PROFILER_TYPE("Profiler1"), EPROFILER_GET_PROFILER_TYPE("Profiler2")>);

    // Verify macro returns EProfiler with correct name
    STATIC_REQUIRE(EPROFILER_GET_PROFILER("Profiler1").name() == "Profiler1");
    STATIC_REQUIRE(EPROFILER_GET_PROFILER("Profiler2").name() == "Profiler2");
}

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <thread>

#include <eprofiler/eprofiler.hpp>
using namespace eprofiler::literals;

TEST_CASE("Verify EProfiler interface functionality", "[EProfiler]") {
    struct SteadyClock {
        using time_point = int;

        static time_point now(bool reset=false) {
            static int current_time = 0;
            if (reset) {
                current_time = 0;
            }
            return current_time++;
        }
    };

    using EProfiler = eprofiler::EProfiler<eprofiler::EProfilerTag{"Test"}, int, SteadyClock>;    

    SECTION("set_time and get_duration method") {
        SteadyClock::now(true);

        EProfiler::set_time("Tag1"_sc);
        EProfiler::set_time("Tag2"_sc);
        EProfiler::set_time("Tag3"_sc);

        REQUIRE(EProfiler::get_time("Tag1"_sc) == 1);
        REQUIRE(EProfiler::get_time("Tag2"_sc) == 2);
        REQUIRE(EProfiler::get_time("Tag3"_sc) == 3); 

        REQUIRE(EProfiler::get_duration("Tag1"_sc, "Tag1"_sc) == 0);
        REQUIRE(EProfiler::get_duration("Tag2"_sc, "Tag2"_sc) == 0);
        REQUIRE(EProfiler::get_duration("Tag3"_sc, "Tag3"_sc) == 0);

        REQUIRE(EProfiler::get_duration("Tag1"_sc, "Tag2"_sc) == 1);
        REQUIRE(EProfiler::get_duration("Tag1"_sc, "Tag3"_sc) == 2);
        REQUIRE(EProfiler::get_duration("Tag2"_sc, "Tag3"_sc) == 1);
    }
}


TEST_CASE("Verify EProfiler functionality with std::steady_clock", "[EProfiler]") {
    using EProfiler = eprofiler::EProfiler<eprofiler::EProfilerTag{"Test"}, int, std::chrono::steady_clock>;    

    SECTION("set_time and get_duration method") {

        const auto dur_cast = [](auto dur) {
            return std::chrono::duration_cast<std::chrono::milliseconds>(dur);
        };

        const auto start_time = std::chrono::steady_clock::now();
        EProfiler::set_time("Tag1"_sc);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        EProfiler::set_time("Tag2"_sc);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        EProfiler::set_time("Tag3"_sc);

        // Verify logged times are within 1ms of expected
        REQUIRE(EProfiler::get_time("Tag1"_sc) - start_time < std::chrono::milliseconds(1));
        REQUIRE(EProfiler::get_time("Tag2"_sc) - start_time < std::chrono::milliseconds(21));
        REQUIRE(EProfiler::get_time("Tag3"_sc) - start_time < std::chrono::milliseconds(31));

        // Verify time ordering
        REQUIRE(EProfiler::get_time("Tag1"_sc) < EProfiler::get_time("Tag2"_sc));
        REQUIRE(EProfiler::get_time("Tag2"_sc) < EProfiler::get_time("Tag3"_sc));

        // Verify duration calculations
        REQUIRE(EProfiler::get_duration("Tag1"_sc, "Tag1"_sc) == std::chrono::steady_clock::duration::zero());
        REQUIRE(EProfiler::get_duration("Tag2"_sc, "Tag2"_sc) == std::chrono::steady_clock::duration::zero());
        REQUIRE(EProfiler::get_duration("Tag3"_sc, "Tag3"_sc) == std::chrono::steady_clock::duration::zero());

        REQUIRE(dur_cast(EProfiler::get_duration("Tag1"_sc, "Tag2"_sc)) == std::chrono::milliseconds(10));
        REQUIRE(dur_cast(EProfiler::get_duration("Tag1"_sc, "Tag3"_sc)) == std::chrono::milliseconds(20));
        REQUIRE(dur_cast(EProfiler::get_duration("Tag2"_sc, "Tag3"_sc)) == std::chrono::milliseconds(10));
    }
}
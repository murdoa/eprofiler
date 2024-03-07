#include <catch2/catch_test_macros.hpp>

#include <eprofiler/eprofiler.hpp>

using namespace eprofiler::literals;

#include <spdlog/fmt/compile.h>

TEST_CASE("EProfilerName construction", "[EProfilerName]") {
    constexpr auto name = eprofiler::detail::EProfilerName{"Profiler"};
    REQUIRE(std::string_view{name.name} == "Profiler");
}

TEST_CASE("Verify EProfiler class functionality", "[EProfiler]") {
    using Profiler = eprofiler::EProfiler<"Profiler1", int, int>;

    const auto profiler = Profiler{};

    SECTION("operator[] and at method") {

        profiler["Tag1"_sc] = 17;

        REQUIRE(profiler["Tag1"_sc] == 17);
        REQUIRE(profiler.at("Tag1"_sc) == 17);
        REQUIRE(Profiler::at("Tag1"_sc) == 17);

        profiler.at("Tag1"_sc) = 42;

        REQUIRE(profiler["Tag1"_sc] == 42);
        REQUIRE(profiler.at("Tag1"_sc) == 42);
        REQUIRE(Profiler::at("Tag1"_sc) == 42);
    }
}

TEST_CASE("Verify unique id's from linked library", "[EProfiler]") {
    using Profiler1 = eprofiler::EProfiler<"Profiler1", int, int>;
    using Profiler2 = eprofiler::EProfiler<"Profiler2", int, int>;

    constexpr auto profiler1_tag1 = Profiler1::StringConstant_WithID{"Tag1"_sc};
    constexpr auto profiler1_tag2 = Profiler1::StringConstant_WithID{"Tag2"_sc};

    constexpr auto profiler2_tag1 = Profiler2::StringConstant_WithID{"Tag1"_sc};
    constexpr auto profiler2_tag2 = Profiler2::StringConstant_WithID{"Tag2"_sc};

    // Check that the tags are unique
    REQUIRE(profiler1_tag1.to_id() != profiler1_tag2.to_id());
    REQUIRE(profiler2_tag1.to_id() != profiler2_tag2.to_id());

    // Check that the tags are unique between profilers
    REQUIRE(profiler1_tag1.to_id() != profiler2_tag1.to_id());
    REQUIRE(profiler1_tag2.to_id() != profiler2_tag2.to_id());

    // Check that the offsets are unique
    REQUIRE(Profiler1::get_offset() != Profiler2::get_offset());
}

TEST_CASE("Verify linked library value_store generation", "[EProfiler]") {
    using Profiler = eprofiler::EProfiler<"Profiler", int, int>;

    // Verify value_store operation
    Profiler::at("Tag_1"_sc) = 0;
    Profiler::at("Tag_2"_sc) = 0;

    REQUIRE(Profiler::at("Tag_1"_sc) == 0);
    REQUIRE(Profiler::at("Tag_2"_sc) == 0);

    Profiler::at("Tag_1"_sc) = 1;
    Profiler::at("Tag_2"_sc) = 2;

    REQUIRE(Profiler::at("Tag_1"_sc) == 1);
    REQUIRE(Profiler::at("Tag_2"_sc) == 2);
}
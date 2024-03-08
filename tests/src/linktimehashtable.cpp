#include <catch2/catch_test_macros.hpp>

#include <eprofiler/eprofiler.hpp>

using namespace eprofiler::literals;

TEST_CASE("Verify LinkTimeHashTable class functionality", "[LinkTimeHashTable]") {
    using LinkTimeHashTable = eprofiler::LinkTimeHashTable<void, int, int>;

    const auto hashtable = LinkTimeHashTable{};

    SECTION("operator[] and at method") {

        hashtable["Tag1"_sc] = 17;

        REQUIRE(hashtable["Tag1"_sc] == 17);
        REQUIRE(hashtable.at("Tag1"_sc) == 17);
        REQUIRE(LinkTimeHashTable::at("Tag1"_sc) == 17);

        hashtable.at("Tag1"_sc) = 42;

        REQUIRE(hashtable["Tag1"_sc] == 42);
        REQUIRE(hashtable.at("Tag1"_sc) == 42);
        REQUIRE(LinkTimeHashTable::at("Tag1"_sc) == 42);
    }
}

TEST_CASE("Verify unique id's from linked library", "[LinkTimeHashTable]") {
    using LinkTimeHashTable1 = eprofiler::LinkTimeHashTable<EPROFILER_UNIQUE_TYPE(), int, int>;
    using LinkTimeHashTable2 = eprofiler::LinkTimeHashTable<EPROFILER_UNIQUE_TYPE(), int, int>;

    constexpr auto hashtable1_tag1 = LinkTimeHashTable1::StringConstant_WithID{"Tag1"_sc};
    constexpr auto hashtable1_tag2 = LinkTimeHashTable1::StringConstant_WithID{"Tag2"_sc};

    constexpr auto hashtable2_tag1 = LinkTimeHashTable2::StringConstant_WithID{"Tag1"_sc};
    constexpr auto hashtable2_tag2 = LinkTimeHashTable2::StringConstant_WithID{"Tag2"_sc};

    // Check that the intra-table tags are unique
    REQUIRE(hashtable1_tag1.to_id() != hashtable1_tag2.to_id());
    REQUIRE(hashtable2_tag1.to_id() != hashtable2_tag2.to_id());

    // Check that the inter-table tags are unique
    REQUIRE(hashtable1_tag1.to_id() != hashtable2_tag1.to_id());
    REQUIRE(hashtable1_tag1.to_id() != hashtable2_tag2.to_id());
    REQUIRE(hashtable1_tag2.to_id() != hashtable2_tag1.to_id());
    REQUIRE(hashtable1_tag2.to_id() != hashtable2_tag2.to_id());

    // Check that the offsets are unique
    REQUIRE(LinkTimeHashTable1::offset != LinkTimeHashTable2::offset);
}

TEST_CASE("Verify linked library value_store generation", "[LinkTimeHashTable]") {
    using LinkTimeHashTable = eprofiler::LinkTimeHashTable<void, int, int>;

    // Verify value_store operation
    LinkTimeHashTable::at("Tag_1"_sc) = 0;
    LinkTimeHashTable::at("Tag_2"_sc) = 0;

    REQUIRE(LinkTimeHashTable::at("Tag_1"_sc) == 0);
    REQUIRE(LinkTimeHashTable::at("Tag_2"_sc) == 0);

    LinkTimeHashTable::at("Tag_1"_sc) = 1;
    LinkTimeHashTable::at("Tag_2"_sc) = 2;

    REQUIRE(LinkTimeHashTable::at("Tag_1"_sc) == 1);
    REQUIRE(LinkTimeHashTable::at("Tag_2"_sc) == 2);
}
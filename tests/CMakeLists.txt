include(CTest)

file(GLOB_RECURSE TEST_SOURCES "**.cpp")
add_executable(eprofiler_tests ${TEST_SOURCES})
target_link_libraries(eprofiler_tests PRIVATE eprofiler)

ADD_LIB(LIB_ID          Catch2::Catch2
        SUBDIR_NAME     "Catch2"
        GIT_REPOSITORY  https://github.com/catchorg/Catch2.git
        GIT_TAG         b9baae6d938133ff2fdc938053e88ccf51dd3afe)

target_link_libraries(eprofiler_tests PRIVATE Catch2::Catch2WithMain)
target_include_directories(eprofiler_tests SYSTEM PRIVATE ${Catch2IncludeDirs})
get_target_property(Catch2IncludeDirs Catch2::Catch2 INTERFACE_INCLUDE_DIRECTORIES)
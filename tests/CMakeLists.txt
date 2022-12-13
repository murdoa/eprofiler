include(CTest)

file(GLOB_RECURSE TEST_SOURCES "src/**.cpp")

add_library(tests_lib OBJECT ${TEST_SOURCES})
target_link_libraries(tests_lib PUBLIC eprofiler_base)

ADD_LIB(LIB_ID          Catch2::Catch2
        SUBDIR_NAME     "Catch2"
        GIT_REPOSITORY  https://github.com/catchorg/Catch2.git
        GIT_TAG         b9baae6d938133ff2fdc938053e88ccf51dd3afe)

target_link_libraries(tests_lib PUBLIC Catch2::Catch2WithMain)

REGISTER_EPROFILER_TARGET( TARGET_IN tests_lib 
                           TARGET_GEN tests_lib_gen )

add_executable(eprofiler_tests $<TARGET_OBJECTS:tests_lib_gen>)
target_link_libraries(eprofiler_tests PUBLIC tests_lib)
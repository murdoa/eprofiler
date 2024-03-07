
file(GLOB_RECURSE EXAMPLE_SOURCES "src/**.cpp")

add_library(example_app OBJECT ${EXAMPLE_SOURCES})
target_link_libraries(example_app PUBLIC eprofiler_base)

ADD_LIB(LIB_ID          spdlog::spdlog
        SUBDIR_NAME     "spdlog"
        GIT_REPOSITORY  https://github.com/gabime/spdlog.git
        GIT_TAG         5532231bbc31bbdf95ac15febdac0413ee1d07ad)

target_link_libraries(example_app PUBLIC spdlog::spdlog)

REGISTER_EPROFILER_TARGET( TARGET_IN example_app 
                           TARGET_GEN example_app_gen )

add_executable(eprofiler_example main.cpp)
target_link_libraries(eprofiler_example PUBLIC    example_app
                                                  example_app_gen)

file(GLOB_RECURSE EXAMPLE_SOURCES "src/**.cpp")

add_library(example_app OBJECT ${EXAMPLE_SOURCES})
target_link_libraries(example_app PUBLIC eprofiler_base)

REGISTER_EPROFILER_TARGET( TARGET_IN example_app 
                           TARGET_GEN example_app_gen )

add_executable(eprofiler_example main.cpp)
target_link_libraries(eprofiler_example PUBLIC    example_app
                                                  example_app_gen)
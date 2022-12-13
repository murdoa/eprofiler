# 📁 Set variable for source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 🏗️ Add eprofile library from SOURCES
add_library(eprofiler ${SOURCES})
add_library(murdoa::eprofiler ALIAS eprofiler)

# 📂 Set include directories for eprofiler library
target_include_directories(eprofiler PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

# 🔗 Link global project libraries to eprofiler
target_link_libraries(eprofiler PRIVATE
                      project_libs project_options project_warnings)
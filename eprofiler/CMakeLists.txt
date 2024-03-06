# 📁 Set variable for source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 🏗️ Add eprofile base library from SOURCES
add_library(eprofiler_base ${SOURCES})
add_library(murdoa::eprofiler_base ALIAS eprofiler_base)

# 📂 Set include directories for eprofiler base library
target_include_directories(eprofiler_base PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

# 🔗 Link global project libraries to eprofiler base
target_link_libraries(eprofiler_base PRIVATE
                      project_libs project_options project_warnings)

# Function to register eprofiler targets
# This function will create a new target which will generate the unresolved symbols
# Usage:
#   register_eprofiler_target(
#       TARGET_IN <target_in>
#       TARGET_GEN <target_gen>
#   )
function(REGISTER_EPROFILER_TARGET)
    cmake_parse_arguments(
        EPROFILER # PREFIX
        "" # BOOLEAN
        "TARGET_IN;TARGET_GEN" # MONOVALUES
        "" # MULTIVALUES
        ${ARGN} #ARGUMENTS
    )

    message(STATUS "Registering eprofiler target ${EPROFILER_TARGET_IN} -> ${EPROFILER_TARGET_GEN}")
    message(STATUS "CMAKE_CURRENT_FUNCTION_LIST_DIR:")

    SET(EPROFILER_INTERMEDIATE_TARGET ${EPROFILER_TARGET_IN}_gen_build_step)

    add_library(${EPROFILER_INTERMEDIATE_TARGET} STATIC $<TARGET_OBJECTS:${EPROFILER_TARGET_IN}>)

    add_custom_command(
        OUTPUT ${EPROFILER_INTERMEDIATE_TARGET}_gen.cpp ${EPROFILER_INTERMEDIATE_TARGET}_gen.cpp.json 
        COMMAND python3 ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/gen/gen.py ${EPROFILER_INTERMEDIATE_TARGET}_gen.cpp $<TARGET_FILE:${EPROFILER_INTERMEDIATE_TARGET}>
        DEPENDS ${EPROFILER_TARGET_IN}_gen_build_step
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # Generated targets
    add_library(${EPROFILER_TARGET_GEN} OBJECT ${CMAKE_CURRENT_BINARY_DIR}/${EPROFILER_INTERMEDIATE_TARGET}_gen.cpp)
    target_link_libraries(${EPROFILER_TARGET_GEN} PRIVATE eprofiler_base)

endfunction()


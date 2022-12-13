cmake_minimum_required(VERSION 3.1)

project(eprofiler CXX)

include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

set(CMAKE_CXX_STANDARD 20)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)

# Dev options
option(ENABLE_SANITIZER_ALL "Enables sanitizers" OFF)
option(ENABLE_PROTO "Relaxes unused warnings" OFF)

# External static analysis options
option(ENABLE_CPPCHECK ON)
option(ENABLE_CLANG_TIDY ON)
option(ENABLE_INCLUDE_WHAT_YOU_USE ON)

# Target options
option(BUILD_EXAMPLE "Build Example App" ON)
option(BUILD_TESTS "Build Tests" ON)

target_compile_options(project_options INTERFACE -O3)

if(ENABLE_PROTO)
target_compile_options(project_options INTERFACE -Wno-unused-variable)
target_compile_options(project_options INTERFACE -Wno-unused-parameter)
target_compile_options(project_options INTERFACE -Wno-unused-but-set-variable)
target_compile_options(project_options INTERFACE -Wno-useless-cast)
endif()

message(STATUS "Compiling with ${CMAKE_CXX_COMPILER_ID}")

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    target_compile_options(project_options INTERFACE -ftime-trace)
  endif()
elseif()
    target_compile_options(project_options INTERFACE -fconcepts)
endif()


# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)
# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# enable cache system
include(cmake/Cache.cmake)

# Add linker configuration
include(cmake/Linker.cmake)
configure_linker(project_options)

# Add Sanitiers 
if (ENABLE_SANITIZER_ALL)
    set(ENABLE_SANITIZER_ADDRESS ON)
    set(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR ON)
    set(ENABLE_SANITIZER_LEAK ON)
    set(ENABLE_SANITIZER_THREAD ON)
    set(ENABLE_SANITIZER_MEMORY ON)
endif()
# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# enable doxygen
include(cmake/Doxygen.cmake)
enable_doxygen()

# allow for static analysis options
include(cmake/StaticAnalyzers.cmake)

# Build Tests
if(BUILD_TESTS)
  enable_testing()
  message("Building Tests.")
  add_subdirectory(tests)
endif()

# Build App
if(BUILD_EXAMPLE)
message("Building example add.")
add_subdirectory(example)
endif()

# Build Library
add_subdirectory(src)

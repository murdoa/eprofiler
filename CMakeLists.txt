cmake_minimum_required(VERSION 3.17)

# Define eprofiler project with C++20
project(eprofiler CXX)
set(CMAKE_CXX_STANDARD 20)

###################################
#      CMake Project options      #
###################################

# 🔧 Dev options
option(ENABLE_PROTO "Prototyping mode (Relaxes unused and useless cast warnings)" OFF)

# 🔍 External static analysis options
option(ENABLE_CPPCHECK ON)
option(ENABLE_CLANG_TIDY ON)
option(ENABLE_INCLUDE_WHAT_YOU_USE ON)

# 🎯 Target options
option(BUILD_TESTS   "Build Tests" ON)
option(BUILD_EXAMPLE "Build Example App" OFF)

###################################
#      🔨 Compiler Options        #
###################################

# Add standard project settings and prevent in source builds
include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)
# Utility to fetch dependencies with FetchContent
include(cmake/AddLib.cmake)

# Use ccache if available
include(cmake/Cache.cmake)

# Add library project_options for compiler options
add_library(project_options INTERFACE)
target_compile_options(project_options INTERFACE -O3)

# Add library project_warnings for compiler warnings
add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# Add linker configuration
include(cmake/Linker.cmake)
configure_linker(project_options)

#  Disable certain warnings in prototype mode
if(ENABLE_PROTO)
  target_compile_options(project_options INTERFACE -Wno-unused-variable)
  target_compile_options(project_options INTERFACE -Wno-unused-parameter)
  target_compile_options(project_options INTERFACE -Wno-unused-but-set-variable)
  target_compile_options(project_options INTERFACE -Wno-useless-cast)
  target_compile_options(project_options INTERFACE -Wno-useless-cast)
endif()

#######################################
#  ⚙️ Add compiler-specific options   #
#######################################

message(STATUS "Compiling with ${CMAKE_CXX_COMPILER_ID}")
if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")

  # Enable compile time trace output for clang 
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    target_compile_options(project_options INTERFACE -ftime-trace)
  endif()

elseif()
  # Enable concepts for gcc
  target_compile_options(project_options INTERFACE -fconcepts)
endif()


#################################################
#  🏗️ Setup external libs for use with project  #
#################################################

add_library(project_libs INTERFACE)

#################################################
#  🔍 Configure static analysis and sanitizers  #
#################################################

# Include sanitizers if supported 
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

#####################################
#  📜 Configure documentation tools  #
#####################################

# setup doxygen if enabled with ENABLE_DOXYGEN
include(cmake/Doxygen.cmake)
enable_doxygen()

# setup static analysis tools if enabled
include(cmake/StaticAnalyzers.cmake)

###############################################
#  🚀 Enable Link Time Optimization Globally  #
###############################################

# Warning all targets created after this will have LTO enabled
# Include libraries before if you want to disable LTO for them

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_error)
if(lto_supported)
    message(STATUS "LTO supported and enabled.")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "LTO not supported: <${lto_error}>")
endif()

####################################
#  🛠️ Add CMake targets for build  #
####################################

# Add subdirectory for main eprofiler library
add_subdirectory(eprofiler)

# Add tests target if enabled
if(BUILD_TESTS)
  enable_testing()
  message("Building Tests.")
  add_subdirectory(tests)
endif()

# Add example target if enabled
if(BUILD_EXAMPLE)
message("Building example.")
add_subdirectory(example)
endif()

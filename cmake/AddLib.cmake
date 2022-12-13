include(FetchContent)

function(ADD_LIB)
    cmake_parse_arguments(
        ADD_LIB # PREFIX
        "" # BOOLEAN
        "LIB_ID;SUBDIR_NAME;GIT_REPOSITORY;GIT_TAG;MAKE_AVAILABLE" # MONOVALUES
        "" # MULTIVALUES
        ${ARGN} #ARGUMENTS
    )
    if(NOT ADD_LIB_LIB_ID)
        message(FATAL_ERROR "You must provide a library id (target alias)")
    endif()
    if(NOT ADD_LIB_SUBDIR_NAME)
        message(FATAL_ERROR "You must provide a library subdirectoy name")
    endif()
    if(NOT ADD_LIB_GIT_REPOSITORY)
        message(FATAL_ERROR "You must provide a git repository")
    endif()
    if(NOT ADD_LIB_GIT_TAG)
        message(FATAL_ERROR "You must provide a git tag")
    endif()

    if(NOT MAKE_AVAILABLE)
      set(MAKE_AVAILABLE TRUE)
    endif()

    if(NOT TARGET ${ADD_LIB_LIB_ID})
      message(STATUS "Library not found, including from deps: ${ADD_LIB_LIB_ID}")
      if(EXISTS "${PROJECT_SOURCE_DIR}/libs/${ADD_LIB_SUBDIR_NAME}")
        message(STATUS "${ADD_LIB_LIB_ID} found within project_dir/libs")
        add_subdirectory("${PROJECT_SOURCE_DIR}/libs/${ADD_LIB_SUBDIR_NAME}" "${PROJECT_BINARY_DIR}/libs/${ADD_LIB_SUBDIR_NAME}")
      elseif(EXISTS "${CMAKE_SOURCE_DIR}/libs/${ADD_LIB_SUBDIR_NAME}")
        message(STATUS "${ADD_LIB_LIB_ID} found within source_dir/libs")
        add_subdirectory("${CMAKE_SOURCE_DIR}/libs/${ADD_LIB_SUBDIR_NAME}" "${PROJECT_BINARY_DIR}/libs/${ADD_LIB_SUBDIR_NAME}")
      else()
        message(STATUS "Fetching ${ADD_LIB_LIB_ID} from git repo.")
        FetchContent_Declare(
          "${ADD_LIB_SUBDIR_NAME}_lib"
          GIT_REPOSITORY          ${ADD_LIB_GIT_REPOSITORY}
          GIT_TAG                 ${ADD_LIB_GIT_TAG}
          GIT_PROGRESS            TRUE
          USES_TERMINAL_DOWNLOAD  TRUE 
        )
        if(${MAKE_AVAILABLE})
          FetchContent_MakeAvailable("${ADD_LIB_SUBDIR_NAME}_lib")
        endif()
    endif()
    
  endif()
endfunction()
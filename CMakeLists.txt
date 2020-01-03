cmake_minimum_required (VERSION 3.2)

project(Traindb)

if( NOT DEFINED CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release CACHE STRING
       "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
       FORCE )
  message(STATUS "Setting build type - ${CMAKE_BUILD_TYPE}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

if(UNIX)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
endif(UNIX)

### external dependencies
find_package(Threads REQUIRED)

#### compiler-specific options
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DPOCO_DLL)
    add_compile_options("/std:c++latest")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /MP")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /MD")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG /OPT:REF /OPT:ICF")
endif()

if(UNIX)
    add_definitions(-DNDEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2 -std=c99 -Wall -D_DEFAULT_SOURCE")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -std=c++14 -lrt -lpthread")
endif()

#### setup external projects
set(EXT_PROJECTS_DIR "${PROJECT_SOURCE_DIR}/external")

add_subdirectory("${EXT_PROJECTS_DIR}/poco")
add_subdirectory("${EXT_PROJECTS_DIR}/gtest")

include_directories(
    "${POCO_INCLUDE_DIRS}"
    "${GTEST_INCLUDE_DIRS}"
    "${GMOCK_INCLUDE_DIRS}"
)

link_directories(
    "${POCO_LIBRARY_DIRS}"
    "${GTEST_LIBRARY_DIRS}"
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)


enable_testing()

# ENABLE WITH:  -DBUILD_PRODUCER=ON
option(BUILD_SERVER "Build server project" ON)
if(BUILD_SERVER)
    add_subdirectory(server)
endif()

# ENABLE WITH:  -DBUILD_CONSUMER=ON
option(DBUILD_CLIENT "Build client project" ON)
if(DBUILD_CLIENT)
    add_subdirectory(client)
endif()
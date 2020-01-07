cmake_minimum_required (VERSION 3.2)

project(Client CXX C)

include_directories(include src ../common "${POCO_INCLUDE_DIRS}")

file(GLOB_RECURSE CLIENT_SRC
         RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

file(GLOB_RECURSE COMMON_SRC
         RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_SOURCE_DIR}/common/*.hpp
         ${CMAKE_SOURCE_DIR}/common/*.cpp)

source_group(Common FILES ${COMMON_SRC})
source_group(Sources FILES ${CLIENT_SRC})

add_executable(${PROJECT_NAME} ${CLIENT_SRC})
add_dependencies(${PROJECT_NAME} poco)


if(UNIX)
    target_link_libraries(${PROJECT_NAME} ${POCO_LIBRARIES})
endif(UNIX)

target_link_libraries(
    ${PROJECT_NAME} 
    ${CMAKE_THREAD_LIBS_INIT}
)

set_target_properties(${PROJECT_NAME} 
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# ENABLE WITH:  -DBUILD_TESTS=ON
option (BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    include(client-tests.cmake)
endif()
﻿set(PROJECT_TEST_NAME ${PROJECT_NAME}.Test)

include_directories(include src tests ../common "${POCO_INCLUDE_DIRS}" "${GTEST_INCLUDE_DIRS}")

file(GLOB_RECURSE TEST_SRC
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hpp)

file(GLOB_RECURSE EXTERNAL_SRC
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

file(GLOB_RECURSE COMMON_SRC
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/common/*.hpp
        ${CMAKE_SOURCE_DIR}/common/*.cpp)

list(REMOVE_ITEM EXTERNAL_SRC src/main.cpp)

source_group(Common FILES ${COMMON_SRC})
source_group(Sources FILES ${EXTERNAL_SRC})
source_group(Tests FILES ${TEST_SRC})

add_executable(${PROJECT_TEST_NAME} ${TEST_SRC} ${EXTERNAL_SRC} ${COMMON_SRC})
add_dependencies(${PROJECT_TEST_NAME} poco googletest)


if(UNIX)
    target_link_libraries(${PROJECT_TEST_NAME} ${POCO_LIBRARIES})
endif(UNIX)

target_link_libraries(
    ${PROJECT_TEST_NAME}
    ${GTEST_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

set_target_properties(${PROJECT_TEST_NAME}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(NAME     ${PROJECT_TEST_NAME}
         COMMAND  ${PROJECT_TEST_NAME})

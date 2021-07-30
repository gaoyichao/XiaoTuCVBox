cmake_minimum_required(VERSION 2.8)
project(XiaoTuCVBox)

add_compile_options(-std=c++17)
set(CMAKE_BUILD_TYPE DEBUG)

message(STATUS "源码目录: " ${PROJECT_SOURCE_DIR})

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

file(GLOB PROJECT_SRC_FILES ./src/*cpp)
message("${PROJECT_SRC_FILES}")

add_library(${PROJECT_NAME} SHARED ${PROJECT_SRC_FILES})
target_link_libraries(${PROJECT_NAME} png)

############################################################################

enable_testing()

add_compile_definitions(XTCVB_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

add_executable(hello_test test/hello_test.cpp)
target_link_libraries(hello_test gtest gtest_main pthread rt ${PROJECT_NAME})

include(GoogleTest)
gtest_discover_tests(hello_test)


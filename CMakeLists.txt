cmake_minimum_required(VERSION 2.8)
project(XiaoTuCVBox)

add_compile_options(-std=c++17)
set(CMAKE_BUILD_TYPE DEBUG)

message(STATUS "源码目录: " ${PROJECT_SOURCE_DIR})

find_package(OpenCV 4.0.0)
message(STATUS "OpenCV头文件目录: " ${OpenCV_INCLUDE_DIRS})

find_package(Eigen3)
message(STATUS "Eigen头文件目录: " ${EIGEN3_INCLUDE_DIR})

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(ZBar0)
find_package(Zint)
message(STATUS "Zint 库文件: " ${ZINT_LIBRARY})

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
    ${ZBAR_INCLUDE_DIR}
)

file(GLOB PROJECT_SRC_FILES ./src/*cpp)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SRC_FILES})
target_link_libraries(${PROJECT_NAME} XiaoTuNetBox png)

############################################################################

enable_testing()

add_compile_definitions(XTCVB_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

add_executable(hello_test test/hello_test.cpp)
target_link_libraries(hello_test gtest gtest_main pthread rt ${PROJECT_NAME})

add_executable(hello_test_eigen test/hello_test_eigen.cpp)
target_link_libraries(hello_test_eigen gtest gtest_main pthread rt ${PROJECT_NAME})

add_executable(hello_test_opencv test/hello_test_opencv.cpp)
target_link_libraries(hello_test_opencv gtest gtest_main pthread rt XiaoTuNetBox ${OpenCV_LIBRARIES} ${PROJECT_NAME})

add_executable(t_stdin_v4l2_talk test/t_stdin_v4l2_talk.cpp)
target_link_libraries(t_stdin_v4l2_talk gtest gtest_main pthread rt XiaoTuNetBox ${ZBAR_LIBRARIES} ${OpenCV_LIBRARIES} ${PROJECT_NAME})

add_executable(t_bar_qrcode test/t_bar_qrcode.cpp)
target_link_libraries(t_bar_qrcode gtest gtest_main pthread rt XiaoTuNetBox ${ZBAR_LIBRARIES} ${OpenCV_LIBRARIES} ${PROJECT_NAME})

add_executable(t_bar_qrcode_generate test/t_bar_qrcode_generate.cpp)
target_link_libraries(t_bar_qrcode_generate gtest gtest_main pthread rt XiaoTuNetBox ${ZINT_LIBRARY} ${ZBAR_LIBRARIES} ${OpenCV_LIBRARIES} ${PROJECT_NAME})




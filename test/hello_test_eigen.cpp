#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include <Eigen/Eigen>

TEST(HelloTest, EigenRawMap) {

    int array[8];
    for(int i = 0; i < 8; ++i)
        array[i] = i;

    Eigen::Matrix<int, 2, 4> matrix = Eigen::Map<Eigen::Matrix<int,2,4>>(array);
    matrix(1, 0) = 44;

    std::cout << "Column-major:\n" << matrix << std::endl;
    std::cout << "Row-major:\n" << Eigen::Map<Eigen::Matrix<int,2,4,Eigen::RowMajor> >(array) << std::endl;
    std::cout << "Row-major using stride:\n" <<
    Eigen::Map<Eigen::Matrix<int,2,4>, Eigen::Unaligned, Eigen::Stride<1,4> >(array) << std::endl;

    Eigen::Map<Eigen::Matrix<int,2,4>> douwona = Eigen::Map<Eigen::Matrix<int,2,4>>(array);
    douwona(1, 0) = 111;

    std::cout << "Column-major:\n" << douwona << std::endl;
    std::cout << "Row-major:\n" << Eigen::Map<Eigen::Matrix<int,2,4,Eigen::RowMajor> >(array) << std::endl;
}


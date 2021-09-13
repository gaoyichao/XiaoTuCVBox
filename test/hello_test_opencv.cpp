#include <iostream>
#include <string>

#include <gtest/gtest.h>

#include <opencv2/opencv.hpp>

#include <XiaoTuCVBox/Image.h>
#include <XiaoTuCVBox/ImageUtils.h>


int main(int argc, char *argv[]) {
    std::string fname_str = "";
    xiaotu::cv::ImageMetaData metadata;
    std::vector<uint8_t> data;

    std::string tmp_dir = std::string(XTCVB_SOURCE_DIR) + "/resources/tmp";
    if (0 != access(tmp_dir.c_str(), F_OK))
        mkdir(tmp_dir.c_str(), 0755);

    fname_str = std::string(XTCVB_SOURCE_DIR) + "/resources/rgb.png";
    EXPECT_TRUE(xiaotu::cv::ReadPng(fname_str.c_str(), data, metadata));

    cv::Mat img(metadata.height, metadata.width, CV_8UC3, data.data());

    cv::imshow("bienao", img);
    cv::waitKey(-1);

    fname_str = std::string(XTCVB_SOURCE_DIR) + "/resources/depth.png";
    EXPECT_TRUE(xiaotu::cv::ReadPng(fname_str.c_str(), data, metadata));

    img = cv::Mat(metadata.height, metadata.width, CV_16UC1, data.data());

    cv::imshow("bienao", img);
    cv::waitKey(-1);

    cv::Mat imD = cv::imread(fname_str, cv::IMREAD_UNCHANGED);
    cv::imshow("不", imD);
    cv::waitKey(-1);

    std::cout << "imD type:" << imD.type() << std::endl;
    std::cout << "img type:" << img.type() << std::endl;

    std::cout << "img size:" << img.size() << std::endl;
    std::cout << "img rows:" << img.rows << std::endl;
    std::cout << "img cols:" << img.cols << std::endl;

    return 0;
}


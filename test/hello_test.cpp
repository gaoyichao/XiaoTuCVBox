#include <string>

#include <XiaoTuCVBox/Image.h>
#include <XiaoTuCVBox/ImageUtils.h>

#include <gtest/gtest.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>



TEST(HelloTest, PngReadWrite) {
    std::string fname_str = "";
    xiaotu::cv::ImageMetaData metadata;
    std::vector<uint8_t> data;

    std::string tmp_dir = std::string(XTCVB_SOURCE_DIR) + "/resources/tmp";
    if (0 != access(tmp_dir.c_str(), F_OK))
        mkdir(tmp_dir.c_str(), 0755);

    fname_str = std::string(XTCVB_SOURCE_DIR) + "/resources/rgb.png";
    EXPECT_TRUE(xiaotu::cv::ReadPng(fname_str.c_str(), data, metadata));

    fname_str = std::string(XTCVB_SOURCE_DIR) + "/resources/tmp/rgb.png";
    EXPECT_TRUE(xiaotu::cv::WritePng(fname_str.c_str(), data, metadata));

    data.clear();

    fname_str = std::string(XTCVB_SOURCE_DIR) + "/resources/depth.png";
    EXPECT_TRUE(xiaotu::cv::ReadPng(fname_str.c_str(), data, metadata));

    fname_str = std::string(XTCVB_SOURCE_DIR) + "/resources/tmp/depth.png";
    EXPECT_TRUE(xiaotu::cv::WritePng(fname_str.c_str(), data, metadata));
}


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
    uint8_t * img_ptr = NULL;

    std::string tmp_dir = std::string(XTCVB_SOURCE_DIR) + "/resources/tmp";
    if (0 != access(tmp_dir.c_str(), F_OK))
        mkdir(tmp_dir.c_str(), 0755);

    fname_str = std::string(XTCVB_SOURCE_DIR) + "/resources/rgb.png";
    std::cout << "读:" << fname_str << std::endl;
    img_ptr = xiaotu::cv::ReadPng(fname_str.c_str(), &metadata);

    fname_str = std::string(XTCVB_SOURCE_DIR) + "/resources/tmp/rgb.png";
    std::cout << "写:" << fname_str << std::endl;
    EXPECT_TRUE(xiaotu::cv::WritePng(fname_str.c_str(), &metadata, img_ptr));

    free(img_ptr);

    fname_str = std::string(XTCVB_SOURCE_DIR) + "/resources/depth.png";
    std::cout << "读:" << fname_str << std::endl;
    img_ptr = xiaotu::cv::ReadPng(fname_str.c_str(), &metadata);

    fname_str = std::string(XTCVB_SOURCE_DIR) + "/resources/tmp/depth.png";
    std::cout << "写:" << fname_str << std::endl;
    EXPECT_TRUE(xiaotu::cv::WritePng(fname_str.c_str(), &metadata, img_ptr));
    free(img_ptr);
}


#include <string>

#include <opencv2/opencv.hpp>

#include <zbar.h>

#include <gtest/gtest.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

TEST(zbar, qrcode) {
    std::string fname_str = "";
    std::vector<uint8_t> data;

    fname_str = "out.png";
    std::cout << fname_str << std::endl;

    zbar::ImageScanner scanner;
    scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);

    cv::Mat gray;
    cv::Mat img = cv::imread(fname_str);
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    zbar::Image zbar_image(img.cols, img.rows, "Y800", (uint8_t *)gray.data, img.cols * img.rows);
    int n = scanner.scan(zbar_image);

    std::cout << "检查到 " << n << " 个二维码" << std::endl;
    for (auto it = zbar_image.symbol_begin(); it != zbar_image.symbol_end(); ++it) {
        std::cout << "    类型:" << it->get_type_name() << std::endl;
        std::cout << "    数据:" << it->get_data() << std::endl;
        std::cout << "    定位:" << it->get_location_size() << std::endl;
        for (int i = 0; i < it->get_location_size(); i++) {
            std::cout << "        " << it->get_location_x(i) << ", " << it->get_location_y(i) << std::endl;
        }
    }

}


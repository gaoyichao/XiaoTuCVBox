#ifndef XTCVB_IMAGE_UTILS_H
#define XTCVB_IMAGE_UTILS_H

#include <iostream>
#include <vector>
#include <cstdio>

#include <XiaoTuCVBox/Image.h>

namespace xiaotu {
namespace cv {
    /*
     * ReadPng - 读一个png格式的图片
     * 
     * @filename: 文件名
     * @data: 图片数据
     * return: 图片数据,malloc申请的内存
     */
    bool ReadPng(char const * filename, std::vector<uint8_t> & data, ImageMetaData & metadata);
    /*
     * ReadPng - 读一个png格式的图片
     * 
     * @file: 标准文件指针
     * @data: 图片数据
     * @metadata: 描述数据
     */
    bool ReadPng(FILE * file, std::vector<uint8_t> & data, ImageMetaData & metadata);
    /*
     * CheckPng - 检查是否为png文件
     * 
     * libpng通过检查文件初始的几个字节来判定文件格式，字节数量越多判定越严格
     * 该检查会影响文件指针
     * 
     * @file: 标准文件指针
     * @bytes_to_check: 用于判定的字节数量
     */
    bool CheckPng(FILE * file, int bytes_to_check);

    /*
     * ParsePngColor - 解析颜色类型
     */
    ImageColorType ParsePngColor(int color_type);
    /*
     * ConvertPngColor - 转换为png颜色类型
     */
    int ConvertPngColor(ImageColorType color_type);
    /*
     * WritePng - 写一个png格式的图片
     * 
     * @filename: 文件名
     * @data: 已经按照metadata组织好的像素矩阵
     * @metadata: 描述数据
     */
    bool WritePng(char const * filename, std::vector<uint8_t> const & data, ImageMetaData const & metadata);
    /*
     * WritePng - 写一个png格式的图片
     * 
     * @file: 标准文件指针
     * @data: 已经按照metadata组织好的像素矩阵
     * @metadata: 描述数据
     */
    bool WritePng(FILE * file, std::vector<uint8_t> const & data, ImageMetaData const & metadata);

}
}

#endif


#include <iostream>

#include <cstdio>
#include <cstring>
#include <cassert>

#include <png.h>

#include <XiaoTuCVBox/Exception.h>
#include <XiaoTuCVBox/Image.h>
#include <XiaoTuCVBox/ImageUtils.h>

namespace xiaotu {
namespace cv {

    /*
     * ReadPng - 读一个png格式的图片
     * 
     * @filename: 文件名
     * @metadata: 描述数据
     * return: 图片数据,malloc申请的内存
     */
    uint8_t * ReadPng(char const * filename, ImageMetaData * metadata)
    {
        FILE *png = fopen(filename, "rb");
        assert(png);

        uint8_t *re = ReadPng(png, metadata);

        fclose(png);
        return re;
    }
    /*
     * ReadPng - 读一个png格式的图片
     * 
     * @file: 标准文件指针
     * @metadata: 描述数据
     * return: 图片数据,malloc申请的内存
     */
    uint8_t * ReadPng(FILE * file, ImageMetaData * metadata)
    {
        // 根据文件的前若干字节检查是否png
        const int bytes_to_check = 8;
        if (!CheckPng(file, bytes_to_check))
            return NULL;

        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png_ptr)
            return NULL;
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            png_destroy_read_struct(&png_ptr, nullptr, nullptr);
            return NULL;
        }

        // 告知检查文件时消费的字节数量
        png_init_io(png_ptr, file);
        png_set_sig_bytes(png_ptr, bytes_to_check);
        png_read_info(png_ptr, info_ptr);

        png_uint_32 w, h;
        int bit_depth, color_type, c;
        // int interlace_type; 暂不处理交错模式
        png_get_IHDR(png_ptr, info_ptr, &w, &h, &bit_depth, &color_type, nullptr, nullptr, nullptr);
        c = png_get_channels(png_ptr, info_ptr);

        // 注册各种Transform
        // 重新加载
               
        // 申请内存保存数据
        png_uint_32 row_bytes = png_get_rowbytes(png_ptr, info_ptr);
        // 记录描述数据
        metadata->width = w;
        metadata->height = h;
        metadata->channel = c;
        metadata->bit_depth = bit_depth;
        metadata->row_bytes = row_bytes;
        metadata->color_type = ParsePngColor(color_type);

        uint8_t * img_ptr = (uint8_t *)malloc(h * row_bytes);
        assert(img_ptr);
        // 准备行指针
        png_byte **row_pp = (png_bytep*)malloc(h * sizeof(png_bytep));
        assert(row_pp);
        for (png_uint_32 i = 0; i < h; i++)
            row_pp[i] = img_ptr + i * row_bytes;

        png_read_image(png_ptr, row_pp);

        free(row_pp);
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

        return img_ptr;
    }
    /*
     * CheckPng - 检查是否为png文件
     * 
     * libpng通过检查文件初始的几个字节来判定文件格式，字节数量越多判定越严格
     * 该检查会影响文件指针
     * 
     * @file: 标准文件指针
     * @bytes_to_check: 用于判定的字节数量
     */
    bool CheckPng(FILE * file, int bytes_to_check)
    {
        uint8_t buf[bytes_to_check];
        if (fread(buf, 1, bytes_to_check, file) != bytes_to_check)
            return false;

        return(!png_sig_cmp(buf, 0, bytes_to_check));
    }

    /*
     * ParsePngColor - 解析颜色类型
     * 
     * 在1.6.37, png.h的668行有描述颜色类型，是RGB、ALPHA、PALETTE几种的组合
     */
    ImageColorType ParsePngColor(int color_type)
    {
        switch (color_type) {
            case PNG_COLOR_TYPE_GRAY:
                return ImageColorType::IMG_COLOR_TYPE_GRAY;
            case PNG_COLOR_TYPE_RGB:
                return ImageColorType::IMG_COLOR_TYPE_RGB;
            case PNG_COLOR_TYPE_RGB_ALPHA:
            case PNG_COLOR_TYPE_GRAY_ALPHA:
                throw Exception(__FILE__":暂不支持ALPHA通道");
            case PNG_COLOR_TYPE_PALETTE:
                throw Exception(__FILE__":暂不支持调色板");
            default:
                throw Exception(__FILE__":未知颜色类型");
        }
    }

    /*
     * ConvertPngColor - 转换为png颜色类型
     * 
     * 在1.6.37, png.h的668行有描述颜色类型，是RGB、ALPHA、PALETTE几种的组合
     */
    int ConvertPngColor(ImageColorType color_type)
    {
        switch (color_type) {
            case ImageColorType::IMG_COLOR_TYPE_GRAY:
                return PNG_COLOR_TYPE_GRAY;
            case ImageColorType::IMG_COLOR_TYPE_RGB:
                return PNG_COLOR_TYPE_RGB;
            case ImageColorType::IMG_COLOR_TYPE_RGBA:
                throw Exception(__FILE__":暂不支持ALPHA通道");
            default:
                throw Exception(__FILE__":未知颜色类型");
        }
    }

    /*
     * WritePng - 写一个png格式的图片
     * 
     * @filename: 文件名
     * @metadata: 描述数据
     * @data: 已经按照metadata组织好的像素矩阵
     */
    bool WritePng(char const * filename, ImageMetaData const * metadata, uint8_t const * data)
    {
        FILE *png = fopen(filename, "wb");
        assert(png);

        bool re = WritePng(png, metadata, data);

        fclose(png);
        return re;

    }
    /*
     * WritePng - 写一个png格式的图片
     * 
     * @file: 标准文件指针
     * @metadata: 描述数据
     * @data: 已经按照metadata组织好的像素矩阵
     */
    bool WritePng(FILE * file, ImageMetaData const * metadata, uint8_t const * data)
    {
        assert(file);
        assert(metadata);
        assert(data);

        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png_ptr)
            return false;
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
            return false;

        png_init_io(png_ptr, file);
        png_set_IHDR(png_ptr, info_ptr, metadata->width,
                                        metadata->height,
                                        metadata->bit_depth,
                                        ConvertPngColor(metadata->color_type),
                                        PNG_INTERLACE_NONE,
                                        PNG_COMPRESSION_TYPE_DEFAULT,
                                        PNG_FILTER_TYPE_DEFAULT);
        png_write_info(png_ptr, info_ptr);

        for (int i = 0; i < metadata->height; i++)
            png_write_row(png_ptr, data + i * metadata->row_bytes);

        png_write_end(png_ptr, nullptr);
        png_destroy_write_struct(&png_ptr, &info_ptr);

        return true;
    }
}
}

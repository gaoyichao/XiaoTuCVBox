#ifndef XTCVB_IMAGE_H
#define XTCVB_IMAGE_H


namespace xiaotu {
namespace cv {

    enum ImageColorType {
        IMG_COLOR_TYPE_UNKNOWN = 0,
        IMG_COLOR_TYPE_GRAY = 1,
        IMG_COLOR_TYPE_RGB  = 2,
        IMG_COLOR_TYPE_RGBA = 3
    };

    /*
     * ImageMetaData - 图片的描述数据
     */
    struct ImageMetaData {
        int width;     // 图片宽度
        int height;    // 图片高度
        int channel;   // 图片通道数量

        int bit_depth;  // 图片元素位长
        int row_bytes;  // 每行数据字节长度
        ImageColorType color_type; // 颜色类型, RGB...
    };

}
}

#endif


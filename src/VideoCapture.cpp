
#include <XiaoTuCVBox/VideoCapture.h>

#include <iostream>
#include <string>

#include <stdio.h>
#include <cstring>
#include <cassert>
#include <map>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>


namespace xiaotu {
namespace cv {

    /*
     * VideoCapture - 构造函数
     * 
     * 打开设备 --> 查看设备功能 --> 设置图片格式 --> 申请缓存 --> 用户指针映射
     */
    VideoCapture::VideoCapture(std::string const & path, int nbuf)
    {
        mModuleFd = open(path.c_str(), O_RDWR | O_NONBLOCK); 
        if (-1 == mModuleFd) {
            fprintf(stderr, "无法打开设备 '%s': %d, %s\n", path.c_str(), errno, strerror(errno));
            exit(-1);
        }

        int re = ioctl(mModuleFd, VIDIOC_QUERYCAP, &mCap);
        if (-1 == re) {
            fprintf(stderr, "无法获取设备信息 '%s': %d, %s\n", path.c_str(), errno, strerror(errno));
            exit(-1);
        }

        if (!(mCap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            fprintf(stderr, "%s 不是 video capture 设备\n", path.c_str());
            exit(-1);
        }


        if (!(mCap.capabilities & V4L2_CAP_STREAMING)) {
            fprintf(stderr, "目前只支持 STREAMING 类型的设备\n");
            exit(-1);
        }
 
        re = ioctl(mModuleFd, VIDIOC_G_PRIORITY, &mPrio);
        assert(-1 != re);

        /////////////////////////////////////////////////////////
        //memset(&mCropCap, 0, sizeof(mCropCap));
        //mCropCap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        //re = ioctl(mModuleFd, VIDIOC_CROPCAP, &mCropCap);
        //if (-1 == re) {
        //    fprintf(stderr, "ERROR VIDIOC_CROPCAP '%s': %d, %s\n", path.c_str(), errno, strerror(errno));
        //    //exit(-1);
        //}
        //
        //mCrop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        //mCrop.c = mCropCap.defrect;
        //re = ioctl(mModuleFd, VIDIOC_S_CROP, &mCrop);
        //if (-1 == re) {
        //    fprintf(stderr, "ERROR VIDIOC_S_CROP '%s': %d, %s\n", path.c_str(), errno, strerror(errno));
        //    //exit(-1);
        //}
        /////////////////////////////////////////////////////////

        // v4l2_format
        memset(&mFmt, 0, sizeof(mFmt));
        mFmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        re = ioctl(mModuleFd, VIDIOC_G_FMT, &mFmt);
        if (-1 == re) {
            fprintf(stderr, "ERROR VIDIOC_G_FMT '%s': %d, %s\n", path.c_str(), errno, strerror(errno));
            exit(-1);
        }
        mFmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        re = ioctl(mModuleFd, VIDIOC_S_FMT, &mFmt);
        if (-1 == re) {
            fprintf(stderr, "ERROR VIDIOC_S_FMT '%s': %d, %s\n", path.c_str(), errno, strerror(errno));
            fprintf(stderr, "不支持 V4L2_PIX_FMT_YUYV");
            exit(-1);
        }

        mNumBuffers = nbuf;
        // 使用 user ptr 获取数据
        std::cout << "size image:" << mFmt.fmt.pix.sizeimage << std::endl;
        memset(&mBufReq, 0, sizeof(mBufReq));
        mBufReq.count = mNumBuffers;
        mBufReq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        mBufReq.memory = V4L2_MEMORY_USERPTR;
        re = ioctl(mModuleFd, VIDIOC_REQBUFS, &mBufReq);
        if (-1 == re) {
            fprintf(stderr, "ERROR VIDIOC_REQBUFS '%s': %d, %s\n", path.c_str(), errno, strerror(errno));
            exit(-1);
        }

        mBuffers.resize(mNumBuffers);
        for (int i = 0; i < mNumBuffers; i++) {
            mBuffers[i] = BufferPtr(new Buffer);
            mBuffers[i]->Alloc(mFmt.fmt.pix.sizeimage);
            mBuffers[i]->id = i + 1;
        }
        mBufferInUse = BufferPtr(new Buffer);
        mBufferInUse->Alloc(mFmt.fmt.pix.sizeimage);
        mBufferInUse->id = 0;
        mUsingBuffer = false;

        mEventHandler = xiaotu::net::PollEventHandlerPtr(new xiaotu::net::PollEventHandler(mModuleFd));
        mEventHandler->EnableRead(true);
        mEventHandler->EnableWrite(false);
        mEventHandler->SetReadCallBk(std::bind(&VideoCapture::OnReadEvent, this));
    }

    VideoCapture::~VideoCapture()
    {
        close(mModuleFd);
    }

    /*
     * EnumFmt - 枚举支持的格式
     */
    void VideoCapture::EnumFmt()
    {
        struct v4l2_fmtdesc desc;
        desc.index = 0;
        desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        int re = ioctl(mModuleFd, VIDIOC_ENUM_FMT, &desc);
        while (0 == re) {
            std::cout << desc << std::endl;
            desc.index++;
            re = ioctl(mModuleFd, VIDIOC_ENUM_FMT, &desc);
        }
    }

    /*
     * QueryCtrl - 查询控制指令的合理格式
     */
    void VideoCapture::QueryCtrl(uint32_t id)
    {
        struct v4l2_queryctrl ctrl;
        ctrl.id = id;

        int re = ioctl(mModuleFd, VIDIOC_QUERYCTRL, &ctrl);
        if (-1 == re) {
            perror("不支持 VIDIOC_QUERYCTRL");
            return;
        }

        std::cout << ctrl << std::endl;
    }

    /*
     * IsAutoGain - 判定设备是否是自动增益
     */
    bool VideoCapture::IsAutoGain()
    {
        struct v4l2_control ctrl;
        ctrl.id = V4L2_CID_AUTOGAIN;
        int re = ioctl(mModuleFd, VIDIOC_G_CTRL, &ctrl);
        if (-1 == re) {
            perror("不支持 V4L2_CID_AUTOGAIN");
            return false;
        }

        return ctrl.value != 0;
    }

    /*
     * GetCtrlInteger - 查询整型的控制指令
     */
    int VideoCapture::GetCtrlInteger(uint32_t id)
    {
        struct v4l2_control ctrl;
        ctrl.id = id;
        int re = ioctl(mModuleFd, VIDIOC_G_CTRL, &ctrl);
        if (-1 == re) {
            perror("不支持");
            return 0;
        }

        return ctrl.value;
    }

    /*
     * SetCtrlInteger - 设定整型的控制指令
     */
    bool VideoCapture::SetCtrlInteger(uint32_t id, int value)
    {
        struct v4l2_control ctrl;
        ctrl.id = id;
        ctrl.value = value;
        int re = ioctl(mModuleFd, VIDIOC_S_CTRL, &ctrl);
        if (-1 == re) {
            perror("不支持 V4L2_CID_GAIN");
            return false;
        }

        return true;
    }


    /*
     * OnReadEvent - PollLoop循环的可读事件回调
     * 
     * 读数据 --> 替换缓存 --> 应用回调 --> 缓存重入列
     */
    void VideoCapture::OnReadEvent()
    {
        struct v4l2_buffer buf;
        unsigned int i;
        memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;
 
        if (-1 == ioctl(mModuleFd, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
                case EAGAIN:
                    return;
                case EIO:
                    break;
                default:
                    perror("VIDIOC_DQBUF");
                    exit(-1);
            }
        }

        for (int i = 0; i < mNumBuffers; i++) {
            if (buf.m.userptr == (unsigned long)mBuffers[i]->start && buf.length == mBuffers[i]->length) {
                if (mCaptureCB && !mUsingBuffer) {
                    mBufferInUseMutex.lock();
                    mUsingBuffer = true;
                    std::swap(mBufferInUse, mBuffers[i]);
                    mBufferInUseMutex.unlock();
                    
                    mCaptureCB(mBufferInUse);

                    buf.m.userptr = (unsigned long)mBuffers[i]->start;
                    buf.length = mBuffers[i]->length;
                }
            }
        }

        if (-1 == ioctl(mModuleFd, VIDIOC_QBUF, &buf)) {
            perror("VIDIOC_QBUF");
            exit(-1);
        }
    }

    void VideoCapture::ReleaseBufferInUse()
    {
        mBufferInUseMutex.lock();
        mUsingBuffer = false;
        mBufferInUseMutex.unlock();
    }
    
    /*
     * StartCapturing - 开始读图像
     * 
     * 缓存入列 --> 开始采集
     */
    void VideoCapture::StartCapturing()
    {
        for (int i = 0; i < mNumBuffers; i++) {
            struct v4l2_buffer buf;
            memset(&buf, 0, sizeof(buf));

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;
            buf.index = i;
            buf.m.userptr = (unsigned long)mBuffers[i]->start;
            buf.length = mBuffers[i]->length;

            if (-1 == ioctl(mModuleFd, VIDIOC_QBUF, &buf)) {
                fprintf(stderr, "ERROR VIDIOC_QBUF '%s': %d, %s\n", __FILE__, errno, strerror(errno));
                exit(-1);
            }
        }

        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == ioctl(mModuleFd, VIDIOC_STREAMON, &type)) {
            fprintf(stderr, "ERROR VIDIOC_STREAMON '%s': %d, %s\n", __FILE__, errno, strerror(errno));
            exit(-1);
        }
    }

    /*
     * CountVideoInput - 计数
     */
    int VideoCapture::CountVideoInput()
    {
        int count = 0;
        struct v4l2_input input;

        input.index = count;
        int re = ioctl(mModuleFd, VIDIOC_ENUMINPUT, &input);
        while (0 == re) {
            count++;
            input.index = count;
            re = ioctl(mModuleFd, VIDIOC_ENUMINPUT, &input);
        }
        return count;
    }
    /*
     * IsCapabilityValid - 判定v4l2_capability是否合法
     */
    bool VideoCapture::IsCapabilityValid()
    {
        // __u8 driver[16] '\0'结尾的ASCII码
        if (!IsNulEnded(mCap.driver, 16))
            return false;

        // __u8 card[32] '\0'结尾的UTF-8
        if (!IsNulEnded(mCap.card, 32))
            return false;
        
        // __u8 bus_info[32] '\0'结尾的ASCII
        if (!IsNulEnded(mCap.bus_info, 32))
            return false;

        return true;
    }
    /*
     * SetPriority - 设置当前应用访问访问设备的优先级
     */
    bool VideoCapture::SetPriority(enum v4l2_priority prio)
    {
        int re = ioctl(mModuleFd, VIDIOC_S_PRIORITY, &prio);
        if (-1 == re) {
            perror("设置优先级出错:");
            return false;
        }

        enum v4l2_priority tmp;
        re = ioctl(mModuleFd, VIDIOC_G_PRIORITY, &tmp);
        if (-1 == re) {
            perror("获取优先级出错:");
            return false;
        }

        mPrio = tmp;
        return mPrio == prio;
    }

}
}


    /*******************************************************************************************************/

    /*
     * __OStreamCap - 解析并输出v4l2的特性
     * 
     * @stream: 输出流对象
     * @cap: 设备特性描述字
     */
    std::ostream & __OStreamCap(std::ostream & stream, const uint32_t cap)
    {
        if (V4L2_CAP_VIDEO_CAPTURE & cap)
            stream << "    [Support] Single-planar API Video Capture!" << std::endl;
        if (V4L2_CAP_VIDEO_CAPTURE_MPLANE & cap)
            stream << "    [Support] Multi-planar API Video Capture!" << std::endl;

        if (V4L2_CAP_VIDEO_OUTPUT & cap)
            stream << "    [Support] Single-planar API Video Output!" << std::endl;
        if (V4L2_CAP_VIDEO_OUTPUT_MPLANE & cap)
            stream << "    [Support] Multi-planar API Video Output!" << std::endl;

        if (V4L2_CAP_VIDEO_M2M & cap)
            stream << "    [Support] Single-planar API Video Memory-To-Memory interface!" << std::endl;
        if (V4L2_CAP_VIDEO_M2M_MPLANE & cap)
            stream << "    [Support] Multi-planar API Video Memory-To-Memory interface!" << std::endl;

        if (V4L2_CAP_VIDEO_OVERLAY & cap)
            stream << "    [Support] Video Overlay! A video overlay device typically stores captured images directly in the video memory of a graphics card, with hardware clipping and scaling." << std::endl;

        if (V4L2_CAP_VBI_CAPTURE & cap)
            stream << "    [Support] Raw VBI Capture!" << std::endl;
        if (V4L2_CAP_VBI_OUTPUT & cap)
            stream << "    [Support] Raw VBI Output!" << std::endl;

        if (V4L2_CAP_SLICED_VBI_CAPTURE & cap)
            stream << "    [Support] Sliced VBI Capture!" << std::endl;
        if (V4L2_CAP_SLICED_VBI_OUTPUT & cap)
            stream << "    [Support] Sliced VBI Output!" << std::endl;

        if (V4L2_CAP_RDS_CAPTURE & cap)
            stream << "    [Support] RDS Capture!" << std::endl;

        if (V4L2_CAP_VIDEO_OUTPUT_OVERLAY & cap)
            stream << "    [Support] Video Output Overlay (OSD)!" << std::endl;

        if (V4L2_CAP_HW_FREQ_SEEK & cap)
            stream << "    [Support] ioctl VIDIOC_S_HW_FREQ_SEEK! hardware frequency seeking" << std::endl;

        if (V4L2_CAP_RDS_OUTPUT & cap)
            stream << "    [Support] RDS output interface" << std::endl;

        if (V4L2_CAP_TUNER & cap)
            stream << "    [Support] The device has some sort of tuner to receive RF-modulated video signals." << std::endl;

        if (V4L2_CAP_AUDIO & cap)
            stream << "    [Support] The device has audio inputs or outputs. It may or may not support audio recording or playback, in PCM or compressed formats. PCM audio support must be implemented as ALSA or OSS interface." << std::endl;

        if (V4L2_CAP_RADIO & cap)
            stream << "    [Support] This is a radio receiver." << std::endl;

        if (V4L2_CAP_MODULATOR & cap)
            stream << "    [Support] The device has some sort of modulator to emit RF-modulated video/audio signals." << std::endl;

        if (V4L2_CAP_SDR_CAPTURE & cap)
            stream << "    [Support] SDR Capture." << std::endl;

        if (V4L2_CAP_EXT_PIX_FORMAT & cap)
            stream << "    [Support] struct v4l2_pix_format extended fields." << std::endl;

        if (V4L2_CAP_SDR_OUTPUT & cap)
            stream << "    [Support] SDR Output." << std::endl;

        if (V4L2_CAP_READWRITE & cap)
            stream << "    [Support] read() write() I/O." << std::endl;

        if (V4L2_CAP_ASYNCIO & cap)
            stream << "    [Support] 异步I/O." << std::endl;

        if (V4L2_CAP_STREAMING & cap)
            stream << "    [Support] streaming I/O." << std::endl;

        if (V4L2_CAP_TOUCH & cap)
            stream << "    [Support] 触摸设备." << std::endl;

        if (V4L2_CAP_DEVICE_CAPS & cap)
            stream << "    [Support] The driver fills the device_caps field. This capability can only appear in the capabilities field and never in the device_caps field." << std::endl;

        return stream;
    }

    /*
     * operator << - 序列化输出v4l2_capability
     */
    std::ostream & operator << (std::ostream & stream, struct v4l2_capability const & cap)
    {
        stream << "driver:   " << cap.driver << std::endl;
        stream << "card:     " << cap.card << std::endl;
        stream << "bus_info: " << cap.bus_info << std::endl;

        char str[80];
        sprintf(str, "version:  %u.%u.%u", (cap.version >> 16) & 0xFF, (cap.version >> 8) & 0xFF, cap.version & 0xFF);
        stream << str << std::endl;

        stream << "整机特性:" << std::endl;
        __OStreamCap(stream, cap.capabilities) << std::endl;

        if (V4L2_CAP_DEVICE_CAPS & cap.capabilities) {
            stream << "当前打开设备的特性:" << std::endl;
            __OStreamCap(stream, cap.device_caps) << std::endl;
        }

        return stream;
    }

    char const * __InputTypeStr[] = {
        {0},
        {"TUNER"},
        {"CAMERA"},
        {"TOUCH"}
    };
    /*
     * operator << - 序列化输出v4l2_input
     */
    std::ostream & operator << (std::ostream & stream, struct v4l2_input const & input)
    {
        stream << "index:" << input.index << std::endl;
        stream << "name: " << input.name << std::endl;
        stream << "type: " << __InputTypeStr[input.type] << std::endl;

        char str[80];
        sprintf(str, "audioset:  %4x", input.audioset);
        stream << str << std::endl;

        memset(str, 0, 80);
        sprintf(str, "tuner:  %4x", input.tuner);
        stream << str << std::endl;

        memset(str, 0, 80);
        sprintf(str, "std:  %8llx", input.std);
        stream << str << std::endl;

        memset(str, 0, 80);
        sprintf(str, "status:  %4x", input.status);
        stream << str << std::endl;

        if (V4L2_IN_CAP_DV_TIMINGS & input.capabilities)
            stream << "    [support] setting video timings by using VIDIOC_S_DV_TIMINGS" << std::endl;
        if (V4L2_IN_CAP_STD & input.capabilities)
            stream << "    [support] setting TV standard by using VIDIOC_S_STD" << std::endl;
        if (V4L2_IN_CAP_NATIVE_SIZE & input.capabilities)
            stream << "    [support] setting native size by using V4L2_SEL_TGT_NATIVE_SIZE" << std::endl;


        return stream;
    }

    std::ostream & operator << (std::ostream & stream, struct v4l2_rect const & rect)
    {
        stream << "    left:  " << rect.left << std::endl;
        stream << "    top:   " << rect.top << std::endl;
        stream << "    width: " << rect.width << std::endl;
        stream << "    height:" << rect.height << std::endl;

        return stream;
    }


    std::ostream & operator << (std::ostream & stream, struct v4l2_fract const & fract)
    {
        stream << "numerator:  " << fract.numerator << std::endl;
        stream << "denominator:" << fract.denominator << std::endl;
        stream << "帧周期:" << (fract.numerator / fract.denominator) << std::endl;
        stream << "帧率:" << (fract.denominator / fract.numerator) << std::endl;

        return stream;
    }

    std::ostream & operator << (std::ostream & stream, struct v4l2_cropcap const & cap)
    {
        stream << "type: " << cap.type << std::endl;
        stream << "bounds:" << std::endl << cap.bounds;
        stream << "defrect:" << std::endl << cap.defrect;
        stream << cap.pixelaspect << std::endl;

        return stream;
    }

    char const * __FormatTypeStr[] = {
        {0},
        {"V4L2_BUF_TYPE_VIDEO_CAPTURE"},
        {"V4L2_BUF_TYPE_VIDEO_OUTPUT"},
        {"V4L2_BUF_TYPE_VIDEO_OVERLAY"},
        {"V4L2_BUF_TYPE_VBI_CAPTURE"},
        {"V4L2_BUF_TYPE_VBI_OUTPUT"},
        {"V4L2_BUF_TYPE_SLICED_VBI_CAPTURE"},
        {"V4L2_BUF_TYPE_SLICED_VBI_OUTPUT"},
        {"V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY"},
        {"V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE"},
        {"V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE"},
        {"V4L2_BUF_TYPE_SDR_CAPTURE"},
        {"V4L2_BUF_TYPE_SDR_OUTPUT"},
    };

    std::map<uint32_t, std::string> __gPixelFormat = {
        { V4L2_PIX_FMT_RGB332, "V4L2_PIX_FMT_RGB332"},
        { V4L2_PIX_FMT_ARGB444, "V4L2_PIX_FMT_ARGB444"},

        { V4L2_PIX_FMT_YUYV, "V4L2_PIX_FMT_YUYV"},
    };

    std::ostream & operator << (std::ostream & stream, struct v4l2_format const & format)
    {
        stream << "type: " << __FormatTypeStr[format.type] << std::endl;
        stream << "width: " << format.fmt.pix.width << std::endl;
        stream << "height:" << format.fmt.pix.height << std::endl;
        stream << "bytesperline:" << format.fmt.pix.bytesperline << std::endl;
        stream << "sizeimage:" << format.fmt.pix.sizeimage << std::endl;
        char str[80];
        sprintf(str, "pixelformat: %c%c%c%c",  format.fmt.pix.pixelformat & 0xFF,
                                                (format.fmt.pix.pixelformat >> 8) & 0xFF,
                                                (format.fmt.pix.pixelformat >> 16) & 0xFF,
                                                (format.fmt.pix.pixelformat >> 24) & 0xFF);
        stream << str << std::endl;
        return stream;
    }

    std::ostream & operator << (std::ostream & stream, struct v4l2_fmtdesc const & desc)
    {
        stream << "--------------------------" << std::endl;
        stream << "index: " << desc.index << std::endl;
        stream << "type: " << __FormatTypeStr[desc.type] << std::endl;
        stream << "flags: " << desc.flags << std::endl;

        stream << desc.description << std::endl;

        char str[80];
        sprintf(str, "pixelformat: %c%c%c%c", desc.pixelformat & 0xFF,
                                             (desc.pixelformat >> 8) & 0xFF,
                                             (desc.pixelformat >> 16) & 0xFF,
                                             (desc.pixelformat >> 24) & 0xFF);
        stream << str << std::endl;
   
        return stream;
    }

    std::map<uint32_t, std::string> __gCtrlId = {
        { V4L2_CID_BRIGHTNESS, "BRIGHTNESS, 亮度"},
        { V4L2_CID_CONTRAST, "CONTRAST, 对比度"},
        { V4L2_CID_SATURATION, "SATURATION, 饱和度"},
        { V4L2_CID_HUE, "HUE, 色度"},
        { V4L2_CID_EXPOSURE, "EXPOSURE, 曝光"},
        { V4L2_CID_AUTOGAIN, "AUTOGAIN, 自动增益/曝光"},
        { V4L2_CID_GAIN, "GAIN, 增益"},
    };

    std::map<uint32_t, std::string> __gCtrlType = {
        { V4L2_CTRL_TYPE_INTEGER, "INTEGER"},
        { V4L2_CTRL_TYPE_BOOLEAN, "BOOLEAN"},
    };


    std::ostream & operator << (std::ostream & stream, struct v4l2_queryctrl const & ctrl)
    {
        stream << "id:" << ctrl.id << __gCtrlId[ctrl.id] << std::endl;    
        stream << "type:" << ctrl.type << ", " << __gCtrlType[ctrl.type] << std::endl;
        stream << "name:" << ctrl.name << std::endl;
        stream << "min:" << ctrl.minimum << std::endl;
        stream << "max:" << ctrl.maximum << std::endl;
        stream << "step:" << ctrl.step << std::endl;
        stream << "default_value:" << ctrl.default_value << std::endl;
        stream << "flags:" << std::hex << ctrl.flags << std::dec << std::endl;

        return stream;
    }


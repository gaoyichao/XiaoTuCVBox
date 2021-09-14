#ifndef XTCVB_V4L2INTERFACE_H
#define XTCVB_V4L2INTERFACE_H

#include <iostream>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include <cassert>
#include <mutex>

#include <XiaoTuNetBox/EventHandler.h>

namespace xiaotu {
namespace cv {

    class VideoCapture {
        public:
            struct Buffer {
                Buffer() : start(nullptr), length(0) {}
                ~Buffer() { Free(); }

                inline void Free()
                {
                    if (nullptr != start)
                        free(start);
                    start = nullptr;
                    length = 0;
                }

                inline void Alloc(size_t size)
                {
                    if (nullptr != start)
                        Free();
                    length = size;
                    start = malloc(size);
                    assert(start);
                }

                void *start;
                size_t  length;
                int id;
            };

            typedef std::shared_ptr<Buffer> BufferPtr;
            typedef std::shared_ptr<const Buffer> BufferConstPtr;

        public:
            VideoCapture(std::string const & path, int nbuf);
            ~VideoCapture();

            void OnReadEvent();
            void ReleaseBufferInUse();

            void EnumFmt();
            void QueryCtrl(uint32_t id);

            int GetCtrlInteger(uint32_t id);
            bool SetCtrlInteger(uint32_t id, int value);

            bool IsAutoGain();


            int NumBuffers() const { return mNumBuffers; }
            int GetFd() const { return mModuleFd; }
            xiaotu::net::PollEventHandlerPtr & GetHandler() { return mEventHandler; }
        private:
            int mModuleFd;
            xiaotu::net::PollEventHandlerPtr mEventHandler;
            int mNumBuffers;

            std::vector<BufferPtr> mBuffers;
            BufferPtr mBufferInUse;
            bool mUsingBuffer;
            std::mutex mBufferInUseMutex;

        public:
            typedef std::function<void(BufferPtr const & buffer)> CaptureCB;
            void SetCaptureCB(CaptureCB cb) { mCaptureCB = std::move(cb); }
            CaptureCB mCaptureCB;

        public:
            bool SetPriority(enum v4l2_priority prio);
            void StartCapturing();

        private:
            bool IsCapabilityValid();
            int CountVideoInput();

            inline int GetCurrentInput()
            {
                int idx;
                if (-1 == ioctl(mModuleFd, VIDIOC_G_INPUT, &idx)) {
                    perror("VIDIOC_G_INPUT:");
                    exit(-1);
                }
                return idx;
            }

        public:
            struct v4l2_capability mCap;
            struct v4l2_cropcap mCropCap; // 视频裁剪和采样特性
            struct v4l2_crop mCrop;
            struct v4l2_format mFmt;
            struct v4l2_requestbuffers mBufReq;


            enum v4l2_priority mPrio;

            int mNumVideoInput;
            int mIndex;
            struct v4l2_input mInput;
            struct v4l2_standard mStd;
    };

    typedef std::shared_ptr<VideoCapture> VideoCapturePtr;
    typedef std::shared_ptr<const VideoCapture> VideoCaptureConstPtr;


}
}

    /*******************************************************************************************************/

    /*
     * operator << - 序列化输出v4l2特性
     */
    std::ostream & operator << (std::ostream & stream, struct v4l2_capability const & cap);
    std::ostream & operator << (std::ostream & stream, struct v4l2_input const & input);
    std::ostream & operator << (std::ostream & stream, struct v4l2_rect const & rect);
    std::ostream & operator << (std::ostream & stream, struct v4l2_cropcap const & cap);
    std::ostream & operator << (std::ostream & stream, struct v4l2_fract const & fract);

    std::ostream & operator << (std::ostream & stream, struct v4l2_format const & format);
    std::ostream & operator << (std::ostream & stream, struct v4l2_fmtdesc const & desc);
    std::ostream & operator << (std::ostream & stream, struct v4l2_queryctrl const & ctrl);


    /*
     * IsNulEnded - 判定c字符串是否以'\0'结尾
     * 
     * @str: 目标字符串
     * @len: 检查长度，超出该长度则判定为否
     */
    inline bool IsNulEnded(uint8_t const * str, int len)
    {
        for (int i = 0; i < len; i++)
            if ('\0' == str[i])
                return true;
        return false;
    }



#endif


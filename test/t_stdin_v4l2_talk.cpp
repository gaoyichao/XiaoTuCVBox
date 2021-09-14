#include <string>
#include <iostream>
#include <vector>

#include <XiaoTuCVBox/Image.h>
#include <XiaoTuCVBox/ImageUtils.h>
#include <XiaoTuCVBox/VideoCapture.h>

#include <XiaoTuNetBox/Connection.h>
#include <XiaoTuNetBox/PollLoop.h>

#include <opencv2/opencv.hpp>

xiaotu::cv::VideoCapture::BufferPtr gBuffer;

void OnStdinRawMsg(xiaotu::cv::VideoCapturePtr capture, xiaotu::net::RawMsgPtr const & msg) {
    std::cout << "msg.size = " << msg->size() << std::endl;

    switch ((*msg)[0]) {
        case 'a':
            std::cout << capture->mCap << std::endl;
            std::cout << capture->mFmt << std::endl;
            capture->ReleaseBufferInUse();
            break;
        default:
            break;
    }
}

void OnRead(xiaotu::cv::VideoCapture::BufferPtr const & buffer)
{
    gBuffer = buffer;
    std::cout << __FUNCTION__ << ":" << gBuffer->id << std::endl;
}

int main(int argc, char *argv[])
{
    if (2 != argc) {
        std::cout << "./t_stdin_v4l2_talk /dev/video0" << std::endl;
        exit(-1);
    }

    std::string dev_path(argv[1]);
    gBuffer.reset();

	xiaotu::cv::VideoCapturePtr capture(new xiaotu::cv::VideoCapture(dev_path, 8));
    capture->SetCaptureCB(std::bind(&OnRead, std::placeholders::_1));
    capture->StartCapturing();

    xiaotu::net::ConnectionPtr stdin_conn = xiaotu::net:: ConnectionPtr(new xiaotu::net::Connection(0, "标准输入:stdin:0"));
    stdin_conn->SetRecvRawCallBk(std::bind(&OnStdinRawMsg, capture, std::placeholders::_1));

    xiaotu::net::PollLoopPtr gLoop = xiaotu::net::CreatePollLoop();
    xiaotu::net::ApplyOnLoop(capture, gLoop);
    xiaotu::net::ApplyOnLoop(stdin_conn, gLoop);

    while (1) {
        gLoop->LoopOnce(0);

        if (nullptr != gBuffer) {
            cv::Size img_size(capture->mFmt.fmt.pix.width, capture->mFmt.fmt.pix.height);

            cv::Mat yuyv(img_size, CV_8UC2, gBuffer->start);
            cv::Mat bgr(img_size, CV_8UC3);
            cv::cvtColor(yuyv, bgr, cv::COLOR_YUV2BGR_YUYV);

            cv::imshow("bgr", bgr);
            cv::waitKey(1);

            gBuffer.reset();
            capture->ReleaseBufferInUse();
        }
    }
    return 0;
}



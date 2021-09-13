#include <string>
#include <iostream>

#include <XiaoTuCVBox/Image.h>
#include <XiaoTuCVBox/ImageUtils.h>
#include <XiaoTuCVBox/VideoCapture.h>

#include <XiaoTuNetBox/Connection.h>
#include <XiaoTuNetBox/PollLoop.h>

#include <gtest/gtest.h>

void OnRead(xiaotu::cv::VideoCapture::Buffer * buffer)
{
    std::cout << "你在逗我吗:" << buffer->length << std::endl;
}

int main(int argc, char *argv[])
{
    if (2 != argc) {
        std::cout << "./t_stdin_v4l2_talk /dev/video0" << std::endl;
        exit(-1);
    }

    std::string dev_path(argv[1]);


	xiaotu::cv::VideoCapturePtr capture(new xiaotu::cv::VideoCapture(dev_path, 8));
    capture->SetCaptureCB(std::bind(&OnRead, std::placeholders::_1));
    capture->StartCapturing();

    xiaotu::net::PollLoopPtr gLoop = xiaotu::net::CreatePollLoop();
    xiaotu::net::ApplyOnLoop(capture, gLoop);

    gLoop->Loop(1000);
    return 0;
}



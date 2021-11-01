#include <string>
#include <iostream>
#include <vector>

#include <XiaoTuCVBox/Image.h>
#include <XiaoTuCVBox/ImageUtils.h>
#include <XiaoTuCVBox/VideoCapture.h>
#include <XiaoTuCVBox/QrCodeDetector.h>

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
            capture->EnumFmt();
            capture->ReleaseBufferInUse();
            break;
        case 'g':
            std::cout << "---- V4L2_CID_BRIGHTNESS -----" << std::endl;
            capture->QueryCtrl(V4L2_CID_BRIGHTNESS);
            std::cout << "---- V4L2_CID_CONTRAST -----" << std::endl;
            capture->QueryCtrl(V4L2_CID_CONTRAST);
            std::cout << "---- V4L2_CID_SATURATION -----" << std::endl;
            capture->QueryCtrl(V4L2_CID_SATURATION);
            std::cout << "---- V4L2_CID_HUE -----" << std::endl;
            capture->QueryCtrl(V4L2_CID_HUE);

            std::cout << "---- V4L2_CID_EXPOSURE -----" << std::endl;
            capture->QueryCtrl(V4L2_CID_EXPOSURE);

            std::cout << "---- V4L2_CID_AUTOGAIN -----" << std::endl;
            capture->QueryCtrl(V4L2_CID_AUTOGAIN);

            std::cout << "---- V4L2_CID_GAIN -----" << std::endl;
            capture->QueryCtrl(V4L2_CID_GAIN);

            std::cout << "Gain:" << capture->GetCtrlInteger(V4L2_CID_GAIN) << std::endl;
            std::cout << "亮度:" << capture->GetCtrlInteger(V4L2_CID_BRIGHTNESS) << std::endl;
            std::cout << "对比度:" << capture->GetCtrlInteger(V4L2_CID_CONTRAST) << std::endl;
            std::cout << "饱和度:" << capture->GetCtrlInteger(V4L2_CID_SATURATION) << std::endl;
            std::cout << "色度:" << capture->GetCtrlInteger(V4L2_CID_HUE) << std::endl;

            break;
        case 'h':
            capture->SetCtrlInteger(V4L2_CID_GAIN, 10);
            break;
        case 'H':
            capture->SetCtrlInteger(V4L2_CID_GAIN, 64);
            break;
        case 'i':
            capture->SetCtrlInteger(V4L2_CID_BRIGHTNESS, -32);
            break;
        case 'I':
            capture->SetCtrlInteger(V4L2_CID_BRIGHTNESS, 32);
            break;
        case 'j':
            capture->SetCtrlInteger(V4L2_CID_CONTRAST, 10);
            break;
        case 'J':
            capture->SetCtrlInteger(V4L2_CID_CONTRAST, 90);
            break;
        case 'k':
            capture->SetCtrlInteger(V4L2_CID_SATURATION, 10);
            break;
        case 'K':
            capture->SetCtrlInteger(V4L2_CID_SATURATION, 110);
            break;
        case 'l':
            capture->SetCtrlInteger(V4L2_CID_HUE, -110);
            break;
        case 'L':
            capture->SetCtrlInteger(V4L2_CID_HUE, 110);
            break;
        default:
            break;
    }
}

void OnRead(xiaotu::cv::VideoCapture::BufferPtr const & buffer)
{
    gBuffer = buffer;
}

int main(int argc, char *argv[])
{
    if (2 != argc) {
        std::cout << "./t_stdin_v4l2_talk /dev/video0" << std::endl;
        exit(-1);
    }

    std::string dev_path(argv[1]);
    gBuffer.reset();

    xiaotu::cv::ArtMarkerDetector detector;
    std::vector<xiaotu::cv::ArtMarker> markers;

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
            cv::Mat gray(img_size, CV_8UC1);
            cv::cvtColor(yuyv, bgr, cv::COLOR_YUV2BGR_YUYV);
            cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);

            int n = detector.ScanGrayImg(gray, markers);
            detector.Draw(bgr, markers);
            std::cout << "检查到 " << n << " 个二维码" << std::endl;
            for (int i = 0; i < n; i++) {
                std::cout << "[" << i << "]" << markers[i].msg << std::endl;
            }

            cv::imshow("bgr", bgr);
            cv::waitKey(1);

            gBuffer.reset();
            capture->ReleaseBufferInUse();
        }
    }
    return 0;
}



#pragma once
#include <aruco.h>
#include <memory>
#include <opencv2/highgui/highgui.hpp>


class MemoryBuffer;
class Capture
{
    cv::VideoCapture TheVideoCapturer;
    cv::Mat TheInputImage;

    std::shared_ptr<MemoryBuffer> m_writeBuffer;
    std::shared_ptr<MemoryBuffer> m_readBuffer;

public:
    Capture();
    ~Capture();
    bool Open(int cameraID);
    void Update();
    cv::Mat GetImage()const;
};


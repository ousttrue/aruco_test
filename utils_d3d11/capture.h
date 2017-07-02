#pragma once
#include <opencv2/core/core.hpp>
#include <memory>


namespace cv {
	class VideoCapture;
}

class Capture
{
    std::unique_ptr<cv::VideoCapture> m_TheVideoCapturer;
    bool m_TheCaptureFlag;
	cv::Mat m_currentFrame;

public:
	Capture();
    ~Capture();
    void SetCaptureFlag(bool enable){ m_TheCaptureFlag=enable; }
    bool Open(int input);
    bool Open(const std::string &input);
    void Resize(const cv::Mat &src, int w, int h, cv::Mat &dst);
    bool GetFrame(cv::Mat &image);
};


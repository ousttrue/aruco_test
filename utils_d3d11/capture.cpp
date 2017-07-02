#include "capture.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


Capture::Capture()
    : m_TheVideoCapturer(new cv::VideoCapture), m_TheCaptureFlag(true)
{
}

Capture::~Capture()
{}

bool Capture::Open(int input)
{
    m_TheVideoCapturer->open(input);
    if (!m_TheVideoCapturer->isOpened()) {
        return false;
    }
    return true;
}

bool Capture::Open(const std::string &input)
{
    if (input == "live") m_TheVideoCapturer->open(0);
    else m_TheVideoCapturer->open(input);

    if(!m_TheVideoCapturer->isOpened()){
        return false;
    }
    return true;
}

void Capture::Resize(const cv::Mat &src, int iWidth, int iHeight, cv::Mat &dst)
{
	auto TheGlWindowSize = cv::Size(iWidth, iHeight);
	//not all sizes are allowed. OpenCv images have padding at the end of each line in these that are not aligned to 4 bytes
	if (iWidth * 3 % 4 != 0) {
		iWidth += iWidth * 3 % 4;//resize to avoid padding
		Resize(src, iWidth, TheGlWindowSize.height, dst);
	}
	else {
		//resize the image to the size of the GL window
		cv::resize(src, dst, TheGlWindowSize);
	}
}

bool Capture::GetFrame(cv::Mat &image){
    if (!m_TheCaptureFlag) {
        return false;
    }

    //capture image
    m_TheVideoCapturer->grab();
    m_TheVideoCapturer->retrieve(image);

    return true;
}


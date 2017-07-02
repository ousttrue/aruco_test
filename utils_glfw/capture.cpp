#include "capture.h"
#include "memorybuffer.h"
#include <Windows.h>
#include <assert.h>
#include <opencv2/imgproc/imgproc.hpp>


Capture::Capture()
    : 
    m_writeBuffer(new MemoryBuffer("CAPTURE_IMAGE_BUFFER", 1024 * 1024 * 512)),
    m_readBuffer(new MemoryBuffer("CAPTURE_IMAGE_BUFFER"))
{
}

Capture::~Capture()
{
}

bool Capture::Open(int cameraID)
{
    /*
    if (inputVideo=="live"){
        TheVideoCapturer.open(cameraID);
    }
    else{
        TheVideoCapturer.open(inputVideo);
    }
    */

    TheVideoCapturer.open(cameraID);
    if(!TheVideoCapturer.isOpened()){
        return false;
    }

    //read first image
    TheVideoCapturer>>TheInputImage;

    return true;
}

void Capture::Update()
{
    TheVideoCapturer.grab();
    TheVideoCapturer.retrieve( TheInputImage);
    // transform color that by default is BGR to RGB 
    // because windows systems do not allow reading BGR images with opengl properly
    cv::cvtColor(TheInputImage, TheInputImage, CV_BGR2RGB);

    m_writeBuffer->Write(TheInputImage.ptr(), 
            TheInputImage.size().width * TheInputImage.size().height * 3);
}

cv::Mat Capture::GetImage()const
{ 
    auto p=m_readBuffer->GetPointer();
    if(!p){
        return cv::Mat(cv::Size(640, 480), CV_8UC3);
    }
    return cv::Mat(cv::Size(640, 480), CV_8UC3, p);
}


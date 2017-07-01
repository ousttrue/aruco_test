#pragma once
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <aruco.h>
#include <vector>


class Renderer
{
    std::string TheInputVideo;
    bool The3DInfoAvailable = false;
    float TheMarkerSize = 0.05f;
    aruco::MarkerDetector PPDetector;
    cv::VideoCapture TheVideoCapturer;
    std::vector<aruco::Marker> TheMarkers;
    cv::Mat TheInputImage;
    cv::Mat TheResizedImage;
    aruco::CameraParameters TheCameraParams;
    cv::Size TheGlWindowSize;
    bool TheCaptureFlag = true;
    bool m_initialized = false;

public:
    void ToggleCaptureFlag() { TheCaptureFlag = !TheCaptureFlag; }
    int InputWidth()const { return TheInputImage.size().width; }
    int InputHeight()const { return TheInputImage.size().height; }
    bool Initialize(int videoId);
    void Draw();
    void Update();
    void Resize(int w, int h);

private:
    void DrawCaptureImage();
    void DrawAxis(float size);
};

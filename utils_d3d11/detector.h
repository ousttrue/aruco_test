#pragma once
#include <opencv2/core/core.hpp>
#include <memory>


class DetectorImpl;
class Detector
{
    std::unique_ptr<DetectorImpl> m_pImpl;
    float m_TheMarkerSize;
public:
    Detector();
    ~Detector();
    void SetMakerSize(float size){ m_TheMarkerSize=size; }
	float GetMarkerSize()const{ return m_TheMarkerSize; }
    size_t Detect(const cv::Mat &input, const cv::Mat &cameraMatrix);
    void GetModelViewMatrix(size_t index, float matrix[16]);
    void GetProjectionMatrix(const cv::Size &cameraSize, const cv::Size &renderSize,
            const cv::Mat &CameraMatrix,
            float matrix[16], double zNear, double zFar);
};


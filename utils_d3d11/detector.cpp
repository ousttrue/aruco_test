#include "detector.h"
#include <aruco.h>


class DetectorImpl
{
    aruco::MarkerDetector m_PPDetector;
public:
    std::vector<aruco::Marker> TheMarkers;

    void Detect(const cv::Mat &input, const cv::Mat &cameraMatrix
            , float markerSize)
    {
		m_PPDetector.detect(input, TheMarkers
                , cameraMatrix, cv::Mat(), markerSize, false);
    }
};


Detector::Detector()
: m_pImpl(new DetectorImpl), m_TheMarkerSize(0.05f)
{}

Detector::~Detector()
{}

size_t Detector::Detect(const cv::Mat &input, const cv::Mat &cameraMatrix)
{
    m_pImpl->Detect(input, cameraMatrix, m_TheMarkerSize);
    return m_pImpl->TheMarkers.size();
}

void Detector::GetModelViewMatrix(size_t index, float modelview_matrix[16])
{
    double matrix[16];
    m_pImpl->TheMarkers[index].glGetModelViewMatrix(matrix);
    for (int i = 0; i < 16; ++i) {
        modelview_matrix[i] = matrix[i];
    }
}

void Detector::GetProjectionMatrix(const cv::Size &orgImgSize, const cv::Size &size,
        const cv::Mat &CameraMatrix,
        float proj_matrix[16], double gnear, double gfar)
{
    //Deterime the rsized info
    double Ax=double(size.width)/double(orgImgSize.width);
    double Ay=double(size.height)/double(orgImgSize.height);
    double _fx=CameraMatrix.at<float>(0,0)*Ax;
    double _cx=CameraMatrix.at<float>(0,2)*Ax;
    double _fy=CameraMatrix.at<float>(1,1)*Ay;
    double _cy=CameraMatrix.at<float>(1,2)*Ay;
    double cparam[3][4] =
    {
        {
            _fx,  0,  _cx,  0
        },
        {0,          _fy,  _cy, 0},
        {0,      0,      1,      0}
    };

	bool invert =false;
    double matrix[16];
	aruco::CameraParameters::argConvGLcpara2(cparam, size.width, size.height, gnear, gfar, matrix, invert, true);
    for (int i = 0; i < 16; ++i) {
        proj_matrix[i] = matrix[i];
    }
}

#include "ArUcoImpl.h"
#include <glm/gtc/matrix_transform.hpp>


//-----------------------------------------------------------------------------
ArUcoImpl::ArUcoImpl()
    : m_right_handed(true), m_y_up(true), Distorsion(cv::Mat()), CameraMatrix(cv::Mat())
{
    Distorsion=cv::Mat::zeros(4,1,CV_32FC1);
    CameraMatrix=cv::Mat::eye(3,3,CV_32FC1);
}

ArUcoImpl::~ArUcoImpl()
{
}

void ArUcoImpl::Load(const std::string &xml, const cv::Size &size)
{
    aruco::CameraParameters TheCameraParams;

    //read camera paramters if passed
    TheCameraParams.readFromXMLFile(xml);

    TheCameraParams.resize(size);

    CameraMatrix=TheCameraParams.CameraMatrix;
    Distorsion=TheCameraParams.Distorsion;
}

void ArUcoImpl::SetIntrinsics(float fovy_radian, int w, int h)
{
    float cx=w*0.5;
    float cy=h*0.5;
    float fy=cy/tan(fovy_radian*0.5f);
    float fx=fy;
    CameraMatrix.at<float>(0,0)=fx;
    CameraMatrix.at<float>(0,2)=cx;
    CameraMatrix.at<float>(1,1)=fy;
    CameraMatrix.at<float>(1,2)=cy;
}

void ArUcoImpl::GetProjectionMatrix(float proj_matrix[16], const cv::Size &inputImageSize)
{
    double m[16];
    aruco::CameraParameters::glGetProjectionMatrix(CameraMatrix, Distorsion,
            inputImageSize,
            inputImageSize,
            m, 
            0.05, 10,
            m_y_up, m_right_handed);
    std::copy(m, m+16, proj_matrix);
}

void ArUcoImpl::Detect(const cv::Mat &TheInputImage, float markerSize)
{
    PPDetector.detect(TheInputImage, TheMarkers, 
            CameraMatrix, cv::Mat(), markerSize);
}

void ArUcoImpl::GetModelViewMatrix(size_t index, float modelview_matrix[16])
{
    double m[16];
    TheMarkers[index].glGetModelViewMatrix(m);
    std::copy(m, m+16, modelview_matrix);

    if(!m_right_handed){
        modelview_matrix[8]*=-1.0f;
        modelview_matrix[9]*=-1.0f;
        modelview_matrix[10]*=-1.0f;
        modelview_matrix[11]*=-1.0f;

        modelview_matrix[2]*=-1.0f;
        modelview_matrix[6]*=-1.0f;
        modelview_matrix[10]*=-1.0f;
        modelview_matrix[14]*=-1.0f;
    }

    if(m_y_up){
        modelview_matrix[1]*=-1.0f;
        modelview_matrix[5]*=-1.0f;
        modelview_matrix[9]*=-1.0f;
        modelview_matrix[13]*=-1.0f;
    }
}

int ArUcoImpl::GetMarkerID(size_t index)const
{
    return TheMarkers[index].id;
}


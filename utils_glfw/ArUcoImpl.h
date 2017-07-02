#pragma once
#pragma warning(push)
#pragma warning(disable:4290)
#include <aruco.h>
#pragma warning(pop)


class ArUcoImpl
{
    aruco::MarkerDetector PPDetector;
    std::vector<aruco::Marker> TheMarkers;

    cv::Mat CameraMatrix;
    cv::Mat Distorsion;

    bool m_right_handed;
    bool m_y_up;

public:
    ArUcoImpl();
    ~ArUcoImpl();

    void SetYUp(bool y_up){ m_y_up=y_up; }
    bool GetYUp()const{ return m_y_up; }

    void SetRightHanded(bool right_handed){ m_right_handed=right_handed; }
    bool GetRightHanded()const{ return m_right_handed; }

    void Load(const std::string &xml, const cv::Size &size);
    void SetIntrinsics(float fovy_radian, int w, int h);

    void GetProjectionMatrix(float proj_matrix[16], const cv::Size &inputImageSize);
    void Detect(const cv::Mat &TheInputImage, float markerSize);
    size_t GetMarkerCount()const{ return TheMarkers.size(); }
    void GetModelViewMatrix(size_t m, float modelview_matrix[16]);
    int GetMarkerID(size_t index)const;
};


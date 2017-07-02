#pragma once
#include <memory>


class Capture;
class ArUcoImpl;
class OpenGL;
class MemoryBuffer;
class ARApp
{
    std::shared_ptr<Capture> m_capture;
    std::shared_ptr<ArUcoImpl> m_ar;
    std::shared_ptr<OpenGL> m_gl;
    std::shared_ptr<MemoryBuffer> m_buffer;
    bool TheCaptureFlag;
    double m_fovy_radian;
    float m_markerSize;

public:
    ARApp(std::shared_ptr<Capture> capture, 
            std::shared_ptr<ArUcoImpl> ar, std::shared_ptr<OpenGL> gl);
    void draw();
    void update();

    bool getEnableCapture()const{ return TheCaptureFlag; }
    void setEnableCapture(bool enable){ TheCaptureFlag=enable; }

    bool getYUp()const;
    void setYUp(bool y_up);

    bool getRightHanded()const;
    void setRightHanded(bool right_handed);

    float getFovyRadians()const
    {
        return m_fovy_radian;
    }

    void setFovyRadians(float radians);
};


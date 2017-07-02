#include "arapp.h"
#include "capture.h"
#include "ArUcoImpl.h"
#include "opengl.h"
#include "udpsender.h"
#include "memorybuffer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define _USE_MATH_DEFINES
#include <math.h>


static double toRadian(double degree)
{
    return M_PI * degree/180.0;
}

static double toDegree(double radian)
{
    return radian/M_PI * 180.0;
}


ARApp::ARApp(std::shared_ptr<Capture> capture,
            std::shared_ptr<ArUcoImpl> ar, std::shared_ptr<OpenGL> gl)
    : m_capture(capture), m_ar(ar), m_gl(gl), 
        m_buffer(new MemoryBuffer("CAPTURE_IMAGE_BUFFER")),
        TheCaptureFlag(true),
        m_fovy_radian(toRadian(30.0f)),
        m_markerSize(0.01f)
{
}

bool ARApp::getYUp()const
{
    return m_ar->GetYUp();
}

void ARApp::setYUp(bool y_up)
{
    m_ar->SetYUp(y_up);
}

bool ARApp::getRightHanded()const
{
    return m_ar->GetRightHanded();
}

void ARApp::setRightHanded(bool right_handed)
{
    m_ar->SetRightHanded(right_handed);
}

void ARApp::setFovyRadians(float radians)
{
    m_fovy_radian=radians;
    if(m_fovy_radian<0){
        m_fovy_radian=0;
    }
    else if(m_fovy_radian>M_PI){
        m_fovy_radian=M_PI;
    }
}

void ARApp::update()
{
    if (TheCaptureFlag) {
        m_capture->Update();
        m_ar->Detect(m_capture->GetImage(), m_markerSize);
    }
}

void ARApp::draw()
{
    m_gl->Clear();

    auto &image=m_capture->GetImage();
    m_gl->DrawImage(image.size().width, image.size().height, image.ptr(0));

    m_ar->SetIntrinsics(m_fovy_radian, image.size().width, image.size().height);

    ///Set the appropriate projection matrix so that rendering is done in a enrvironment
    //like the real camera (without distorsion)
    glm::mat4 proj_matrix;

#if 1
    m_ar->GetProjectionMatrix(&proj_matrix[0][0], m_capture->GetImage().size());
    m_gl->SetProjectionMatrix(&proj_matrix[0][0]);
#else
    auto proj_matrix2 = glm::perspective<float>(toDegree(m_fovy_radian),
            ((float)image.size().width)/image.size().height,
            0.05, 10.0);

    m_gl->SetProjectionMatrix(&proj_matrix2[0][0]);
#endif

    //now, for each marker,
    glm::mat4 modelview_matrix;
    size_t markerCount=m_ar->GetMarkerCount();
    for(unsigned int i=0; i<markerCount; i++)
    {
        m_ar->GetModelViewMatrix(i, &modelview_matrix[0][0]);

        m_gl->DrawAxis(&modelview_matrix[0][0], m_markerSize);

        if(i==0){
            std::cout
                << m_ar->GetYUp() << ":" << m_ar->GetRightHanded()
                << ", fovy " << toDegree(m_fovy_radian) << "degree "
                << "[marker]" << m_ar->GetMarkerID(i) << std::endl;
            sendMatrix(&glm::inverse(modelview_matrix)[0][0]);
        }
    }
}


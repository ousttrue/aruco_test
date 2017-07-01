#include "renderer.h"
#include <Windows.h>
#include <GL/glut.h>


bool Renderer::Initialize(int videoId)
{
    TheVideoCapturer.open(videoId);
    if (!TheVideoCapturer.isOpened())
    {
        std::cerr << "Could not open video" << std::endl;
        return false;
    }

    //read first image
    TheVideoCapturer >> TheInputImage;
    //read camera paramters if passed
    TheCameraParams.resize(TheInputImage.size());

    TheGlWindowSize = TheInputImage.size();

    return true;
}

void Renderer::Draw()
{
    if (TheResizedImage.rows == 0) //prevent from going on until the image is initialized
        return;

    if (!m_initialized) {
        m_initialized = true;
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClearDepth(1.0);
    }

    ///clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ///draw image in the buffer
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, TheGlWindowSize.width, 0, TheGlWindowSize.height, -1.0, 1.0);
    glViewport(0, 0, TheGlWindowSize.width, TheGlWindowSize.height);
    glDisable(GL_TEXTURE_2D);
    glPixelZoom(1, -1);
    glRasterPos3f(0, TheGlWindowSize.height - 0.5, -1.0);
    glDrawPixels(TheGlWindowSize.width, TheGlWindowSize.height, GL_RGB, GL_UNSIGNED_BYTE, TheResizedImage.ptr(0));
    ///Set the appropriate projection matrix so that rendering is done in a enrvironment
    //like the real camera (without distorsion)
    glMatrixMode(GL_PROJECTION);
    double proj_matrix[16];
    TheCameraParams.glGetProjectionMatrix(TheInputImage.size(), TheGlWindowSize, proj_matrix, 0.05, 10);
    glLoadIdentity();
    glLoadMatrixd(proj_matrix);

    //now, for each marker,
    double modelview_matrix[16];
    for (unsigned int m = 0; m < TheMarkers.size(); m++)
    {
        TheMarkers[m].glGetModelViewMatrix(modelview_matrix);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glLoadMatrixd(modelview_matrix);


        axis(TheMarkerSize);

        glColor3f(1, 0.4, 0.4);
        glTranslatef(0, 0, TheMarkerSize / 2);
        glPushMatrix();
        glutWireCube(TheMarkerSize);

        glPopMatrix();
    }

}

void Renderer::Update()
{
    if (TheCaptureFlag) {
        //capture image
        TheVideoCapturer.grab();
        TheVideoCapturer.retrieve(TheInputImage);
        TheUndInputImage.create(TheInputImage.size(), CV_8UC3);
        //transform color that by default is BGR to RGB because windows systems do not allow reading BGR images with opengl properly
        cv::cvtColor(TheInputImage, TheInputImage, CV_BGR2RGB);
        //remove distorion in image
        cv::undistort(TheInputImage, TheUndInputImage, TheCameraParams.CameraMatrix, TheCameraParams.Distorsion);
        //detect markers
        PPDetector.detect(TheUndInputImage, TheMarkers, TheCameraParams.CameraMatrix, cv::Mat(), TheMarkerSize, false);
        //resize the image to the size of the GL window
        cv::resize(TheUndInputImage, TheResizedImage, TheGlWindowSize);
    }
}

void Renderer::Resize(int w, int h)
{
    TheGlWindowSize = cv::Size(w, h);
    //resize the image to the size of the GL window
    if (TheUndInputImage.rows != 0)
        cv::resize(TheUndInputImage, TheResizedImage, TheGlWindowSize);
}

void Renderer::axis(float size)
{
    glColor3f(1, 0, 0);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f); // origin of the line
    glVertex3f(size, 0.0f, 0.0f); // ending point of the line
    glEnd();

    glColor3f(0, 1, 0);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f); // origin of the line
    glVertex3f(0.0f, size, 0.0f); // ending point of the line
    glEnd();


    glColor3f(0, 0, 1);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f); // origin of the line
    glVertex3f(0.0f, 0.0f, size); // ending point of the line
    glEnd();
}


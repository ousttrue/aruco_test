#include "opengl.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


static void axis(float size)
{
    glPushMatrix();

    glColor3f (1,0,0 );
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f); // origin of the line
    glVertex3f(size,0.0f, 0.0f); // ending point of the line
    glEnd( );

    glColor3f ( 0,1,0 );
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f); // origin of the line
    glVertex3f( 0.0f,size, 0.0f); // ending point of the line
    glEnd( );

    glColor3f (0,0,1 );
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f); // origin of the line
    glVertex3f(0.0f, 0.0f, size); // ending point of the line
    glEnd( );

    glPopMatrix();
}

static void square(float size)
{
    size*=0.5f;

    glPushMatrix();
    glColor3f (1,0,0 );
    glBegin(GL_LINES);
    glVertex3f(-size, -size, 0);
    glVertex3f(-size, size, 0);

    glVertex3f(-size, size, 0);
    glVertex3f(size, size, 0);

    glVertex3f(size, size, 0);
    glVertex3f(size, -size, 0);

    glVertex3f(size, -size, 0);
    glVertex3f(-size, -size, 0);
    glEnd( );
    glPopMatrix();
}


void OpenGL::Clear()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void OpenGL::DrawImage(int width, int height, const unsigned char *p)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1.0, 1.0);
    glViewport(0, 0, width , height);
    glDisable(GL_TEXTURE_2D);
    glPixelZoom( 1, -1);
    glRasterPos3f( 0, height  - 0.5, -1.0 );
    glDrawPixels (width, height, GL_RGB, GL_UNSIGNED_BYTE, p);
}

void OpenGL::SetProjectionMatrix(const float proj_matrix[16])
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf(proj_matrix);
}

void OpenGL::DrawAxis(const float modelview_matrix[16], float markerSize)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(modelview_matrix);

    glLineWidth(1);
    axis(markerSize);

    glLineWidth(4);
    square(markerSize);
}


/*****************************
Copyright 2011 Rafael Muñoz Salinas. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Rafael Muñoz Salinas ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Rafael Muñoz Salinas OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Rafael Muñoz Salinas.
********************************/
#include <iostream>
#include <fstream>
#include <sstream>

#ifdef __APPLE__
#include <GLUT/glut.h>
#elif defined(_MSC_VER)
#include <Windows.h>
#include <GL/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include "renderer.h"


Renderer *g_renderer=nullptr;


void vMouse(int b, int s, int x, int y)
{
    if (b == GLUT_LEFT_BUTTON && s == GLUT_DOWN) {
        g_renderer->ToggleCaptureFlag();
    }
}


void vDrawScene()
{
    g_renderer->Draw();
    glutSwapBuffers();
}


void vIdle()
{
    g_renderer->Update();
    glutPostRedisplay();
}


void vResize(GLsizei iWidth, GLsizei iHeight)
{
    //not all sizes are allowed. 
    // OpenCv images have padding at the end of each line 
    // in these that are not aligned to 4 bytes
    if (iWidth * 3 % 4 != 0) {
        iWidth += iWidth * 3 % 4;//resize to avoid padding
        vResize(iWidth, iHeight);
    }
    else {
        g_renderer->Resize(iWidth, iHeight);
    }
}


int main(int argc, char **argv)
{
    g_renderer = new Renderer();

    if (!g_renderer->Initialize(1)) {
        return 1;
    }

    glutInit(&argc, argv);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(g_renderer->InputWidth(), g_renderer->InputHeight());
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow("AruCo");
    glutDisplayFunc(vDrawScene);
    glutIdleFunc(vIdle);
    glutReshapeFunc(vResize);
    glutMouseFunc(vMouse);
    vResize(g_renderer->InputWidth(), g_renderer->InputHeight());
    glutMainLoop();

    // not reach here
    return 0;
}


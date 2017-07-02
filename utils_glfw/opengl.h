#pragma once
#include <windows.h>
#include <GL/gl.h>


class OpenGL
{
public:
    void Clear();
    void DrawImage(int w, int h, const unsigned char *p);
    void SetProjectionMatrix(const float proj_matrix[16]);
    void DrawAxis(const float modelview_matrix[16], float markerSize);
};


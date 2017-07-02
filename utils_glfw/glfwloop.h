#pragma once
#include <memory>


class ARApp;
class GLFWwindow;
class GlfwLoop
{
    GLFWwindow *m_window;
public:
    GlfwLoop();
    ~GlfwLoop();
    bool initialize(int w, int h);
    int run(std::shared_ptr<ARApp> app);
};


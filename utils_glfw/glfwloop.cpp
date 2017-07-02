#include <GLFW/glfw3.h>
#include "glfwloop.h"
#include "arapp.h"
#define _USE_MATH_DEFINES
#include <math.h>

std::shared_ptr<ARApp> g_app;


double toRadian(double degree)
{
    return M_PI * degree/180.0;
}

double toDegree(double radian)
{
    return radian/M_PI * 180.0;
}


static void vMouse(int b,int s,int x,int y)
{
    /*
    if (b==GLUT_LEFT_BUTTON && s==GLUT_DOWN) {
        g_app->setEnableCapture(g_app->getEnableCapture());
    }
    */
}

static void vKey(GLFWwindow*, int key, int scancode, int action, int mods) 
{
    switch(key)
    {
        case 'q':
        case 'Q':
        case '\033':  /* '\033' は ESC の ASCII コード */
			g_app=0;
            exit(0);
            break;

        case 'y':
            g_app->setYUp(!g_app->getYUp());
            break;

        case 'z':
            g_app->setRightHanded(!g_app->getRightHanded());
            break;

        case 'k':
            g_app->setFovyRadians(g_app->getFovyRadians()+M_PI/36.0);
            break;

        case 'j':
            g_app->setFovyRadians(g_app->getFovyRadians()-M_PI/36.0);
            break;

        default:
            printf("Key = %c\n" , key);
            break;
    }
}

static void vResize(GLFWwindow*, int w, int h)
{
}

GlfwLoop::GlfwLoop()
    : m_window(0)
{}

GlfwLoop::~GlfwLoop()
{
    glfwTerminate();
}

bool GlfwLoop::initialize(int w, int h)
{
    if(!glfwInit()){
        return false;
    }

    // Create a windowed mode window and its OpenGL context
    m_window = glfwCreateWindow(640, 480, 
            "Hello World", NULL, NULL);
    if (!m_window)
    {
        return false;
    }

    // Make the window's context current
    glfwMakeContextCurrent(m_window);

    glfwSetWindowSizeCallback(m_window, vResize);
    glfwSetKeyCallback(m_window, vKey);
    //glfwSetMouseButtonCallback(m_window, vMouseButton);
    //glfwSetCursorPosCallback(m_window, vMouseMove);

    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glClearDepth( 1.0 );
    return true;
}

int GlfwLoop::run(std::shared_ptr<ARApp> app)
{
	g_app=app;

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(m_window))
    {
        // Render here
        g_app->update();
        g_app->draw();

        // Swap front and back buffers
        glfwSwapBuffers(m_window);

        // Poll for and process events
        glfwPollEvents();
    }

    // not reach here
    return 0;
}


#include "capture.h"
#include "ArUcoImpl.h"
#include "opengl.h"
#include "arapp.h"
#include "glfwloop.h"


int main(int argc, char **argv)
{
    int camera=1;
    if(argc>1){
        camera=atoi(argv[1]);
    }

    auto capture=std::make_shared<Capture>();
    if(!capture->Open(camera)){
        std::cerr<<"Could not open video"<<std::endl;
        return 1;
    }
    auto &image=capture->GetImage();

    auto ar=std::make_shared<ArUcoImpl>();

    auto gl=std::make_shared<OpenGL>();

    GlfwLoop loop;
    if(!loop.initialize(image.size().width, image.size().height)){
        return 1;
    }

    return loop.run(std::make_shared<ARApp>(capture, ar, gl));
}


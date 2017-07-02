------------------------------------------------------------------------------
-- Project
------------------------------------------------------------------------------
project "aruco_test_gl"
--language "C"
language "C++"
--kind "StaticLib"
--kind "SharedLib"
kind "ConsoleApp"
--kind "WindowedApp"

flags {
    --"WinMain",
}
files {
    "*.cpp", "*.h",
    "../common/*.cpp",
    "../common/*.h",
}

defines {
}
includedirs {
    "../include",
    "../common",
    "../../aruco",
    OPENCV_INCLUDE,
    "../glut-3.7.6-bin",
}
buildoptions {
}

libdirs {
    OPENCV_LIB,
    LIB,
    "../glut-3.7.6-bin",
}
links {
    "aruco",
    --"strmiids",
    "vfw32",
    "comctl32",
}
linkoptions {
}

prebuildcommands {
}

--debugdir "../testdata"

debugargs {
    "1",
}

configuration "Debug"
do
    links {
        "opencv_core247d",
        "opencv_calib3d247d",
        "opencv_features2d247d",
        "opencv_flann247d",
        "opencv_highgui247d",
        "opencv_imgproc247d",
        "libjpegd",
        "zlibd",
        "libpngd",
        "libjasperd",
        "libtiffd",
        "IlmImfd",
    }
end

configuration "Release"
do
    links {
        "opencv_core247",
        "opencv_calib3d247",
        "opencv_features2d247",
        "opencv_flann247",
        "opencv_highgui247",
        "opencv_imgproc247",
        "libjpeg",
        "zlib",
        "libpng",
        "libjasper",
        "libtiff",
        "IlmImf",
    }
end


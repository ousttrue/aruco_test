configuration {}
local GLUT_DIR="C:/CMakeInstall/"
local OPENCV_DIR="C:/opencv-249/build/"
local OPENCV_LIBS={
        "opencv_videostab249",
        "opencv_video249",
        "opencv_ts249",
        "opencv_superres249",
        "opencv_stitching249",
        "opencv_photo249",
        "opencv_ocl249",
        "opencv_objdetect249",
        "opencv_nonfree249",
        "opencv_ml249",
        "opencv_legacy249",
        "opencv_imgproc249",
        "opencv_highgui249",
        "opencv_gpu249",
        "opencv_flann249",
        "opencv_features2d249",
        "opencv_core249",
        "opencv_contrib249",
        "opencv_calib3d249",
        "zlib",
        "libjpeg",
        "libpng",
        "libtiff",
        "libjasper",
        "IlmImf",
}
function map(f, t)
  local t2 = {}
  for k,v in pairs(t) do t2[k] = f(v) end
  return t2
end

-----------------------------------------------------------------------------
project "aruco_test_d3d11"
language "C++"
kind "ConsoleApp"
files {
    "*.cpp",
    "*.h",
    "*.hlsl",
}
defines {
}
includedirs {
    "../src",
    "../DxgiUtil",
    OPENCV_DIR.."include",
}
libdirs {
    OPENCV_DIR.."x86/vc12/staticlib",
}
links {
    "aruco",
    "DxgiUtil",

    "gdi32",
    "winspool",
    "shell32",
    "ole32",
    "oleaut32",
    "uuid",
    "comdlg32",
    "advapi32",
    "comctl32",
    "vfw32",
    "winmm",
    --"OpenGL32",
    --"GLU32",
    "d3d11",
    "d2d1",
    "dwrite",
    "d3dcompiler",
    "dxguid",
    "dxgi",
}
configuration "Debug"
do
    links(map(function(lib) return lib.."d" end, OPENCV_LIBS))
end
configuration "Release"
do
    links(OPENCV_LIBS)
end


project "DxgiUtil"
--language "C
language "C++"
kind "StaticLib"
--kind "SharedLib"
--kind "ConsoleApp"
--kind "WindowedApp"

flags {
    "Unicode",
    --"WinMain",
}
files {
    "*.cpp", 
    "*.h",
    "*.hlsl",
}
includedirs {
    "$(BOOST_DIR)",
}
defines {
    "NOMINMAX",
}
buildoptions {
}
links {
    "strmiids",
    "dxgi",
    "d3d11",
    "d2d1",
    "dwrite",
    "d3dcompiler",
    "winmm",
}


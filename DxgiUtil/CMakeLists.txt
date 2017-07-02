INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src ${GNULIBS_INCLUDE_DIR})
LINK_LIBRARIES(${PROJECT_NAME} ${REQUIRED_LIBRARIES} )

FILE(GLOB SRCS *.cpp *.h)

ADD_LIBRARY(DxgiUtil STATIC ${SRCS})

TARGET_LINK_LIBRARIES(DxgiUtil
    "strmiids"
    "dxgi"
    "dxguid.lib"
    "d3d11"
    "d2d1"
    "dwrite"
    "d3dcompiler"
    "winmm"
    )


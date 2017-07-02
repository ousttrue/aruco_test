INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src ${GNULIBS_INCLUDE_DIR})
LINK_LIBRARIES(${PROJECT_NAME} ${REQUIRED_LIBRARIES} )

IF(OPENGL_LIBS)
    ADD_EXECUTABLE(aruco_test_gl 
        aruco_test_gl.cpp
        renderer.cpp renderer.h
        )
    TARGET_LINK_LIBRARIES(aruco_test_gl ${OPENGL_LIBS})

    #ADD_EXECUTABLE(aruco_test_markermap_gl aruco_test_markermap_gl.cpp)
    #TARGET_LINK_LIBRARIES(aruco_test_markermap_gl ${OPENGL_LIBS})

    #INSTALL(TARGETS aruco_test_gl aruco_test_markermap_gl   RUNTIME DESTINATION bin)
ENDIF()

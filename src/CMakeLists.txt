# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2017 Vangelis Tsiatsianas

add_executable(3D_Model_Viewer
    lib/tiny-file-dialogs/tinyfiledialogs.c
    GUI.cpp
    Environment.cpp
    Object.cpp
    PolygonMesh.cpp
    Main.cpp)

target_link_libraries(3D_Model_Viewer
    ${OPENGL_LIBRARIES}
    ${IMGUI}
    ${SDL2_LIBRARY}
    ${SDLMIXER_LIBRARY}
    ${OpenCV_LIBS}
    ${GLEW32}
    ${ASSIMP}
    ${SOIL2DEBUG}
    glGAMath
    glGA)

include(CTest)
add_test(3D_Model_Viewer ${CMAKE_SOURCE_DIR}/bin/3D_Model_Viewer)
set_tests_properties(3D_Model_Viewer PROPERTIES ENVIRONMENT DYLD_LIBRARY_PATH=${GLGA_PATH}/_thirdPartyLibs/lib/OSX)

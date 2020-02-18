# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2017 Vangelis Tsiatsianas

cmake_minimum_required(VERSION 3.9)

project(3D_Model_Viewer CXX C)

set(ASAN_FLAGS "-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer")
set(DISABLED_WARNINGS "-Wno-parentheses -Wno-#warnings -Wno-sign-compare -Wno-format-security   \
                       -Wno-address-of-packed-member -Wno-unused-variable -Wno-unused-function  \
                       -Wno-pointer-bool-conversion -Wno-embedded-directive")

set(CMAKE_CXX_FLAGS "-Wall ${DISABLED_WARNINGS} -pedantic -g3 ${ASAN_FLAGS}")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "-Wall ${DISABLED_WARNINGS} -pedantic -g3 ${ASAN_FLAGS}")
set(CMAKE_C_STANDARD 11)

set(GLGA_PATH <path_to_glGA>/glGA-edu)

include_directories(include
                    include/lib
                    ${GLGA_PATH}/libraries/include
                    ${GLGA_PATH}/libraries/include/glGA
                    ${GLGA_PATH}/libraries/include/glGAMath
                    ${GLGA_PATH}/libraries/include/KDTree
                    ${GLGA_PATH}/libraries/include/PRT
                    ${GLGA_PATH}/_thirdPartyLibs/include
                    ${GLGA_PATH}/_thirdPartyLibs/include/Bullet
                    ${GLGA_PATH}/_thirdPartyLibs/include/Assimp
                    ${GLGA_PATH}/_thirdPartyLibs/include/glew
                    ${GLGA_PATH}/_thirdPartyLibs/include/glm
                    ${GLGA_PATH}/_thirdPartyLibs/include/ImGUI
                    ${GLGA_PATH}/_thirdPartyLibs/include/SDL2
                    ${GLGA_PATH}/_thirdPartyLibs/include/SOIL2)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_LIBRARY_PATH ${GLGA_PATH}/_thirdPartyLibs/lib/OSX
                       ${GLGA_PATH}/libraries/lib)
find_package(OpenGL REQUIRED)
find_library(IMGUI libImGui.a)
find_library(GLEW32 libGLEW.a)
find_library(ASSIMP libassimpd.3.1.1.dylib)
find_library(SOIL2DEBUG libsoil2-debug.a)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${GLGA_PATH}/cmake)
add_subdirectory(${GLGA_PATH} lib)

enable_testing()
add_subdirectory(src)

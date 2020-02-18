// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2017 Vangelis Tsiatsianas

#pragma once

#define IMVEC4_POINTER(x) (reinterpret_cast<float *>(&(x)))
#define SETUP_UNIFORM(x)  (x##Uniform = static_cast<GLuint>(glGetUniformLocation(program, #x)))

namespace 3d_model_viewer {

constexpr auto rootDirectory = "<root_directory>";
constexpr auto audioFileName = "Dreamy.mp3";

constexpr float windowWidth = 1280;
constexpr float windowHeight = 720;

} // namespace 3d_model_viewer

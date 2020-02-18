#version 150 core

// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2017 Vangelis Tsiatsianas

out vec4 color;

uniform vec4 boundingBoxColor;

void main() {
    color = boundingBoxColor;
}

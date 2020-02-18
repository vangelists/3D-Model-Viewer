#version 150 core

// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2017 Vangelis Tsiatsianas

in vec3 vPosition;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 boundingBoxTransform;

void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * boundingBoxTransform * vec4(vPosition, 1.0);
}

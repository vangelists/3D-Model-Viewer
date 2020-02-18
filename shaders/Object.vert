#version 150 core

// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2017 Vangelis Tsiatsianas

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;
in ivec4 boneIDs;
in vec4 weights;

out vec3 worldPosition;
out vec3 worldEye;
out vec3 worldNormal;
out vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

const int MAX_BONES = 100;
uniform mat4 bones[MAX_BONES];
uniform uint vAnimationEnabled;

void main() {
    if (vAnimationEnabled != uint(0)) {
        mat4 boneTransform = bones[boneIDs[0]] * weights[0];
        boneTransform += bones[boneIDs[1]] * weights[1];
        boneTransform += bones[boneIDs[2]] * weights[2];
        boneTransform += bones[boneIDs[3]] * weights[3];

        worldPosition = (modelMatrix * boneTransform * vec4(vPosition, 1.0)).xyz;
        worldNormal = (modelMatrix * boneTransform * vec4(vNormal, 0.0)).xyz;

        gl_Position = projectionMatrix * viewMatrix * modelMatrix * boneTransform * vec4(vPosition, 1.0);
    } else {
        mat3 modelMatrixMat3 = mat3(modelMatrix);

        worldPosition = modelMatrixMat3 * vPosition;
        worldNormal = modelMatrixMat3 * vNormal;

        gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vPosition, 1.0);
    }

    worldEye = worldPosition;
    texCoord = vTexCoord;
}

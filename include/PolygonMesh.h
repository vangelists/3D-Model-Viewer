// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2017 Vangelis Tsiatsianas

#pragma once

#include <string>
#include <chrono>

#include "Object.h"
#include "Utilities.h"

#include <GL/glew.h>
#include <glGA/glGARigMesh.h>

namespace 3d_model_viewer {

using Point4 = glm::vec4;

constexpr int numVertices = 8;
constexpr int numIndices = 16;
constexpr unsigned maxBones = 100;

class PolygonMesh : public Object {
public:
    explicit PolygonMesh(const std::string& path, unsigned long id);

    void Initialize() override;
    void Render() override;
    void CleanUp() override;

    class BoundingBox {
    public:
        void Initialize();
        void Render(PolygonMesh& polygonMesh);

        GLuint program;
        GLuint boundingBoxColorUniform;

        glm::vec3 size;
        glm::vec3 center;
        glm::mat4 boundingBoxTransform;

        glm::vec3 max;
        glm::vec3 min;

    private:
        void SetupUniforms();

        Point4 vertices[numVertices] = {
            Point4(-0.5, -0.5,  0.5, 1.0),
            Point4(-0.5,  0.5,  0.5, 1.0),
            Point4( 0.5,  0.5,  0.5, 1.0),
            Point4( 0.5, -0.5,  0.5, 1.0),
            Point4(-0.5, -0.5, -0.5, 1.0),
            Point4(-0.5,  0.5, -0.5, 1.0),
            Point4( 0.5,  0.5, -0.5, 1.0),
            Point4( 0.5, -0.5, -0.5, 1.0)
        };

        GLushort indices[numIndices] = {
          0, 1, 2, 3,
          4, 5, 6, 7,
          0, 4, 1, 5,
          2, 6, 3, 7
        };

        GLuint vao;
        GLuint vbo[2];

        GLuint modelMatrixUniform;
        GLuint viewMatrixUniform;
        GLuint projectionMatrixUniform;
        GLuint boundingBoxTransformUniform;
        GLuint vAnimationEnabledUniform;
    };

    std::string path;
    RigMesh mesh;
    BoundingBox boundingBox;

private:
    void SetupUniforms() override;
    float GetRunningTime();

    GLuint vbo[5];

    GLuint boneUniforms[maxBones];
    GLuint hasTexturesUniform;
    GLuint vAnimationEnabledUniform;

    GLuint hasTextures;
    GLuint vAnimationEnabled;

    Time animationStartTime;
};

} // namespace 3d_model_viewer

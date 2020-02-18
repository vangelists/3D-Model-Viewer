// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2017 Vangelis Tsiatsianas

#include <algorithm>
#include <cctype>

#include "Common.h"
#include "PolygonMesh.h>"
#include "Utilities.h"

#include <glGA/glGAHelper.h>

#define ARRAY_SIZE(x) (sizeof(mesh.x[0]) * mesh.x.size())

#define SETUP_VBO(type, name)                                           \
    const auto sizeOf##name = ARRAY_SIZE(name);                         \
    glBindBuffer(type, vbo[vboCount++]);                                \
    glBufferData(type, sizeOf##name, mesh.name.data(), GL_STATIC_DRAW);

#define GET_AND_ENABLE_ATTRIBUTE(x)                                         \
    const auto x = static_cast<GLuint>(glGetAttribLocation(program, #x));   \
    glEnableVertexAttribArray(x);

#define RESET_VBO_COUNT() (vboCount = 0)

#define SETUP_UNIFORM_ARRAY(array, index, x)                                    \
    ((array)[index] = static_cast<GLuint>(glGetUniformLocation(program, x)))

#define UPLOAD_MESH_UNIFORM_MATRIX_4FV(x)                                       \
    glUniformMatrix4fv(x##Uniform, 1, GL_FALSE, glm::value_ptr(polygonMesh.x));

#define UPLOAD_UNIFORM_MATRIX_4FV(x)                                \
    glUniformMatrix4fv(x##Uniform, 1, GL_FALSE, glm::value_ptr(x));

#define UPLOAD_UNIFORM_BONE_TRANSFORM(x)                                                                                    \
    glUniformMatrix4fv(boneUniforms[x], 1, GL_TRUE, reinterpret_cast<const GLfloat*>(glm::value_ptr(boneTransforms[x])))

namespace 3d_model_viewer {

const std::string shadersDirectory = std::string(rootDirectory) + "shaders/";
unsigned vboCount = 0;

PolygonMesh::PolygonMesh(const std::string& path, unsigned long id) : Object(Utilities::GetFilenameFromPath(path), id),
                                                                      path(path),
                                                                      animationStartTime(Utilities::GetCurrentTime()) {
    auto extension = Utilities::GetExtensionFromPath(path);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    if (extension != ".fbx" && extension != ".dae" && extension != ".obj" && extension != ".3ds" &&
        extension != ".blend" && extension != ".md5mesh" && extension != ".md5anim") {
        throw std::string("Invalid model format. Accepted file types:"
                          "\n- Autodesk (.fbx)"
                          "\n- Collada (.dae)"
                          "\n- Wavefront Object(.obj)"
                          "\n- 3ds Max 3DS (.3ds)"
                          "\n- Blender 3D (.blend)"
                          "\n- Doom 3 (.md5mesh and .md5anim)");
    }
}

void PolygonMesh::Initialize() {
    mesh.loadRigMesh(path);

    hasTextures = mesh.m_Textures.empty() ? 0 : 1;
    isAnimated = !mesh.m_BoneInfo.empty() && mesh.m_pScene->HasAnimations();
    vAnimationEnabled = animationEnabled ? 1 : 0;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    const auto vertexShader = shadersDirectory + "Object.vert";
    const auto fragmentShader = shadersDirectory + "Object.frag";
    program = LoadShaders(vertexShader.c_str(), fragmentShader.c_str());

    glUseProgram(program);
    glGenBuffers(sizeof(vbo) / sizeof(GLuint), vbo);

    SETUP_VBO(GL_ARRAY_BUFFER, Positions);
    GET_AND_ENABLE_ATTRIBUTE(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    SETUP_VBO(GL_ELEMENT_ARRAY_BUFFER, Indices);

    SETUP_VBO(GL_ARRAY_BUFFER, Normals);
    GET_AND_ENABLE_ATTRIBUTE(vNormal);
    glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    if (hasTextures) {
        SETUP_VBO(GL_ARRAY_BUFFER, TexCoords);
        GET_AND_ENABLE_ATTRIBUTE(vTexCoord);
        glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    }

    if (isAnimated) {
        const auto sizeOfVertexBoneData = sizeof(mesh.Bones[0]);
        SETUP_VBO(GL_ARRAY_BUFFER, Bones);

        GET_AND_ENABLE_ATTRIBUTE(boneIDs);
        glVertexAttribIPointer(boneIDs, 4, GL_UNSIGNED_BYTE, sizeOfVertexBoneData, BUFFER_OFFSET(0));

        GET_AND_ENABLE_ATTRIBUTE(weights);
        glVertexAttribPointer(weights, 4, GL_FLOAT, GL_FALSE, sizeOfVertexBoneData, BUFFER_OFFSET(16));
    }

    RESET_VBO_COUNT();

    SetupUniforms();

    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);

    boundingBox.Initialize();
}

void PolygonMesh::Render() {
    if (isAnimated) {
        vAnimationEnabled = animationEnabled ? 1 : 0;

        if (animationEnabled) {
            std::vector<glm::mat4> boneTransforms;
            const auto runningTime = GetRunningTime();

            mesh.boneTransform(runningTime, boneTransforms);
            assert(boneTransforms.size() <= maxBones);

            for (unsigned i = 0; i < boneTransforms.size(); ++i) {
                UPLOAD_UNIFORM_BONE_TRANSFORM(i);
            }
        }
    } else {
        vAnimationEnabled = 0;
    }

    glUniform1ui(hasTexturesUniform, hasTextures);
    glUniform1ui(vAnimationEnabledUniform, vAnimationEnabled);

    mesh.render();

    if (showBoundingBox) {
        if (boundingBoxColorChanged) {
            glUseProgram(boundingBox.program);
            glUniform4fv(boundingBox.boundingBoxColorUniform, 1, IMVEC4_POINTER(boundingBoxColor));
            boundingBoxColorChanged = false;
        }

        boundingBox.Render(*this);
    }
}

void PolygonMesh::CleanUp() {
    glDeleteBuffers(sizeof(vbo) / sizeof(GLuint), vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
}

void PolygonMesh::SetupUniforms() {
    Object::SetupUniforms();

    SETUP_UNIFORM(hasTextures);
    SETUP_UNIFORM(vAnimationEnabled);

    if (isAnimated) {
        for (unsigned i = 0; i < maxBones - 1; ++i) {
            SETUP_UNIFORM_ARRAY(boneUniforms, i, ("bones[" + std::to_string(i) + "]").c_str());
        }
    }
}

float PolygonMesh::GetRunningTime() {
    return Utilities::DurationToFloat(Utilities::GetCurrentTime() - animationStartTime);
}

void PolygonMesh::BoundingBox::Initialize() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    const std::string vertexShader = shadersDirectory + "BoundingBox.vert";
    const std::string fragmentShader = shadersDirectory + "BoundingBox.frag";
    program = LoadShaders(vertexShader.c_str(), fragmentShader.c_str());

    glUseProgram(program);
    glGenBuffers(2, vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    SetupUniforms();

    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);
}

void PolygonMesh::BoundingBox::Render(PolygonMesh& polygonMesh) {
    const auto& meshVertices = polygonMesh.mesh.Positions;
    min = meshVertices[0];
    max = meshVertices[0];

    const auto sizeOfMeshVertices = meshVertices.size();
    for (unsigned int i = 1; i < sizeOfMeshVertices; ++i) {
        const auto meshVertex = meshVertices[i];

        min.x = std::min(meshVertex.x, min.x);
        max.x = std::max(meshVertex.x, max.x);
        min.y = std::min(meshVertex.y, min.y);
        max.y = std::max(meshVertex.y, max.y);
        min.z = std::min(meshVertex.z, min.z);
        max.z = std::max(meshVertex.z, max.z);
    }

    size = glm::vec3(max.x - min.x, max.y - min.y, max.z - min.z);
    center = glm::vec3((min.x + max.x) / 2, (min.y + max.y) / 2, (min.z + max.z) / 2);
    boundingBoxTransform = glm::translate(glm::mat4(1.0), center) * glm::scale(glm::mat4(1.0), size);

    glUseProgram(program);
    glBindVertexArray(vao);

    glDisable(GL_CULL_FACE);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    UPLOAD_MESH_UNIFORM_MATRIX_4FV(modelMatrix);
    UPLOAD_MESH_UNIFORM_MATRIX_4FV(viewMatrix);
    UPLOAD_MESH_UNIFORM_MATRIX_4FV(projectionMatrix);
    UPLOAD_UNIFORM_MATRIX_4FV(boundingBoxTransform);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    GET_AND_ENABLE_ATTRIBUTE(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, nullptr);
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, reinterpret_cast<GLvoid*>(4 * sizeof(GLushort)));
    glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, reinterpret_cast<GLvoid*>(8 * sizeof(GLushort)));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glPopAttrib();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void PolygonMesh::BoundingBox::SetupUniforms() {
    SETUP_UNIFORM(modelMatrix);
    SETUP_UNIFORM(viewMatrix);
    SETUP_UNIFORM(projectionMatrix);
    SETUP_UNIFORM(boundingBoxTransform);
    SETUP_UNIFORM(boundingBoxColor);
    SETUP_UNIFORM(vAnimationEnabled);
}

} // namespace 3d_model_viewer

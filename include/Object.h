// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2017 Vangelis Tsiatsianas

#pragma once

#include <string>
#include <memory>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <ImGUI/imgui.h>

#define UI_COMPONENT_NAME(x) ((x "##[" + name + " " + id + "]##").c_str())

namespace 3d_model_viewer {

class Object {
public:
    explicit Object(const std::string& name, unsigned long id);

    virtual void Initialize() = 0;
    virtual void Display();
    virtual void DisplayControls();
    virtual void ProcessEvent(SDL_Event* event);

    void Select();
    void Deselect();

    std::string name;
    std::string id;
    std::string formattedName;
    std::unique_ptr<const char[]> formattedNameCString;

    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

protected:
    virtual void Render() = 0;
    virtual void CleanUp() = 0;
    virtual void SetupUniforms();
    virtual void LoadDefaultValues();

    GLuint program;
    GLuint vao;

    GLuint modelMatrixUniform;
    GLuint viewMatrixUniform;
    GLuint projectionMatrixUniform;
    GLuint lightPositionUniform;
    GLuint lightIntensityUniform;
    GLuint materialShininessUniform;
    GLuint ambientProductUniform;
    GLuint diffuseProductUniform;
    GLuint specularProductUniform;

    glm::mat4 translationMatrix;
    glm::mat4 rotationMatrix;
    glm::mat4 scaleMatrix;

    glm::vec4 translation;
    glm::vec4 rotation;
    glm::vec4 scaling;

    glm::vec3 rotationInDegrees;
    float scaleFactor;

    bool transformed;

    ImVec4 materialAmbientColor;
    ImVec4 materialDiffuseColor;
    ImVec4 materialSpecularColor;
    float materialShininess;

    glm::vec4 ambientProduct;
    glm::vec4 diffuseProduct;
    glm::vec4 specularProduct;

    bool wireframe;
    bool hidden;
    bool showBoundingBox;
    bool animationEnabled;

    ImVec4 boundingBoxColor;
    ImVec4 defaultBoundingBoxColor;
    ImVec4 selectedBoundingBoxColor;

    bool boundingBoxColorChanged;
    bool isSelected;
    bool isAnimated;

private:
    ImColor GetRandomBoundingBoxColor();
};

} // namespace 3d_model_viewer

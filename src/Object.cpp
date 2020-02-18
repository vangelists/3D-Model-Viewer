// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2017 Vangelis Tsiatsianas

#include <random>

#include "Common.h"
#include "Environment.h"
#include "Object.h"

#define GLM_SWIZZLE
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace 3d_model_viewer {

using Env = Environment;

auto& camera = Environment::camera;
auto& light = Environment::light;

Object::Object(const std::string& name, unsigned long id) : name(name), id(std::to_string(id)), isAnimated(false) {
    formattedName = id ? name + " " + this->id : name;
    formattedNameCString = std::make_unique<const char[]>(formattedName.length() + 1);
    std::memcpy(const_cast<char*>(formattedNameCString.get()), formattedName.c_str(), formattedName.length());

    selectedBoundingBoxColor = ImColor(255, 235, 0);
    defaultBoundingBoxColor = GetRandomBoundingBoxColor();

    LoadDefaultValues();
}

void Object::Display() {
    if (hidden) {
        return;
    }

    glUseProgram(program);
    glBindVertexArray(vao);

    glDisable(GL_CULL_FACE);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (transformed) {
        translationMatrix = glm::translate(glm::mat4(1.f), translation.xyz());
        rotationMatrix = glm::mat4(1.f);
        rotationMatrix = glm::rotate(rotationMatrix, rotation.x, glm::vec3(1.f, 0.f, 0.f));
        rotationMatrix = glm::rotate(rotationMatrix, rotation.y, glm::vec3(0.f, 1.f, 0.f));
        rotationMatrix = glm::rotate(rotationMatrix, rotation.z, glm::vec3(0.f, 0.f, 1.f));
        scaleMatrix = glm::scale(glm::mat4(.2f), scaling.xyz());

        modelMatrix = scaleMatrix * rotationMatrix * translationMatrix;
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        transformed = false;
    }

    if (camera.changed) {
        viewMatrix = glm::lookAt(camera.position, camera.center, camera.up);
        glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, glm::value_ptr(viewMatrix));

        projectionMatrix = glm::perspective(camera.fieldOfView, camera.aspectRatio, camera.nearClippingPlane, camera.farClippingPlane);
        glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

        --camera.changed;
    }

    if (light.changed) {
        ambientProduct = glm::vec4(light.ambientColor.x, light.ambientColor.y, light.ambientColor.z, light.ambientColor.w) *
                         glm::vec4(materialAmbientColor.x, materialAmbientColor.y, materialAmbientColor.z, materialAmbientColor.w);
        glUniform4fv(ambientProductUniform, 1, glm::value_ptr(ambientProduct));

        diffuseProduct = glm::vec4(light.diffuseColor.x, light.diffuseColor.y, light.diffuseColor.z, light.diffuseColor.w) *
                         glm::vec4(materialDiffuseColor.x, materialDiffuseColor.y, materialDiffuseColor.z, materialDiffuseColor.w);
        glUniform4fv(diffuseProductUniform, 1, glm::value_ptr(diffuseProduct));

        specularProduct = glm::vec4(light.specularColor.x, light.specularColor.y, light.specularColor.z, light.specularColor.w) *
                          glm::vec4(materialSpecularColor.x, materialSpecularColor.y, materialSpecularColor.z, materialSpecularColor.w);
        glUniform4fv(specularProductUniform, 1, glm::value_ptr(specularProduct));

        glUniform4fv(lightPositionUniform, 1, glm::value_ptr(light.position));
        glUniform1fv(lightIntensityUniform, 1, &light.intensity);
        glUniform1fv(materialShininessUniform, 1, &materialShininess);

        --light.changed;
    }

    Render();

    glPopAttrib();
    glBindVertexArray(0);
}

void Object::DisplayControls() {
    ImGui::Begin("Options");

    if (ImGui::CollapsingHeader(formattedNameCString.get())) {
        if (ImGui::Button(UI_COMPONENT_NAME("Reset Defaults"))) {
            LoadDefaultValues();
        }
        ImGui::Indent(15);
        if (ImGui::CollapsingHeader(UI_COMPONENT_NAME("General"))) {
            ImGui::Checkbox(UI_COMPONENT_NAME("Hidden"), &hidden);
            ImGui::Checkbox(UI_COMPONENT_NAME("Wireframe"), &wireframe);
            ImGui::Checkbox(UI_COMPONENT_NAME("Show Bounding Box"), &showBoundingBox);
            if (ImGui::ColorEdit3("Bounding Box Color", IMVEC4_POINTER(boundingBoxColor))) {
                boundingBoxColorChanged = true;
            }
            if (isAnimated) {
                ImGui::Checkbox(UI_COMPONENT_NAME("Enable Animation [DOES NOT WORK]"), &animationEnabled);
            }
        }
        if (ImGui::CollapsingHeader(UI_COMPONENT_NAME("Translation"))) {
            if (ImGui::DragFloat3(UI_COMPONENT_NAME("XYZ" "##Translation"), glm::value_ptr(translation), .1f, -1000.f, 1000.f)) {
                transformed = true;
            }
        }
        if (ImGui::CollapsingHeader(UI_COMPONENT_NAME("Rotation"))) {
            if (ImGui::DragFloat3(UI_COMPONENT_NAME("XYZ (°)" "##Rotation"), glm::value_ptr(rotationInDegrees), .1f, -180.f, 180.f)) {
                rotation = glm::vec4(glm::radians(rotationInDegrees.xyz()), 0.f);
                transformed = true;
            }
        }
        if (ImGui::CollapsingHeader(UI_COMPONENT_NAME("Scaling"))) {
            if (ImGui::SliderFloat(UI_COMPONENT_NAME("Scale Factor"), &scaleFactor, 0.001f, 10.f)) {
                scaling.x = scaleFactor / 5.f;
                scaling.y = scaleFactor / 5.f;
                scaling.z = scaleFactor / 5.f;
                transformed = true;
            }
            if (ImGui::DragFloat3(UI_COMPONENT_NAME("XYZ" "##Scaling"), glm::value_ptr(scaling), .001f, .001f, 2.f)) {
                transformed = true;
            }
        }
        if (ImGui::CollapsingHeader(UI_COMPONENT_NAME("Material"))) {
            if (ImGui::ColorEdit3(UI_COMPONENT_NAME("Ambient Color" "##Material"), IMVEC4_POINTER(materialAmbientColor))) {
                ++light.changed;
            }
            if (ImGui::ColorEdit3(UI_COMPONENT_NAME("Diffuse Color" "##Material"), IMVEC4_POINTER(materialDiffuseColor))) {
                ++light.changed;
            }
            if (ImGui::ColorEdit3(UI_COMPONENT_NAME("Specular Color" "##Material"), IMVEC4_POINTER(materialSpecularColor))) {
                ++light.changed;
            }
            if (ImGui::SliderFloat(UI_COMPONENT_NAME("Shininess"), &materialShininess, .001f, 150.f)) {
                ++light.changed;
            }
        }
        ImGui::Unindent(15);
        ImGui::Spacing();
    }

    ImGui::End();
}

void Object::ProcessEvent(SDL_Event* event) {
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_w:
                wireframe = !wireframe;
                break;
            case SDLK_d:
                if (isSelected) {
                    hidden = !hidden;
                }
                break;
            case SDLK_b:
                showBoundingBox = !showBoundingBox;
                break;
            case SDLK_a:
                animationEnabled = !animationEnabled;
                break;
            case SDLK_r:
                LoadDefaultValues();
                break;
            default:
                break;
        }
    }
}

void Object::Select() {
    isSelected = true;
    boundingBoxColor = selectedBoundingBoxColor;
    boundingBoxColorChanged = true;
}

void Object::Deselect() {
    isSelected = false;
    boundingBoxColor = defaultBoundingBoxColor;
    boundingBoxColorChanged = true;
}

void Object::SetupUniforms() {
    SETUP_UNIFORM(modelMatrix);
    SETUP_UNIFORM(viewMatrix);
    SETUP_UNIFORM(projectionMatrix);
    SETUP_UNIFORM(lightPosition);
    SETUP_UNIFORM(ambientProduct);
    SETUP_UNIFORM(diffuseProduct);
    SETUP_UNIFORM(specularProduct);
    SETUP_UNIFORM(lightIntensity);
    SETUP_UNIFORM(materialShininess);
}

void Object::LoadDefaultValues() {
    translation = glm::vec4(0.f);
    rotationInDegrees = glm::vec3(0.f);
    rotation = glm::vec4(glm::radians(rotationInDegrees.xyz()), 0.f);
    scaleFactor = 1.f;
    scaling = glm::vec4(scaleFactor / 5.f);

    transformed = true;

    materialAmbientColor = ImColor(0, 0, 0, 255);
    materialDiffuseColor = ImColor(255, 255, 255, 255);
    materialSpecularColor = ImColor(255, 255, 255, 255);
    materialShininess = 30.f;

    wireframe = false;
    hidden = false;
    showBoundingBox = true;
    animationEnabled = false;

    boundingBoxColor = defaultBoundingBoxColor;

    boundingBoxColorChanged = true;
    isSelected = false;

    Environment::ForceUpdate();
}

ImColor Object::GetRandomBoundingBoxColor() {
    std::random_device randomDevice;
    std::linear_congruential_engine<std::uint_fast32_t, 48271, 0, 2147483647> generator(randomDevice());
    std::uniform_int_distribution<> distribution(90, 180);
    return {distribution(generator), distribution(generator), distribution(generator), 255};
}

} // namespace 3d_model_viewer

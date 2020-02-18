// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2017 Vangelis Tsiatsianas

#include "Common.h"
#include "Environment.h"
#include "GUI.h"

#define GLM_SWIZZLE
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define IMVEC4_POINTER(x) (reinterpret_cast<float *>(&(x)))

namespace 3d_model_viewer {

unsigned long Environment::numObjects = 0;

Environment::Camera Environment::camera;
Environment::Light Environment::light;

void Environment::Initialize() {
    LoadDefaultValues();
}

void Environment::DisplayControls() {
    ImGui::Begin("Options");

    if (ImGui::CollapsingHeader("Camera")) {
        if (ImGui::Button("Reset Defaults")) {
            camera.LoadDefaultValues();
        }
        ImGui::SameLine();
        if (ImGui::Button("Focus on Origin")) {
            camera.LoadDefaultValues();
            camera.focusOnOrigin = true;
        }
        ImGui::Indent(15);
        ImGui::DragFloat3("XYZ" "##Camera", glm::value_ptr(camera.position), .01f, -10.f, 10.f);
        ImGui::DragFloat3("Direction" "##Camera", glm::value_ptr(camera.center), .01f, -10.f, 10.f);
        ImGui::DragFloat3("Up" "##Camera", glm::value_ptr(camera.up), .01f, -10.f, 10.f);
        (ImGui::SliderFloat("Move Speed", &camera.speedFactor, 1.f, 10.f));
        ImGui::SliderFloat("Zoom Speed", &camera.zoomSpeedFactor, 1.f, 10.f);
        if (ImGui::SliderFloat("Horizontal Angle (°)", &camera.thetaInDegrees, -179.f, 179.f)) {
            camera.UpdateAngleInformation();
        }
        if (ImGui::SliderFloat("Vertical Angle (°)", &camera.phiInDegrees, 15.f, 165.f)) {
            camera.UpdateAngleInformation();
        }
        if (ImGui::SliderFloat("FOV (°)", &camera.fieldOfViewDegrees, 1.f, 179.f)) {
            camera.fieldOfView = glm::radians(camera.fieldOfViewDegrees);
        }
        ImGui::SliderFloat("Aspect Ratio", &camera.aspectRatio, 1.f, 1.999f);
        if (ImGui::SliderFloat("Near Clipping", &camera.nearClippingPlane, 0.1f, 100.f)) {
            if (camera.nearClippingPlane > camera.farClippingPlane) {
                camera.nearClippingPlane = camera.farClippingPlane;
            }
        }
        if (ImGui::SliderFloat("Far Clipping", &camera.farClippingPlane, 0.1f, 10000.f)) {
            if (camera.farClippingPlane < camera.nearClippingPlane) {
                camera.farClippingPlane = camera.nearClippingPlane;
            }
        }
        ImGui::Unindent(15);
        ImGui::Spacing();
    }
    if (ImGui::CollapsingHeader("Lighting")) {
        if (ImGui::Button("Reset Defaults")) {
            light.LoadDefaultValues();
        }
        ImGui::Indent(15);
        if (ImGui::DragFloat3("XYZ" "##Light", glm::value_ptr(light.position), .1f, -100.f, 100.f)) {
            light.changed = numObjects;
        }
        if (ImGui::ColorEdit3("Ambient Color" "##Light", IMVEC4_POINTER(light.ambientColor))) {
            light.changed = numObjects;
        }
        if (ImGui::ColorEdit3("Diffuse Color" "##Light", IMVEC4_POINTER(light.diffuseColor))) {
            light.changed = numObjects;
        }
        if (ImGui::ColorEdit3("Specular Color" "##Light", IMVEC4_POINTER(light.specularColor))) {
            light.changed = numObjects;
        }
        if (ImGui::SliderFloat("Intensity", &light.intensity, 0.f, 150.f)) {
            light.changed = numObjects;
        }
        ImGui::Unindent(15);
        ImGui::Spacing();
    }

    ImGui::End();
}

void Environment::ProcessEvent(SDL_Event* event) {
    camera.ProcessEvent(event);

    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_r:
                LoadDefaultValues();
                break;
            default:
                break;
        }
    }
}

void Environment::LoadDefaultValues() {
    camera.LoadDefaultValues();
    light.LoadDefaultValues();
}

void Environment::ForceUpdate() {
    camera.changed = numObjects;
    light.changed = numObjects;
}

Environment::Camera::Camera() : position(glm::vec3(1.f)) {}

void Environment::Camera::Update(Time& currentTime) {
    currentFrame = currentTime;
    deltaTime = Utilities::DurationToFloat(currentFrame - lastFrame);
    lastFrame = currentFrame;
    speed = 20 * speedFactor * deltaTime;
    zoomSpeed = zoomSpeedFactor * speed;

    if (focusOnOrigin) {
        center = origin;
    } else {
        const auto selectedModel = GUI::GetSelectedModel();
        if (selectedModel) {
            const auto& transform = selectedModel->modelMatrix;
            const auto& boundingBox = selectedModel->boundingBox;
            const auto target = transform * glm::vec4(boundingBox.center, 1.f);
            center = (target / target.w).xyz();

            const auto boundingBoxSize = transform * vec4(boundingBox.size, 1.f);
            const auto boundingBoxSizeNormalized = (boundingBoxSize / boundingBoxSize.w).xyz();
            const auto maxBoundingBoxSide = std::max({boundingBoxSizeNormalized.x, boundingBoxSizeNormalized.y, boundingBoxSizeNormalized.z});
            distance = (3.f * maxBoundingBoxSide);
        } else {
            center = origin;
            distance = Utilities::DistanceBetween3DPoints(position, origin);
        }
    }

    position.x = distance * sinPhi * cosTheta;
    position.y = distance * cosPhi;
    position.z = distance * sinPhi * sinTheta;

    changed = numObjects;
}

void Environment::Camera::ProcessEvent(SDL_Event* event) {
    switch (event->type) {
        case SDL_KEYDOWN:
            ProcessKeyboardEvent(event);
            break;
        case SDL_MOUSEWHEEL:
            ProcessMouseWheelEvent(event);
            break;
        default:
            break;
    }
}

void Environment::Camera::ProcessKeyboardEvent(SDL_Event* event) {
    switch (event->key.keysym.sym) {
        case SDLK_UP:
            phiInDegrees -= speed;
            if (phiInDegrees < 15.f) {
                phiInDegrees = 15.f;
            }
            UpdateAngleInformation();
            break;
        case SDLK_DOWN:
            phiInDegrees += speed;
            if (phiInDegrees > 165.f) {
                phiInDegrees = 165.f;
            }
            UpdateAngleInformation();
            break;
        case SDLK_LEFT:
            thetaInDegrees += speed;
            if (thetaInDegrees > 179.f) {
                thetaInDegrees = -179.f;
            }
            UpdateAngleInformation();
            break;
        case SDLK_RIGHT:
            thetaInDegrees -= speed;
            if (thetaInDegrees < -179.f) {
                thetaInDegrees = 179.f;
            }
            UpdateAngleInformation();
            break;
        case SDLK_l:
            fieldOfViewDegrees += zoomSpeed;
            if (fieldOfViewDegrees > 179.f) {
                fieldOfViewDegrees = 179.f;
            }
            fieldOfView = glm::radians(fieldOfViewDegrees);
            break;
        case SDLK_k:
            fieldOfViewDegrees -= zoomSpeed;
            if (fieldOfViewDegrees < 1.f) {
                fieldOfViewDegrees = 1.f;
            }
            fieldOfView = glm::radians(fieldOfViewDegrees);
            break;
        case SDLK_f:
            focusOnOrigin = true;
            break;
        case SDLK_r:
            LoadDefaultValues();
            break;
        default:
            break;
    }
}

void Environment::Camera::ProcessMouseWheelEvent(SDL_Event* event) {
    if (event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
        zoomSpeed *= -1.f;
    }

    switch (event->wheel.y) {
        case 1:
            fieldOfViewDegrees += zoomSpeed;
            changed = numObjects;
            break;
        case -1:
            fieldOfViewDegrees -= zoomSpeed;
            changed = numObjects;
            break;
        default:
            break;
    }

    if (changed) {
        if (fieldOfViewDegrees < 1.f) {
            fieldOfViewDegrees = 1.f;
        } else if (fieldOfViewDegrees > 179.f) {
            fieldOfViewDegrees = 179.f;
        }
        fieldOfView = glm::radians(fieldOfViewDegrees);
    }
}

void Environment::Camera::UpdateAngleInformation() {
    phi = glm::radians(phiInDegrees);
    theta = glm::radians(thetaInDegrees);

    sinPhi = sin(phi);
    cosPhi = cos(phi);
    sinTheta = sin(theta);
    cosTheta = cos(theta);
}

void Environment::Camera::LoadDefaultValues() {
    up = glm::vec3(0.f, 1.f, 0.f);

    thetaInDegrees = 45.f;
    phiInDegrees = 65.f;

    UpdateAngleInformation();

    speedFactor = 5.f;
    zoomSpeedFactor = 2.f;

    fieldOfViewDegrees = 45.f;
    fieldOfView = glm::radians(fieldOfViewDegrees);
    aspectRatio = windowWidth / windowHeight;
    nearClippingPlane = 0.1f;
    farClippingPlane = 10000.f;

    origin = glm::vec3(0.f);
    focusOnOrigin = false;

    changed = numObjects;
}

void Environment::Light::LoadDefaultValues() {
    position = glm::vec4(10.f, 10.f, 2.f, 1.f);
    ambientColor = ImColor(0, 0, 0, 255);
    diffuseColor = ImColor(255, 255, 255, 255);
    specularColor = ImColor(255, 255, 255, 255);
    intensity = 50.f;

    changed = numObjects;
}

} // namespace 3d_model_viewer

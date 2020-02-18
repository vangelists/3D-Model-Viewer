// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2017 Vangelis Tsiatsianas

#pragma once

#include <string>

#include "Utilities.h"

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <ImGUI/imgui.h>

namespace 3d_model_viewer {

struct Environment {
    static void Initialize();
    static void DisplayControls();
    static void ProcessEvent(SDL_Event* event);

    static void LoadDefaultValues();
    static void ForceUpdate();

    struct Camera {
    public:
        Camera();

        void Update(Time& currentTime);
        void ProcessEvent(SDL_Event* event);

        void LoadDefaultValues();
        void UpdateAngleInformation();

        glm::vec3 position;
        glm::vec3 center;
        glm::vec3 up;

        float thetaInDegrees;
        float theta;
        float phiInDegrees;
        float phi;
        float distance;

        float sinPhi;
        float cosPhi;
        float sinTheta;
        float cosTheta;

        float speed;
        float speedFactor;

        float zoomSpeed;
        float zoomSpeedFactor;

        Time currentFrame;
        Time lastFrame;
        float deltaTime;

        float fieldOfView;
        float fieldOfViewDegrees;
        float aspectRatio;
        float nearClippingPlane;
        float farClippingPlane;

        glm::vec3 origin;
        bool focusOnOrigin;

        unsigned long changed;

    private:
        void ProcessKeyboardEvent(SDL_Event* event);
        void ProcessMouseWheelEvent(SDL_Event* event);
    };

    struct Light {
        void LoadDefaultValues();

        glm::vec4 position;
        ImVec4 ambientColor;
        ImVec4 diffuseColor;
        ImVec4 specularColor;
        float intensity;

        unsigned long changed;
    };

    static Camera camera;
    static Light light;

    static unsigned long numObjects;
};

} // namespace 3d_model_viewer

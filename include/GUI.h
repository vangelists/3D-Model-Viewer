// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2017 Vangelis Tsiatsianas

#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>
#include <cstdint>

#include "PolygonMesh.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <ImGUI/imgui.h>
#include <tiny-file-dialogs/tinyfiledialogs.h>

namespace 3d_model_viewer {

using DisplayFunction = std::function<void(void)>;
using EventHandler = std::function<void(SDL_Event* event)>;

class GUI final {
public:
    static bool Initialize();
    static void ProcessEvent(SDL_Event* event);
    static void Display();
    static void DisplayErrorMessage(const std::string& errorMessage);
    static void Close();
    static PolygonMesh* GetSelectedModel();

    class Audio {
    public:
        static bool Initialize();
        static void CleanUp();
        static bool LoadAudioFile(const std::string& audioFilePath);

        static bool IsPlaying();
        static bool IsPaused();
        static int Play();
        static void Pause();
        static void Resume();
        static int Stop();

    private:
        static Mix_Music* audioFile;
    };

private:
    static void LoadModel();
    static void UnloadSelectedModel();

    static void InstallDisplayFunction(DisplayFunction displayFunction);
    static void InstallInternalDisplayFunction(DisplayFunction displayFunction);

    static void DisplayWindowControls();
    static void DisplayAudioControls();
    static void DisplayModelControls();
    static void DisplayMetrics();
    static void DisplayHelp();

    static void LoadDefaultValues();
    static void FocusOnOrigin();

    static bool showHelp;
    static bool showMetrics;

    static ImVec4 backgroundColor;

    static SDL_Window* window;
    static SDL_GLContext glContext;

    static int selectedModelIndex;
    static std::vector<PolygonMesh> loadedModels;
    static std::vector<DisplayFunction> displayFunctions;
    static std::vector<DisplayFunction> internalDisplayFunctions;
};

} // namespace 3d_model_viewer

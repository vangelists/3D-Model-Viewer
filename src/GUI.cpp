// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2017 Vangelis Tsiatsianas

#include <iostream>

#include "Common.h"
#include "Environment.h"
#include "GUI.h"
#include "Utilities.h"

#include <fonts/IconsFontAwesome.h>
#include <GL/glew.h>
#include <ImGUI/imgui.h>
#include <ImGUI/imgui_impl_sdl_glew.h>

namespace 3d_model_viewer {

std::string fontsDirectory = std::string(rootDirectory) + "/res/fonts/";
std::string playPauseButtonLabel = ICON_FA_PLAY;
std::vector<const char *> acceptedFileTypes = { "*.fbx", "*.dae", "*.obj", "*.3ds", "*.blend", "*.md5mesh", "*.md5anim" };
constexpr auto loadedModelsListHeightInItems = 6;

ImFont* fontAwesome = nullptr;

bool GUI::showHelp = true;
bool GUI::showMetrics = false;

ImVec4 GUI::backgroundColor;

SDL_Window* GUI::window = nullptr;
SDL_GLContext GUI::glContext = nullptr;
Mix_Music* GUI::Audio::audioFile = nullptr;

int GUI::selectedModelIndex;
auto GUI::loadedModels = std::vector<PolygonMesh>();
auto GUI::displayFunctions = std::vector<DisplayFunction>();
auto GUI::internalDisplayFunctions = std::vector<DisplayFunction>();

bool GUI::Initialize() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0) {
        DisplayErrorMessage("Could not initialize SDL! SDL error: " + std::string(SDL_GetError()));
        return false;
    }

    if (!Audio::Initialize()) {
        DisplayErrorMessage("Count not initialize audio! SDL error: " + std::string(SDL_GetError()));
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_DisplayMode current = SDL_DisplayMode();
    SDL_GetCurrentDisplayMode(0, &current);
    window = SDL_CreateWindow("3D Model Viewer (vangelists)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              static_cast<int>(windowWidth), static_cast<int>(windowHeight), SDL_WINDOW_OPENGL);
    if (window == nullptr) {
        DisplayErrorMessage("Could not create window! SDL Error: " + std::string(SDL_GetError()));
        return false;
    }

    glContext = SDL_GL_CreateContext(window);
    if (glContext == nullptr) {
        DisplayErrorMessage("Could not create OpenGL context! SDL error: " + std::string(SDL_GetError()));
        return false;
    }

    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        auto glewErrorMessage = reinterpret_cast<const char*>(glewGetErrorString(glewError));
        DisplayErrorMessage("Could not initialize GLEW! GLEW error: " + std::string(glewErrorMessage));
    }

    if (SDL_GL_SetSwapInterval(1) < 0) {
        DisplayErrorMessage("Warning: Unable to set Vsync! SDL error: " + std::string(SDL_GetError()));
    }

    if (!ImGui_Impl_Init(window)) {
        DisplayErrorMessage("Error initializing ImGui!");
        return false;
    }

    LoadDefaultValues();

    const auto robotoMediumLocation = fontsDirectory + "RobotoMedium.ttf";
    const auto fontAwesomeLocation = fontsDirectory + "FontAwesome.ttf";
    static const ImWchar fontAwesomeIconRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

    ImFontConfig config;
    config.MergeMode = true;
    config.OversampleH = 5;
    config.OversampleV = 5;
    config.GlyphExtraSpacing.x = 1.0f;

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->ClearFonts();
    io.Fonts->AddFontFromFileTTF(robotoMediumLocation.c_str(), 15.0f, &config);
    fontAwesome = io.Fonts->AddFontFromFileTTF(fontAwesomeLocation.c_str(), 16.0f, &config, fontAwesomeIconRanges);
    io.Fonts->Build();

    InstallInternalDisplayFunction(DisplayHelp);
    InstallInternalDisplayFunction(DisplayMetrics);
    InstallInternalDisplayFunction(DisplayWindowControls);
    InstallInternalDisplayFunction(DisplayModelControls);

    Environment::Initialize();
    InstallDisplayFunction(Environment::DisplayControls);

    return true;
}

void GUI::ProcessEvent(SDL_Event* event) {
    ImGui_Impl_ProcessEvent(event);

    if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_ESCAPE || event->type == SDL_QUIT) {
            GUI::Close();
            std::exit(EXIT_SUCCESS);
        }

        switch (event->key.keysym.sym) {
            case SDLK_h:
                showHelp = !showHelp;
                break;
            case SDLK_m:
                showMetrics = !showMetrics;
                break;
            case SDLK_o:
                LoadModel();
                break;
            case SDLK_BACKSPACE:
            case SDLK_DELETE:
                UnloadSelectedModel();
                break;
            case SDLK_f:
                FocusOnOrigin();
                break;
            case SDLK_r:
                LoadDefaultValues();
                break;
            default:
                break;
        }
    }

    Environment::ProcessEvent(event);

    for (auto& model : loadedModels) {
        model.ProcessEvent(event);
    }
}

void GUI::LoadModel() {
    auto path = tinyfd_openFileDialog("", (rootDirectory + std::string("/res/models/")).c_str(),
                                      acceptedFileTypes.size(), acceptedFileTypes.data(), nullptr, 0);
    if (!path) {
        return;
    }

    unsigned long id = 0;
    for (auto model = loadedModels.rbegin(); model != loadedModels.rend(); ++model) {
        if (model->name == Utilities::GetFilenameFromPath(path)) {
            id = std::stoul(model->id) + 1;
            break;
        }
    }

    try {
        Environment::numObjects = loadedModels.size() + 1;
        loadedModels.emplace_back(PolygonMesh(path, id));
    } catch (const std::string& errorMessage) {
        GUI::DisplayErrorMessage(errorMessage);
        Environment::numObjects = loadedModels.size();
        return;
    }

    auto* polygonMesh = &loadedModels.back();
    polygonMesh->Initialize();

    if (selectedModelIndex > -1) {
        GetSelectedModel()->Deselect();
    }
    selectedModelIndex = static_cast<int>(loadedModels.size()) - 1;
    GetSelectedModel()->Select();

    Environment::camera.focusOnOrigin = false;
}

void GUI::UnloadSelectedModel() {
    if (!loadedModels.empty() && selectedModelIndex < loadedModels.size()) {
        auto selectedModel = GetSelectedModel();
        selectedModel->Deselect();
        selectedModel->CleanUp();
        loadedModels.erase(loadedModels.begin() + selectedModelIndex);

        --selectedModelIndex;
        auto newSelectedModel = GetSelectedModel();
        if (newSelectedModel) {
            newSelectedModel->Select();
        }

        Environment::numObjects = loadedModels.size();
        if (!Environment::numObjects) {
            Environment::camera.focusOnOrigin = true;
        }
    }
}

void GUI::InstallDisplayFunction(DisplayFunction displayFunction) {
    displayFunctions.push_back(displayFunction);
}

void GUI::InstallInternalDisplayFunction(DisplayFunction displayFunction) {
    internalDisplayFunctions.push_back(displayFunction);
}

void GUI::DisplayWindowControls() {
    ImGui::SetNextWindowSize(ImVec2(400, windowHeight - 220), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10, 10));

    ImGui::Begin("Options");

    if (ImGui::CollapsingHeader("Window")) {
        if (ImGui::Button("Reset Defaults")) {
            LoadDefaultValues();
        }
        ImGui::Indent(15);
        ImGui::ColorEdit3("Background Color", IMVEC4_POINTER(backgroundColor));
        ImGui::Unindent(15);
        ImGui::Spacing();
    }

    ImGui::End();
}

void GUI::DisplayAudioControls() {
    ImGui::Begin("Options");

    if (ImGui::CollapsingHeader("Audio")) {
        ImGui::PushFont(fontAwesome);

        if (ImGui::Button(playPauseButtonLabel.c_str())) {
            if (!Audio::IsPlaying()) {
                Audio::Play();
                playPauseButtonLabel = ICON_FA_PAUSE;
            } else {
                if (Audio::IsPaused()) {
                    Audio::Resume();
                    playPauseButtonLabel = ICON_FA_PAUSE;
                } else {
                    Audio::Pause();
                    playPauseButtonLabel = ICON_FA_PLAY;
                }
            }
        }

        ImGui::SameLine(0.0f, 10.0f);
        const std::string stopIcon = ICON_FA_STOP;
        if (ImGui::Button(stopIcon.c_str())) {
            Audio::Stop();
            playPauseButtonLabel = ICON_FA_PLAY;
        }

        ImGui::Spacing();
        ImGui::PopFont();
    }

    ImGui::End();
}

void GUI::DisplayModelControls() {
    ImGui::SetNextWindowSize(ImVec2(400, 190), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(10, windowHeight - 200));

    ImGui::Begin("Loaded Models");

    if (ImGui::Button("Add Model")) {
        LoadModel();
    }
    ImGui::SameLine(0.0f, 10.0f);
    if (ImGui::Button("Remove Selected Model")) {
        UnloadSelectedModel();

        if (selectedModelIndex >= loadedModels.size()) {
            selectedModelIndex = static_cast<unsigned>(loadedModels.size() - 1);
        }
    }
    ImGui::Spacing();

    auto numLoadedModels = static_cast<int>(loadedModels.size());
    if (ImGui::ListBoxHeader("", numLoadedModels, loadedModelsListHeightInItems)) {
        for (unsigned i = 0; i < numLoadedModels; ++i) {
            const auto isSelectedModel = (i == selectedModelIndex);
            const auto& loadedModel = loadedModels[i];
            const char* itemText = loadedModel.id != "0" ? (loadedModel.name + " " + loadedModel.id).c_str()
                                                         : (loadedModel.name).c_str();

            ImGui::PushID(i);
            if (ImGui::Selectable(itemText, isSelectedModel)) {
                auto oldSelectedModel = GetSelectedModel();
                if (oldSelectedModel) {
                    oldSelectedModel->Deselect();
                }

                selectedModelIndex = i;
                auto selectedModel = GetSelectedModel();
                assert(selectedModel);
                selectedModel->Select();

                if (Environment::camera.focusOnOrigin) {
                    Environment::camera.focusOnOrigin = false;
                }
            }
            ImGui::PopID();
        }
        ImGui::ListBoxFooter();
    }

    ImGui::End();
}

void GUI::DisplayMetrics() {
    if (showMetrics) {
        ImGui::SetNextWindowPos(ImVec2(windowWidth - 415, windowHeight - 224), ImGuiSetCond_FirstUseEver);
        ImGui::ShowMetricsWindow();
    }
}

void GUI::DisplayHelp() {
    if (showHelp) {
        ImGui::SetNextWindowSize(ImVec2(260, 294), ImGuiSetCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(windowWidth - 270, 10), ImGuiSetCond_FirstUseEver);

        ImGui::Begin("Help");

        ImGui::Text("H: Show/Hide Help"
                    "\nM: Show/Hide Metrics"
                    "\nR: Reset Defaults"
                    "\n\nO: Add Model"
                    "\nBackspace/Delete: Remove Model"
                    "\n\nW: Toggle Wireframe"
                    "\nB: Toggle Bounding Boxes"
                    "\nD: Toggle Selected Model Display"
                    "\nA: Toggle Animation [NOT WORKING]"
                    "\n\nK/Scroll Up: Zoom In"
                    "\nL/Scroll Down: Zoom Out"
                    "\nF: Focus on Origin"
                    "\n\nUp/Down/Left/Right: Move Camera");

        ImGui::End();
    }
}

void GUI::LoadDefaultValues() {
    backgroundColor = ImColor(64, 64, 64, 255);
    selectedModelIndex = -1;
}

void GUI::FocusOnOrigin() {
    auto selectedModel = GetSelectedModel();
    if (selectedModel) {
        selectedModel->Deselect();
    }
    Environment::camera.focusOnOrigin = true;
}

void GUI::DisplayErrorMessage(const std::string& errorMessage) {
    tinyfd_messageBox("Error", errorMessage.c_str(), "ok", "error", 1);
    std::cerr << errorMessage << std::endl;
}

void GUI::Display() {
    ImGui_Impl_NewFrame(window);

    glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui::Begin("Options");

    if (!internalDisplayFunctions.empty()) {
        ImGui::Text("General");
        ImGui::Spacing();

        for (const auto &internalDisplayFunction : internalDisplayFunctions) {
            internalDisplayFunction();
        }
    }

    if (!displayFunctions.empty()) {
        ImGui::Text("Environment");
        ImGui::Spacing();

        for (const auto &displayFunction : displayFunctions) {
            displayFunction();
        }
    }

    if (!loadedModels.empty()) {
        ImGui::Text("Loaded Models");
        ImGui::Spacing();

        for (auto &model : loadedModels) {
            model.DisplayControls();
            model.Display();
        }
    }

    ImGui::End();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    ImGui::Render();
    SDL_GL_SwapWindow(window);
}

void GUI::Close() {
    Audio::CleanUp();
    ImGui_Impl_Shutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    Mix_Quit();
    SDL_Quit();
}

PolygonMesh* GUI::GetSelectedModel() {
    return selectedModelIndex != -1 ? &loadedModels[selectedModelIndex] : nullptr;
}

bool GUI::Audio::Initialize() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        DisplayErrorMessage("Could not initialize SDL_mixer! SDL_mixer error: " + std::string(Mix_GetError()));
        return false;
    }

    return true;
}

bool GUI::Audio::LoadAudioFile(const std::string& audioFilePath) {
    audioFile = Mix_LoadMUS(audioFilePath.c_str());
    if (audioFile == nullptr) {
        DisplayErrorMessage("Failed to load audio file! SDL_mixer error: " + std::string(Mix_GetError()));
        return false;
    }

    InstallInternalDisplayFunction(DisplayAudioControls);
    return true;
}

bool GUI::Audio::IsPlaying() {
    return Mix_PlayingMusic() != 0;
}

bool GUI::Audio::IsPaused() {
    return Mix_PausedMusic() == 1;
}

int GUI::Audio::Play() {
    return Mix_PlayMusic(audioFile, -1);
}

void GUI::Audio::Pause() {
    Mix_PauseMusic();
}

void GUI::Audio::Resume() {
    Mix_ResumeMusic();
}

int GUI::Audio::Stop() {
    return Mix_HaltMusic();
}

void GUI::Audio::CleanUp() {
    Mix_FreeMusic(audioFile);
    audioFile = nullptr;
    displayFunctions.clear();
}

} // namespace 3d_model_viewer

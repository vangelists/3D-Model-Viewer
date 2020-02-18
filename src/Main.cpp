// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2017 Vangelis Tsiatsianas

#include <functional>
#include <vector>

#include "Common.h"
#include "Environment.h"
#include "GUI.h"
#include "Utilities.h"

using namespace 3d_model_viewer;

const auto mediaDirectory = std::string(rootDirectory) + "res/media/";
constexpr auto maxLoopTime = 16ms;

auto& camera = Environment::camera;

int main() {
    if (!GUI::Initialize()) {
        GUI::DisplayErrorMessage("Initialization failed!");
        std::exit(EXIT_FAILURE);
    }

    const auto audioFilePath = mediaDirectory + audioFileName;
    GUI::Audio::LoadAudioFile(audioFilePath);

    auto event = SDL_Event();

    while (true) {
        auto loopStart = Utilities::GetCurrentTime();
        camera.Update(loopStart);

        while (SDL_PollEvent(&event)) {
            GUI::ProcessEvent(&event);
        }

        GUI::Display();

        // Maintain 60 FPS (on average) without keeping the CPU always busy.
        auto loopEnd = Utilities::GetCurrentTime();
        auto loopDuration = loopEnd - loopStart;
        if (loopDuration < maxLoopTime) {
            Utilities::SleepFor(maxLoopTime - loopDuration);
        }
    }
}

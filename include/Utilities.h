// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2017 Vangelis Tsiatsianas

#pragma once

#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include <chrono>
#include <filesystem>
#include <string>
#include <thread>
#include <cassert>

namespace 3d_model_viewer {

using Time = std::chrono::time_point<std::chrono::high_resolution_clock>;

struct Utilities final {
    static const std::string GetFilenameFromPath(const std::string& path) {
        return std::filesystem::path(path).stem().string();
    }

    static const std::string GetExtensionFromPath(const std::string& path) {
        return std::filesystem::path(path).extension().string();
    }

    static const Time GetCurrentTime() {
        return std::chrono::high_resolution_clock::now();
    }

    static float DistanceBetween3DPoints(glm::vec3 point1, glm::vec3 point2) {
        return static_cast<float>(std::sqrt(std::pow(point1.x - point2.x, 2) +
                                            std::pow(point1.y - point2.y, 2) +
                                            std::pow(point1.z - point2.z, 2)));
    }

    static float DurationToFloat(const std::chrono::duration<float>& durationInNanoseconds) {
        return durationInNanoseconds.count();
    }

    static void SleepFor(const std::chrono::duration<float>& durationInNanoseconds) {
        std::this_thread::sleep_for(durationInNanoseconds);
    }
};

} // namespace 3d_model_viewer

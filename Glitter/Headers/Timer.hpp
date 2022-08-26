#pragma once

#include <chrono>

class Timer {
public:
    void Start() {
        startTime = lastFrameTime = now = std::chrono::high_resolution_clock::now();
    }
    void Update() {
        lastFrameTime = now;
        now = std::chrono::high_resolution_clock::now();
    }

    float GetTime() {
        return std::chrono::duration<float>(now - startTime).count();
    }
    float GetDelta() {
        return std::chrono::duration<float>(now - lastFrameTime).count();
    }

private:
    std::chrono::steady_clock::time_point startTime, lastFrameTime, now;

};

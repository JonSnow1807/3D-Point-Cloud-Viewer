#pragma once

#include <chrono>

namespace pcv {

class Timer {
public:
    Timer() : start_time_(std::chrono::high_resolution_clock::now()) {}
    
    void reset() {
        start_time_ = std::chrono::high_resolution_clock::now();
    }
    
    float elapsed() const {
        auto current_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<float, std::milli>(current_time - start_time_).count();
    }
    
    float elapsedSeconds() const {
        return elapsed() / 1000.0f;
    }
    
private:
    std::chrono::high_resolution_clock::time_point start_time_;
};

} // namespace pcv
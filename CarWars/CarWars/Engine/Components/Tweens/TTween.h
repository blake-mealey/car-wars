#pragma once

#include "../../Systems/Time.h"
#include <glm/glm.hpp>
#include "Tween.h"
#include <functional>

template <typename V, float Ease(float t, float b, float c, float d)>
class TTween : public Tween {
friend class Effects;
public:
    void Update() override {
        if (!started || finished) return;
        const Time current = clock - startTime;
        if (current >= duration) {
            value = end;
            Stop(true);
        } else {
            value = glm::mix(start, end, Ease(current.GetSeconds(), 0.f, 1.f, duration.GetSeconds()));
            if (updateCallback) updateCallback(value);
        }
    }

    void SetUpdateCallback(std::function<void(V&)> a_callback) {
        updateCallback = a_callback;
    }

    void SetFinishedCallback(std::function<void(V&)> a_callback) {
        finishedCallback = a_callback;
    }

    void Stop(const bool naturalStop=false) override {
        Tween::Stop(naturalStop);
        if (naturalStop) {
            if (updateCallback) updateCallback(value);
            if (finishedCallback) finishedCallback(value);
        }
    }

protected:
    TTween(V a_start, V a_end, const Time a_duration, Time& clock = StateManager::globalTime) : Tween(a_duration, clock),
        updateCallback(nullptr), finishedCallback(nullptr), initialValue(a_start), value(initialValue), start(a_start), end(a_end) {}

    TTween(V& a_value, V a_start, V a_end, const Time a_duration, Time& clock = StateManager::globalTime) : Tween(a_duration, clock),
        updateCallback(nullptr), finishedCallback(nullptr), value(a_value), start(a_start), end(a_end) {}

    std::function<void(V&)> updateCallback;
    std::function<void(V&)> finishedCallback;

    V initialValue;
    V& value;
    V start;
    V end;
};

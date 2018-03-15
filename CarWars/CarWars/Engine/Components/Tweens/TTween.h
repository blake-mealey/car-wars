#pragma once

#include "../../Systems/Time.h"
#include <glm/glm.hpp>
#include "../../Systems/StateManager.h"
#include "Tween.h"
#include <functional>

template <typename V, float Ease(float t, float b, float c, float d)>
class TTween : public Tween {
friend class Effects;
public:
    void Update() override {
        if (!started || finished) return;
        const Time current = StateManager::globalTime - startTime;
        finished = current >= duration;
        if (finished) {
            value = end;
            if (nextTween) {
                nextTween->Start();
            }
        } else {
            value = glm::mix(start, end, Ease(current.GetSeconds(), 0.f, 1.f, duration.GetSeconds()));
        }

        if (callback) {
            callback(value);
        }
    }

protected:
    TTween(V a_start, V a_end, const Time a_duration, std::function<void(V&)> a_callback = nullptr) : Tween(a_duration),
        callback(a_callback), initialValue(a_start), value(initialValue), start(a_start), end(a_end) {}

    TTween(V& a_value, V a_start, V a_end, const Time a_duration, std::function<void(V&)> a_callback = nullptr) : Tween(a_duration),
        callback(a_callback), value(a_value), start(a_start), end(a_end) {}

    std::function<void(V&)> callback;

    V initialValue;
    V& value;
    V start;
    V end;
};

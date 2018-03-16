#pragma once

#include "System.h"
#include "../Components/Tweens/TTween.h"
#include <vector>
#include <functional>

class GuiEffect;

class Effects : public System {
public:
    // Access the singleton instance
    static Effects& Instance();
    ~Effects();

    void Update() override;

    template <typename V, float Ease(float t, float b, float c, float d)>
    TTween<V, Ease>* CreateTween(V& a_value, V a_start, V a_end, const Time a_duration) {
        auto tween = new TTween<V, Ease>(a_value, a_start, a_end, a_duration);
        tweens.push_back(tween);
        return tween;
    }

    template <typename V, float Ease(float t, float b, float c, float d)>
    TTween<V, Ease>* CreateTween(V a_start, V a_end, const Time a_duration) {
        auto tween = new TTween<V, Ease>(a_start, a_end, a_duration);
        tweens.push_back(tween);
        return tween;
    }

private:
    // No instantiation or copying
    Effects();
    Effects(const Effects&) = delete;
    Effects& operator= (const Effects&) = delete;

    std::vector<Tween*> tweens;
};

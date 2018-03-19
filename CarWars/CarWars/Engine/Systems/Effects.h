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
        AddTween(tween);
        return tween;
    }

    template <typename V, float Ease(float t, float b, float c, float d)>
    TTween<V, Ease>* CreateTween(V a_start, V a_end, const Time a_duration) {
        auto tween = new TTween<V, Ease>(a_start, a_end, a_duration);
        AddTween(tween);
        return tween;
    }

    void DestroyTween(Tween* tween);

    Tween* FindTween(std::string tag);

private:
    // No instantiation or copying
    Effects();
    Effects(const Effects&) = delete;
    Effects& operator= (const Effects&) = delete;

    void AddTween(Tween* tween) {
        if (inUpdate) {
            tweensCreatedInUpdate.push_back(tween);
        } else {
            tweens.push_back(tween);
        }
    }

    bool inUpdate;
    std::vector<Tween*> tweensCreatedInUpdate;
    std::vector<Tween*> tweens;
};

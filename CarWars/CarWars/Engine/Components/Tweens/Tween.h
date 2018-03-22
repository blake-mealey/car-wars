#pragma once

#include "../../Systems/StateManager.h"

class Tween {
friend class Effects;
public:
    virtual ~Tween() = default;
    void Start() {
        startTime = clock;
        started = true;
    }

    virtual void Update() = 0;

    bool Finished() const {
        return finished;
    }

    void SetNext(Tween* tween) {
        nextTween = tween;
    }

    void TakeOwnership() {
        ownedByEffectsSystem = false;
    }

    bool IsOwnedByEffectsSystem() const {
        return ownedByEffectsSystem;
    }

    virtual void Stop(const bool naturalStop = false) {
        finished = true;
        if (naturalStop) {
            if (nextTween) nextTween->Start();
        }
    }

    void SetTag(std::string _tag) {
        tag = _tag;
    }

    bool HasTag(std::string a_tag) const {
        return tag.compare(a_tag) == 0;
    }
protected:
    Tween(const Time a_duration, Time& _clock = StateManager::globalTime) :
        started(false), finished(false), clock(_clock), duration(a_duration), nextTween(nullptr), ownedByEffectsSystem(true) {}

    bool started;
    bool finished;

    Time& clock;
    Time startTime;
    Time duration;

    Tween* nextTween;

    bool ownedByEffectsSystem;

    std::string tag;
};

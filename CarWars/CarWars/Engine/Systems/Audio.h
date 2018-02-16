#pragma once

#include "System.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"

class Audio : public System {
public:
    // Access the singleton instance
    static Audio& Instance();
    ~Audio();

    void Initialize();

    void Update(Time currentTime, Time deltaTime) override;

private:
    // No instantiation or copying
    Audio();
    Audio(const Audio&) = delete;
    Audio& operator= (const Audio&) = delete;
};

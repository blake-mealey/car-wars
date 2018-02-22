#pragma once

#include "System.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"

typedef FMOD::Sound* SoundClass;

class Audio : public System {
public:
    // Access the singleton instance
    static Audio& Instance();
    ~Audio();

    void Initialize();

    void Update() override;

private:
    FMOD::System *soundSystem;
    FMOD::Sound *sound, *soundToPlay;
    FMOD::Channel *channel = 0;
    FMOD_RESULT result;
    unsigned int version;
    int numsubsounds;

    // No instantiation or copying
    Audio();
    Audio(const Audio&) = delete;
    Audio& operator= (const Audio&) = delete;
};

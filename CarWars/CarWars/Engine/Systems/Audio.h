#pragma once

#include "System.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"
#include "../Entities/EntityManager.h"
#include "glm/glm.hpp"

typedef FMOD::Sound* SoundClass;

class Audio : public System {
public:
    // Access the singleton instance
    static Audio& Instance();
    ~Audio();

    void Initialize();

    void Update() override;
    void PlayAudio(const char *filename);
    void PlayAudio(const char *filename, glm::vec3 position, glm::vec3 velocity);

private:
    FMOD::System *soundSystem;
    FMOD::Sound *sound, *soundToPlay;
    FMOD::Sound *sound1, *sound2;
    FMOD::Channel *channel = 0, *channel2 = 0;
    FMOD_RESULT result;
    unsigned int version;
    int numsubsounds;

    float minDistance = 0.5f;
    float maxDistance = 5000.f;

    // No instantiation or copying
    Audio();
    Audio(const Audio&) = delete;
    Audio& operator= (const Audio&) = delete;
};

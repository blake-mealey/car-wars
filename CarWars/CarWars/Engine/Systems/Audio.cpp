#include "Audio.h"

// Singleton
Audio::Audio() { }
Audio &Audio::Instance() {
    static Audio instance;
    return instance;
}

Audio::~Audio() { 
    sound->release();
    soundSystem->close();
    soundSystem->release();
}

void Audio::Initialize() { 
    FMOD::System_Create(&soundSystem);
    soundSystem->init(32, FMOD_INIT_NORMAL, 0);
    soundSystem->createStream("Content/Sounds/engine-running.mp3", FMOD_LOOP_NORMAL | FMOD_2D, 0, &sound);
    sound->getNumSubSounds(&numsubsounds);

    if (numsubsounds) {
        sound->getSubSound(0, &soundToPlay);
    } else {
        soundToPlay = sound;
    }
    soundSystem->playSound(soundToPlay, 0, false, &channel);
}

void Audio::Update() { 
    soundSystem->update();
}

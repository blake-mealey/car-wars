#include "Audio.h"
#include "../Entities/EntityManager.h"

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
    soundSystem->set3DSettings(1.0f, 1.0f, 1.0f); 
    //soundSystem->set3DNumListeners(number of players...)
}

void Audio::PlayAudio(const char *filename) {
    soundSystem->createStream(filename, FMOD_LOOP_OFF | FMOD_3D, 0, &sound);
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

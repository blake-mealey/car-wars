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
    soundSystem->set3DSettings(1.0f, 1.0f, 1.0f); 
    soundSystem->set3DNumListeners(4); // need to get number of players
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

    //FMOD_VECTOR pos = { 10.0f * 1.0f, 0.0f, 0.0f };
    //FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
    //soundSystem->playSound(sound1, 0, true, &channel2);
    //channel2->set3DAttributes(&pos, &vel);
    //channel2->setPaused(false); // sound starts paused
}

void Audio::PlayAudio(const char *filename, glm::vec3 position, glm::vec3 velocity) {
    FMOD_VECTOR pos = { position.x, position.y, position.z };
    FMOD_VECTOR vel = { velocity.x, velocity.y, velocity.z };

    soundSystem->createSound(filename, FMOD_3D, 0, &sound1);
    sound1->set3DMinMaxDistance(minDistance, maxDistance);
    sound1->setMode(FMOD_LOOP_OFF);
    soundSystem->playSound(sound1, 0, true, &channel2);
    channel2->set3DAttributes(&pos, &vel);
    channel2->setPaused(false);
}

void Audio::Update() { 
    // fetch from physx
    // iterate over number of players and adjsust listeners
    //soundSystem->set3DListenerAttributes(0, &listenerpos, &vel, &forward, &up);
    
    soundSystem->update();
}

#include "Audio.h"
#include "../Entities/EntityManager.h"
#include "StateManager.h"

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
    //soundSystem->set3DNumListeners(number of players...)
}

void Audio::PlayAudio(const char *filename) {
    soundSystem->createStream(filename, FMOD_LOOP_OFF | FMOD_2D, 0, &sound);
    sound->getNumSubSounds(&numsubsounds);

    if (numsubsounds) {
        sound->getSubSound(0, &soundToPlay);
    } else {
        soundToPlay = sound;
    }
    soundSystem->playSound(soundToPlay, 0, false, &channel);
}

FMOD_VECTOR GlmToFmodVector(glm::vec3 v) {
    return {v.x, v.y, v.z};
}

void Audio::Update() { 
    float timePassed = StateManager::deltaTime.GetTimeMilliSeconds();
    //glm::vec3 vehiclePos = EntityManager::FindEntities("Vehicle")[0]->transform.GetGlobalPosition();
    //glm::vec3 cameraPos = EntityManager::FindEntities("Vehicle")[0]->transform.GetGlobalPosition();
    //Transform vehiclePos = EntityManager::FindEntities("Vehicle")[0]->transform;

    //soundSystem->set3DListenerAttributes(0,)
    soundSystem->update();
}

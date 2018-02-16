#include "Audio.h"

// Singleton
Audio::Audio() { }
Audio &Audio::Instance() {
    static Audio instance;
    return instance;
}

Audio::~Audio() { }

void Audio::Initialize() { }

void Audio::Update(Time currentTime, Time deltaTime) { }

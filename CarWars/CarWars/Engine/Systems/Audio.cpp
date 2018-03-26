#include "Audio.h"
#include <iostream>

// Singleton
Audio::Audio() { }

Audio &Audio::Instance() {
    static Audio instance;
    return instance;
}

Audio::~Audio() { 
    sound->release();
    for (auto s : carSounds) { s.sound->release(); }
    soundSystem->close();
    soundSystem->release();
}

void Audio::Initialize() { 
    FMOD::System_Create(&soundSystem);
    soundSystem->init(MAX_CHANNELS, FMOD_INIT_NORMAL, 0);
    soundSystem->set3DSettings(1.0f, 1.0f, 1.0f); 
    soundSystem->set3DNumListeners(Game::gameData.humanCount);

    for (int i = 0; i < 100; i++) { availableSound[i] = true; }

    prevGameState = StateManager::GetState();
    // main screen intro music
    PlayMusic("Content/Music/imperial-march.mp3");
}

void Audio::PlayAudio2D(const char *filename) {
    //soundSystem->createStream(filename, FMOD_LOOP_NORMAL | FMOD_2D, 0, &sound);
    //soundSystem->playSound(sound, 0, false, &channel);
    PlayAudio(filename, 1.f);
}

void Audio::PlayAudio(const char *filename) {
    //sound->getNumSubSounds(&numsubsounds);

    //if (numsubsounds) {
    //    sound->getSubSound(0, &soundToPlay);
    //} else {
    //    soundToPlay = sound;
    //}

    PlayAudio(filename, 1.f);
}

int Audio::PlaySound(const char *filename) {
    int index=0;
    for (auto s : availableSound) {
        if (s) break;
        index++;
    }
    availableSound[index] = false;
    soundSystem->createSound(filename, FMOD_3D | FMOD_LOOP_OFF, 0, &soundArray[index]);
    soundSystem->playSound(soundArray[index], 0, false, &channelArray[index]);
    channelArray[index]->setVolume(1.f);
    channelArray[index]->setPaused(false);

    return index;
}

void Audio::StopSound(int index) {
    availableSound[index] = true;
    soundArray[index]->release();
}

void Audio::PlayAudio(const char *filename, float volume) {
    soundSystem->createStream(filename, FMOD_LOOP_OFF | FMOD_3D, 0, &sound);
    soundSystem->playSound(sound, 0, false, &channel);
    channel->setVolume(volume);
}

void Audio::PlayAudio(const char *filename, glm::vec3 position, glm::vec3 velocity) {
    FMOD_VECTOR pos = { position.x, position.y, position.z };
    FMOD_VECTOR vel = { velocity.x, velocity.y, velocity.z };
    sound1->release();
    soundSystem->createSound(filename, FMOD_3D, 0, &sound1);
    sound1->set3DMinMaxDistance(MIN_DISTANCE, MAX_DISTANCE);
    sound1->setMode(FMOD_LOOP_OFF);
    soundSystem->playSound(sound1, 0, true, &channel2);
    channel2->set3DAttributes(&pos, &vel);
    channel2->setPaused(false);
    channel2->setVolume(5.f);
}

void Audio::PlayAudio3D(const char *filename, glm::vec3 position, glm::vec3 velocity) {
    PlayAudio(filename, position, velocity);
}

void Audio::PlayMusic(const char *filename) {
    result = soundSystem->createStream(filename, FMOD_LOOP_NORMAL | FMOD_2D, 0, &music);
    result = soundSystem->playSound(music, 0, false, &musicChannel);
    musicChannel->setVolume(musicVolume);
}

void Audio::MenuMusicControl() {
    auto currGameState = StateManager::GetState();
    if (currGameState != prevGameState) {
        if (currGameState == GameState_Playing) {
            music->release();
            prevGameState = currGameState;
            PlayMusic("Content/Music/unity.mp3");
            musicChannel->setPosition(gameMusicPosition, FMOD_TIMEUNIT_MS);
        } else if (currGameState == GameState_Paused) {
            musicChannel->getPosition(&gameMusicPosition, FMOD_TIMEUNIT_MS);
            music->release();
            prevGameState = currGameState;
            PlayMusic("Content/Music/imperial-march.mp3");
        } else if (currGameState == GameState_Menu) {
            gameMusicPosition = 0;
        }
    }
}

void Audio::UpdateListeners() {
    // update listener position for every camera/player vehicle
    if (StateManager::GetState() == GameState_Playing) {
        for (int i = 0; i < Game::gameData.humanCount; i++) {
            FMOD_VECTOR velocity, forward, up, position;
            auto player = Game::humanPlayers[i];
			if (!player.alive) return;
            auto carForward = player.vehicleEntity->transform.GetForward();
            auto carUp = player.vehicleEntity->transform.GetUp();
            auto carPosition = player.vehicleEntity->transform.GetGlobalPosition();
            auto carVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
            forward = { carForward.x, carForward.y, carForward.z };
            up = { carUp.x, carUp.y, carUp.z };
            position = { carPosition.x, carPosition.y, carPosition.z };
            velocity = { carVelocity.x, carVelocity.y, carVelocity.z };
            soundSystem->set3DListenerAttributes(i, &position, &velocity, &forward, &up);
        }
    }
}

void Audio::StartCars() {
    //const char *engineSound = "Content/Sounds/engine-running.mp3";
    const char *engineSound = "Content/Sounds/Truck/idle.mp3";

    // channels and sounds for cars
    carSounds.resize(
        Game::gameData.aiCount +
        Game::gameData.humanCount);

    //players
    for (int i = 0; i < Game::gameData.humanCount; i++) {
        auto playerPos = Game::humanPlayers[i].vehicleEntity->transform.GetGlobalPosition();
        soundSystem->createSound(engineSound, FMOD_3D | FMOD_LOOP_NORMAL, 0, &carSounds[i].sound);
        carSounds[i].sound->set3DMinMaxDistance(MIN_DISTANCE, MAX_DISTANCE);
        soundSystem->playSound(carSounds[i].sound, 0, true, &carSounds[i].channel);
        FMOD_VECTOR pos = { playerPos.x+1.0f, playerPos.y+1.0f, playerPos.z+1.0f };
        FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
        carSounds[i].channel->set3DAttributes(&pos, &vel);
        carSounds[i].channel->setPaused(false);
        carSounds[i].channel->setVolume(0.25f);
    }
    size_t offset = Game::gameData.humanCount;
    //ai
    for (int i = 0; i < Game::gameData.aiCount; i++) {
        auto aiPos = Game::ais[i].vehicleEntity->transform.GetGlobalPosition();
        soundSystem->createSound(engineSound, FMOD_3D | FMOD_LOOP_NORMAL, 0, &carSounds[i + offset].sound);
        carSounds[i + offset].sound->set3DMinMaxDistance(MIN_DISTANCE, MAX_DISTANCE);
        soundSystem->playSound(carSounds[i + offset].sound, 0, true, &carSounds[i + offset].channel);
        FMOD_VECTOR pos = { aiPos.x, aiPos.y, aiPos.z };
        FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
        carSounds[i + offset].channel->set3DAttributes(&pos, &vel);
        carSounds[i + offset].channel->setPaused(false);
        carSounds[i + offset].channel->setVolume(0.5f);
    }
    carsStarted = true;
}

void Audio::PauseCars(bool paused) {
    for (auto carSound : carSounds) { carSound.channel->setPaused(paused); }
}

void Audio::UpdateCars() {
    const char *engineIdle = "Content/Sounds/Truck/idle.mp3";
    const char *engineAccelerate = "Content/Sounds/Truck/accelerate.mp3";
    const char *engineReverse = "Content/Sounds/Truck/reverse.mp3";

    //player
    for (int i = 0; i < Game::gameData.humanCount; i++) {
        PlayerData& player = Game::humanPlayers[i];
        if (!player.alive) continue;
        const auto playerPos = player.vehicleEntity->transform.GetGlobalPosition();
        VehicleComponent* vehicle = Game::humanPlayers[i].vehicleEntity->GetComponent<VehicleComponent>();
        if (vehicle->pxVehicle->mDriveDynData.getCurrentGear() == PxVehicleGearsData::eREVERSE) {
            // set reverse sound
            if (!carSounds[i].reversing) {
                carSounds[i].sound->release();
                carSounds[i].reversing = true;
                carSounds[i].changedDirection = true;
                soundSystem->createSound(engineReverse, FMOD_3D | FMOD_LOOP_NORMAL, 0, &carSounds[i].sound);
                carSounds[i].sound->set3DMinMaxDistance(MIN_DISTANCE, MAX_DISTANCE);
                soundSystem->playSound(carSounds[i].sound, 0, true, &carSounds[i].channel);
                carSounds[i].channel->setPaused(false);

            }
        } else {
            // set forward sound
            if (carSounds[i].changedDirection) {
                carSounds[i].sound->release();
                carSounds[i].changedDirection = false;
                carSounds[i].reversing = false;
                soundSystem->createSound(engineIdle, FMOD_3D | FMOD_LOOP_NORMAL, 0, &carSounds[i].sound);
                carSounds[i].sound->set3DMinMaxDistance(MIN_DISTANCE, MAX_DISTANCE);
                soundSystem->playSound(carSounds[i].sound, 0, true, &carSounds[i].channel);
                carSounds[i].channel->setPaused(false);

            }
        }
        //soundSystem->createSound(engineSound, FMOD_3D | FMOD_LOOP_NORMAL, 0, &carSounds[i].sound);
        //carSounds[i].sound->set3DMinMaxDistance(MIN_DISTANCE, MAX_DISTANCE);
        //soundSystem->playSound(carSounds[i].sound, 0, true, &carSounds[i].channel);
        FMOD_VECTOR pos = { playerPos.x, playerPos.y+1.0f, playerPos.z };
        FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
        carSounds[i].channel->set3DAttributes(&pos, &vel);
        //carSounds[i].channel->setPaused(true);
        carSounds[i].channel->setVolume(1.0f);
    }
    size_t offset = Game::gameData.humanCount;
    //ai
    for (int i = 0; i < Game::gameData.aiCount; i++) {
        AiData& ai = Game::ais[i];
        if (!ai.alive) continue;
        const auto aiPos = ai.vehicleEntity->transform.GetGlobalPosition();
        //soundSystem->createSound(engineSound, FMOD_3D | FMOD_LOOP_NORMAL, 0, &carSounds[i + offset].sound);
        //carSounds[i + offset].sound->set3DMinMaxDistance(MIN_DISTANCE, MAX_DISTANCE);
        //soundSystem->playSound(carSounds[i + offset].sound, 0, true, &carSounds[i + offset].channel);
        FMOD_VECTOR pos = { aiPos.x, aiPos.y, aiPos.z };
        FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
        carSounds[i + offset].channel->set3DAttributes(&pos, &vel);
        //carSounds[i + offset].channel->setPaused(false);
        carSounds[i + offset].channel->setVolume(3.0f);
    }
}

void Audio::StopCars() {
    for (auto carSound : carSounds) { carSound.sound->release(); }
    carsStarted = false;
}


void Audio::UpdateRunningCars() {
    auto currGameState = StateManager::GetState();
    if (currGameState != prevGameState) {
        if (carsStarted && currGameState == GameState_Playing) {
            PauseCars(false);
        } else if (!carsStarted && currGameState == GameState_Playing) {
            StartCars();
        } else if (carsStarted && currGameState == GameState_Paused) {
            PauseCars(true);
        } else if (carsStarted && currGameState == GameState_Menu) {
            StopCars();
        }
    }
    if (carsStarted && currGameState == GameState_Playing) {
        UpdateCars();
    }
}

void Audio::Update() { 
    UpdateRunningCars();
    MenuMusicControl(); // prevGameState saved
    UpdateListeners();

    soundSystem->update();
}

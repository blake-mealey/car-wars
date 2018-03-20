#include "Audio.h"

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
    soundSystem->set3DNumListeners(Game::gameData.playerCount);


    prevGameState = StateManager::GetState();
    // main screen intro music
    PlayMusic("Content/Music/imperial-march.mp3");
}

void Audio::PlayAudio2D(const char *filename) {
    soundSystem->createStream(filename, FMOD_LOOP_NORMAL | FMOD_2D, 0, &sound);
    soundSystem->playSound(sound, 0, false, &channel);
}

void Audio::PlayAudio(const char *filename) {
    soundSystem->createStream(filename, FMOD_LOOP_OFF | FMOD_3D, 0, &sound);
    //sound->getNumSubSounds(&numsubsounds);

    //if (numsubsounds) {
    //    sound->getSubSound(0, &soundToPlay);
    //} else {
    //    soundToPlay = sound;
    //}
    soundSystem->playSound(sound, 0, false, &channel);
}

void Audio::PlayAudio(const char *filename, glm::vec3 position, glm::vec3 velocity) {
    FMOD_VECTOR pos = { position.x, position.y, position.z };
    FMOD_VECTOR vel = { velocity.x, velocity.y, velocity.z };

    soundSystem->createSound(filename, FMOD_3D, 0, &sound1);
    sound1->set3DMinMaxDistance(MIN_DISTANCE, MAX_DISTANCE);
    sound1->setMode(FMOD_LOOP_OFF);
    soundSystem->playSound(sound1, 0, true, &channel2);
    channel2->set3DAttributes(&pos, &vel);
    channel2->setPaused(false);
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
        for (int i = 0; i < Game::gameData.playerCount; i++) {
            FMOD_VECTOR velocity, forward, up, position;
            auto player = Game::players[i];
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

    //for (int i = 0; i<cameras.size(); i++) {
    //    FMOD_VECTOR velocity, forward, up;
    //    if (cars.size() < 1 || StateManager::GetState() != GameState_Playing) {
    //        velocity = { 0.0, 0.0, 0.0 };
    //        forward = { 0.0, 0.0, 1.0 };
    //        up = { 0.0, 1.0, 0.0 };
    //    } else { // transform causing a crash
    //             //glm::vec3 carUp = cars[i]->transform.GetUp();
    //             //glm::vec3 carForward = cars[i]->transform.GetForward();
    //        velocity = { 0.0, 0.0, 0.0 };
    //        //forward = { carForward.x, carForward.y, carForward.z };
    //        //up = { carUp.x, carUp.y, carUp.z };
    //        forward = { 0.0, 0.0, 1.0 };
    //        up = { 0.0, 1.0, 0.0 };
    //    }

    //    glm::vec3 cameraPos = static_cast<CameraComponent*>(cameras[i])->GetPosition();
    //    FMOD_VECTOR position = { cameraPos.x, cameraPos.y, cameraPos.z };
    //}
}

void Audio::StartCars() {
    //const char *engineSound = "Content/Sounds/engine-running.mp3";
    const char *engineSound = "Content/Sounds/Truck/idle.mp3";

    // channels and sounds for cars
    carSounds.resize(
        Game::gameData.aiCount +
        Game::gameData.playerCount);

    //players
    for (int i = 0; i < Game::gameData.playerCount; i++) {
        auto playerPos = Game::players[i].vehicleEntity->transform.GetGlobalPosition();
        soundSystem->createSound(engineSound, FMOD_3D | FMOD_LOOP_NORMAL, 0, &carSounds[i].sound);
        carSounds[i].sound->set3DMinMaxDistance(MIN_DISTANCE, MAX_DISTANCE);
        soundSystem->playSound(carSounds[i].sound, 0, true, &carSounds[i].channel);
        FMOD_VECTOR pos = { playerPos.x+1.0f, playerPos.y+1.0f, playerPos.z+1.0f };
        FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
        carSounds[i].channel->set3DAttributes(&pos, &vel);
        carSounds[i].channel->setPaused(true);
        carSounds[i].channel->setVolume(10.0f);
    }
    size_t offset = Game::gameData.playerCount;
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
        carSounds[i + offset].channel->setVolume(5.0f);
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

    //players
    for (int i = 0; i < Game::gameData.playerCount; i++) {
        auto playerPos = Game::players[i].vehicleEntity->transform.GetGlobalPosition();
        //auto vehicle = Game::players[i].vehicleEntity->
        //soundSystem->createSound(engineSound, FMOD_3D | FMOD_LOOP_NORMAL, 0, &carSounds[i].sound);
        //carSounds[i].sound->set3DMinMaxDistance(MIN_DISTANCE, MAX_DISTANCE);
        //soundSystem->playSound(carSounds[i].sound, 0, true, &carSounds[i].channel);
        FMOD_VECTOR pos = { playerPos.x, playerPos.y+1.0f, playerPos.z };
        FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
        carSounds[i].channel->set3DAttributes(&pos, &vel);
        //carSounds[i].channel->setPaused(true);
        //carSounds[i].channel->setVolume(3.0f);
    }
    size_t offset = Game::gameData.playerCount;
    //ai
    for (int i = 0; i < Game::gameData.aiCount; i++) {
        auto aiPos = Game::ais[i].vehicleEntity->transform.GetGlobalPosition();
        //soundSystem->createSound(engineSound, FMOD_3D | FMOD_LOOP_NORMAL, 0, &carSounds[i + offset].sound);
        //carSounds[i + offset].sound->set3DMinMaxDistance(MIN_DISTANCE, MAX_DISTANCE);
        //soundSystem->playSound(carSounds[i + offset].sound, 0, true, &carSounds[i + offset].channel);
        FMOD_VECTOR pos = { aiPos.x, aiPos.y, aiPos.z };
        FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
        carSounds[i + offset].channel->set3DAttributes(&pos, &vel);
        //carSounds[i + offset].channel->setPaused(false);
        //carSounds[i + offset].channel->setVolume(5.0f);
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
    // update car sounds
    auto cars = EntityManager::FindEntities("Vehicle");


    soundSystem->update();
}

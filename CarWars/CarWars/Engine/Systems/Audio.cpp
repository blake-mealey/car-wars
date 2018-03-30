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
    sound3d->release();
    music->release();
    for (auto s : carSounds) { s.sound->release(); }
    for (auto s : soundArray) { s->release(); }
    for (auto s : soundArray3D) { s->release(); }
    soundSystem->close();
    soundSystem->release();
}

void Audio::ReleaseSounds() {
    sound->release();
    sound3d->release();
    for (auto s : carSounds) { s.sound->release(); }
    for (auto s : soundArray) { s->release(); }
    for (auto s : soundArray3D) { s->release(); }
}

void Audio::Initialize() { 
    srand(time(NULL));
    currentMusicIndex = rand() % NUM_MUSIC;
    FMOD::System_Create(&soundSystem);
    soundSystem->init(MAX_CHANNELS, FMOD_INIT_NORMAL, 0);
    soundSystem->set3DSettings(1.0f, 1.f, .18f); 
    soundSystem->set3DNumListeners(Game::gameData.humanCount);

	for (int i = 0; i < 100; i++) { availableSound[i] = true; }
	for (int i = 0; i < 100; i++) { availableSound3D[i] = true; }

    prevGameState = StateManager::GetState();
    // main screen intro music
    PlayMusic("Content/Music/imperial-march.mp3");
}

void Audio::PlayAudio2D(const char *filename) {
    //soundSystem->createStream(filename, FMOD_LOOP_NORMAL | FMOD_2D, 0, &sound);
    //soundSystem->playSound(sound, 0, false, &channel);
    PlayAudio(filename, 0.11f);
}

void Audio::PlayAudio(const char *filename) {
    

    PlayAudio(filename, 1.f);
}

int Audio::PlaySound3D(const char *filename, glm::vec3 position, glm::vec3 velocity, float volume) {
	FMOD_VECTOR pos = { position.x, position.y, position.z };
	FMOD_VECTOR vel = { velocity.x, velocity.y, velocity.z };
	int index = 0;
	for (auto s : availableSound3D) {
		if (s) break;
		index++;
	}
	availableSound3D[index] = false;
	soundSystem->createSound(filename, FMOD_3D | FMOD_LOOP_OFF, 0, &soundArray3D[index]);
	soundSystem->playSound(soundArray3D[index], 0, false, &channelArray3D[index]);
	channelArray3D[index]->setVolume(volume);
	channelArray3D[index]->set3DAttributes(&pos, &vel);
	channelArray3D[index]->setPaused(false);
	return index;
}

void Audio::StopSound3D(int index) {
	availableSound3D[index] = true;
    channelArray3D[index]->setPaused(true);
	soundArray3D[index]->release();
}

int Audio::PlaySound(const char *filename) {
    int index=0;
    for (auto s : availableSound) {
        if (s) break;
        index++;
    }
    availableSound[index] = false;
    soundSystem->createSound(filename, FMOD_2D | FMOD_LOOP_OFF, 0, &soundArray[index]); // to ignore positioning for now
    //soundSystem->createSound(filename, FMOD_3D | FMOD_LOOP_OFF, 0, &soundArray[index]);
    soundSystem->playSound(soundArray[index], 0, false, &channelArray[index]);
    channelArray[index]->setVolume(0.55f);
    channelArray[index]->setPaused(false);
    return index;
}

void Audio::StopSound(int index) {
    availableSound[index] = true;
	soundArray[index]->release();
}

void Audio::PlayAudio(const char *filename, float volume) {
    sound->getNumSubSounds(&numsubsounds);

    if (numsubsounds) {
        sound->getSubSound(0, &soundToPlay);
    } else {
        soundToPlay = sound;
    }

    soundSystem->createStream(filename, FMOD_LOOP_OFF | FMOD_3D, 0, &soundToPlay);
    soundSystem->playSound(soundToPlay, 0, false, &channel);
    channel->setVolume(volume);
}

void Audio::PlayAudio(const char *filename, glm::vec3 position, glm::vec3 velocity) {
    PlayAudio(filename, position, velocity, 1.f);
}

void Audio::PlayAudio(const char *filename, glm::vec3 position, glm::vec3 velocity, float volume) {
    //sound3d->getNumSubSounds(&numsubsounds3d);

    //if (numsubsounds3d) {
    //    sound3d->getSubSound(0, &soundToPlay3d);
    //} else {
        soundToPlay3d = sound3d;
    //}


    FMOD_VECTOR pos = { position.x, position.y, position.z };
    FMOD_VECTOR vel = { velocity.x, velocity.y, velocity.z };
    soundSystem->createSound(filename, FMOD_3D, 0, &soundToPlay3d);
    soundToPlay3d->set3DMinMaxDistance(MIN_DISTANCE, MAX_DISTANCE);
    soundToPlay3d->setMode(FMOD_LOOP_OFF);
    soundSystem->playSound(soundToPlay3d, 0, true, &channel3d);
    channel3d->set3DAttributes(&pos, &vel);
    channel3d->setPaused(false);
    channel3d->setVolume(volume);
}

void Audio::PlayAudio3D(const char *filename, glm::vec3 position, glm::vec3 velocity) {
    PlayAudio(filename, position, velocity);
}
void Audio::PlayAudio3D(const char *filename, glm::vec3 position, glm::vec3 velocity, float volume) {
    PlayAudio(filename, position, velocity, volume);
}

void Audio::PauseSounds() {
    for (auto c : channelArray3D) c->setPaused(true);
    for (auto c : channelArray) c->setPaused(true);
    channel->setPaused(true);
    channel3d->setPaused(true);
}
void Audio::ResumeSounds() {
    for (auto c : channelArray3D) c->setPaused(false);
    for (auto c : channelArray) c->setPaused(false);
    channel->setPaused(false);
    channel3d->setPaused(false);
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
			ResumeSounds();
            if (!gameStarted) {
                music->release();
                PlayMusic(musicPlaylist[currentMusicIndex]);
                gameStarted = true;
            }
            //musicChannel->setPosition(gameMusicPosition, FMOD_TIMEUNIT_MS);
        } else if (currGameState == GameState_Paused) {
			PauseSounds();
            //musicChannel->getPosition(&gameMusicPosition, FMOD_TIMEUNIT_MS);
            //music->release();
            //PlayMusic("Content/Music/imperial-march.mp3");
        } else if (currGameState == GameState_Menu) {
            //gameMusicPosition = 0;
            srand(time(NULL));
            currentMusicIndex = rand() % NUM_MUSIC;
            ReleaseSounds();
            if (gameStarted) {
                music->release();
                PlayMusic("Content/Music/imperial-march.mp3");
                gameStarted = false;
            }
        }
        prevGameState = currGameState;

    }
}

void Audio::UpdateListeners() {
    // update listener position for every camera/player vehicle
    if (StateManager::GetState() == GameState_Playing) {
        for (int i = 0; i < 4; i++) {
            auto player = Game::humanPlayers[i];
            if (!player.ready || !player.alive) continue;
            const auto carForward = player.vehicleEntity->transform.GetForward();
            const auto carUp = player.vehicleEntity->transform.GetUp();
            const auto carPosition = player.vehicleEntity->transform.GetGlobalPosition();
            const auto carVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
            FMOD_VECTOR forward = { carForward.x, carForward.y, carForward.z };
            FMOD_VECTOR up = { carUp.x, carUp.y, carUp.z };
            FMOD_VECTOR position = { carPosition.x, carPosition.y, carPosition.z };
            FMOD_VECTOR velocity = { carVelocity.x, carVelocity.y, carVelocity.z };
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
    for (int i = 0; i < 4; i++) {
        HumanData& player = Game::humanPlayers[i];
        if (!player.ready || !player.alive) continue;
        const auto playerPos = player.vehicleEntity->transform.GetGlobalPosition();
        soundSystem->createSound(engineSound, FMOD_3D | FMOD_LOOP_NORMAL, 0, &carSounds[i].sound);
        carSounds[i].sound->set3DMinMaxDistance(MIN_DISTANCE, MAX_DISTANCE);
        soundSystem->playSound(carSounds[i].sound, 0, true, &carSounds[i].channel);
        FMOD_VECTOR pos = { playerPos.x+1.0f, playerPos.y+1.0f, playerPos.z+1.0f };
        FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
        carSounds[i].channel->set3DAttributes(&pos, &vel);
        carSounds[i].channel->setPaused(false);
        carSounds[i].channel->setVolume(playerSoundVolume);
    }
    size_t offset = Game::gameData.humanCount;
    //ai
    for (int i = 0; i < Game::gameData.aiCount; i++) {
		AiData& player = Game::aiPlayers[i];
		if (!player.alive) continue;
        auto aiPos = player.vehicleEntity->transform.GetGlobalPosition();
        soundSystem->createSound(engineSound, FMOD_3D | FMOD_LOOP_NORMAL, 0, &carSounds[i + offset].sound);
        carSounds[i + offset].sound->set3DMinMaxDistance(MIN_DISTANCE, MAX_DISTANCE);
        soundSystem->playSound(carSounds[i + offset].sound, 0, true, &carSounds[i + offset].channel);
        FMOD_VECTOR pos = { aiPos.x, aiPos.y, aiPos.z };
        FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
        carSounds[i + offset].channel->set3DAttributes(&pos, &vel);
        carSounds[i + offset].channel->setPaused(false);
        carSounds[i + offset].channel->setVolume(aiSoundVolume);
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
    for (int i = 0; i < 4; i++) {
        HumanData& player = Game::humanPlayers[i];
        if (!player.ready || !player.alive) continue;
        const auto playerPos = player.vehicleEntity->transform.GetGlobalPosition();
        VehicleComponent* vehicle = player.vehicleEntity->GetComponent<VehicleComponent>();
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
        carSounds[i].channel->setVolume(playerSoundVolume);
    }
    size_t offset = Game::gameData.humanCount;
    //ai
    for (int i = 0; i < Game::gameData.aiCount; i++) {
        AiData& ai = Game::aiPlayers[i];
        if (!ai.alive) continue;
        const auto aiPos = ai.vehicleEntity->transform.GetGlobalPosition();
        //soundSystem->createSound(engineSound, FMOD_3D | FMOD_LOOP_NORMAL, 0, &carSounds[i + offset].sound);
        //carSounds[i + offset].sound->set3DMinMaxDistance(MIN_DISTANCE, MAX_DISTANCE);
        //soundSystem->playSound(carSounds[i + offset].sound, 0, true, &carSounds[i + offset].channel);
        FMOD_VECTOR pos = { aiPos.x, aiPos.y, aiPos.z };
        FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
        carSounds[i + offset].channel->set3DAttributes(&pos, &vel);
        //carSounds[i + offset].channel->setPaused(false);
        carSounds[i + offset].channel->setVolume(aiSoundVolume);
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

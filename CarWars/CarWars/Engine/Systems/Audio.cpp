#include "Audio.h"

// Singleton
Audio::Audio() { }

Audio &Audio::Instance() {
    static Audio instance;
    return instance;
}

Audio::~Audio() { 
    sound->release();
    //for (auto s : sounds2D) { s->release(); }
    //for (auto s : sounds3D) { s->release(); }
    for (auto s : carSounds) { s->release(); }
    soundSystem->close();
    soundSystem->release();
}

void Audio::Initialize() { 
    FMOD::System_Create(&soundSystem);
    soundSystem->init(MAX_CHANNELS, FMOD_INIT_NORMAL, 0);
    soundSystem->set3DSettings(1.0f, 1.0f, 1.0f); 
    soundSystem->set3DNumListeners(Game::numberOfPlayers);

    // channels and sounds for cars
    carSounds.resize(Game::numberOfAi+Game::numberOfPlayers);
    carChannels.resize(Game::numberOfAi + Game::numberOfPlayers);

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

    soundSystem->createSound("Content/Sounds/drumloop.wav", FMOD_3D, 0, &sound1);
    sound1->set3DMinMaxDistance(MIN_DISTANCE, MAX_DISTANCE);
    sound1->setMode(FMOD_LOOP_NORMAL);

    FMOD_VECTOR pos = { -10.0f * 1.0f, 0.0f, 0.0f };
    FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
    soundSystem->playSound(sound1, 0, true, &channel);
    channel->set3DAttributes(&pos, &vel);
    //channel->setPaused(false); // sound starts paused
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
}

void Audio::MenuMusicControl() {
    auto currGameState = StateManager::GetState();
    if (currGameState != prevGameState && currGameState == GameState_Playing) {
        music->release();
        prevGameState = currGameState;
        PlayMusic("Content/Music/unity.mp3");
        musicChannel->setPosition(gameMusicPosition, FMOD_TIMEUNIT_MS);
    } else if (currGameState != prevGameState && currGameState == GameState_Paused) {
        musicChannel->getPosition(&gameMusicPosition, FMOD_TIMEUNIT_MS);
        music->release();
        prevGameState = currGameState;
        PlayMusic("Content/Music/imperial-march.mp3");
    }
}

void Audio::UpdateListeners() {
    // update listener position for every camera/player vehicle

    auto cars = EntityManager::FindEntities("Vehicle");
    auto cameras = EntityManager::GetComponents(ComponentType_Camera);

    for (int i = 0; i<cameras.size(); i++) {
        FMOD_VECTOR velocity, forward, up;
        if (cars.size() < 1 || StateManager::GetState() != GameState_Playing) {
            velocity = { 0.0, 0.0, 0.0 };
            forward = { 0.0, 0.0, 1.0 };
            up = { 0.0, 1.0, 0.0 };
        } else { // transform causing a crash
                 //glm::vec3 carUp = cars[i]->transform.GetUp();
                 //glm::vec3 carForward = cars[i]->transform.GetForward();
            velocity = { 0.0, 0.0, 0.0 };
            //forward = { carForward.x, carForward.y, carForward.z };
            //up = { carUp.x, carUp.y, carUp.z };
            forward = { 0.0, 0.0, 1.0 };
            up = { 0.0, 1.0, 0.0 };
        }

        glm::vec3 cameraPos = static_cast<CameraComponent*>(cameras[i])->GetPosition();
        FMOD_VECTOR position = { cameraPos.x, cameraPos.y, cameraPos.z };

        soundSystem->set3DListenerAttributes(i, &position, &velocity, &forward, &up);
    }
}

void Audio::UpdateRunningCars() {

}

void Audio::Update() { 
    MenuMusicControl();
    UpdateListeners();
    // update car sounds
    auto cars = EntityManager::FindEntities("Vehicle");


    soundSystem->update();
}

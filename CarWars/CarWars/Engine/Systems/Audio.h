#pragma once

#include <vector>

#include "System.h"
#include "Game.h"
#include "StateManager.h"
#include "../Components/CameraComponent.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"
#include "../Entities/EntityManager.h"
#include "glm/glm.hpp"

#define MAX_DISTANCE 5000.0
#define MIN_DISTANCE 0.5
#define MAX_CHANNELS 100

typedef FMOD::Sound* SoundClass;

struct CarSound {
    unsigned int currentSoundPos;
    glm::vec3 currentPosition;
    glm::vec3 lastPosition;
    glm::vec3 velocity;
    FMOD::Sound *sound=0;
    FMOD::Channel *channel=0;
};



class Audio : public System {
public:
    float musicVolume = 0.25f;

    // Access the singleton instance
    static Audio& Instance();
    ~Audio();

    void Initialize();

    void Update() override;
    void PlayAudio(const char *filename);
    void PlayAudio(const char *filename, glm::vec3 position, glm::vec3 velocity);
    void PlayMusic(const char *filename);
    void PlayAudio2D(const char *filename);
    void PlayAudio3D(const char *filename, glm::vec3 position, glm::vec3 velocity);

private:
    const char *musicPlaylist[6] = {
        "Content/Music/hello.mp3",
        "Content/Music/highscore.mp3",
        "Content/Music/invincible.mp3",
        "Content/Music/lone-digger.mp3",
        "Content/Music/monody.mp3",
        "Content/Music/xenogenesis.mp3"
    };

    FMOD::System *soundSystem;
    GameState prevGameState;
    bool carsStarted = false;

    //std::vector<FMOD::Sound*> sounds2D;
    //std::vector<FMOD::Sound*> sounds3D;
    //std::vector<FMOD::Channel> channels3D;
    std::vector<CarSound> carSounds;
    unsigned int gameMusicPosition;
    FMOD::Sound *music;
    FMOD::Channel *musicChannel;

    FMOD::Sound *sound, *soundToPlay;
    FMOD::Sound *sound1, *sound2;
    FMOD::Channel *channel = 0, *channel2 = 0;
    FMOD_RESULT result;
    unsigned int version;
    int numsubsounds;

    void MenuMusicControl();
    void UpdateListeners();
    void UpdateRunningCars();
    void StartCars();
    void PauseCars(bool paused);
    void UpdateCars();
    void StopCars();

    // No instantiation or copying
    Audio();
    Audio(const Audio&) = delete;
    Audio& operator= (const Audio&) = delete;
};

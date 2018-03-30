#pragma once

#include <vector>

#include "System.h"
#include "Game.h"
#include "StateManager.h"
#include "../Components/CameraComponent.h"
#include "../Components/RigidbodyComponents/VehicleComponent.h"
#include "fmod/fmod.hpp"
#include "fmod/fmod_errors.h"
#include "../Entities/EntityManager.h"
#include "glm/glm.hpp"

#define MAX_DISTANCE 5000.0
#define MIN_DISTANCE 0.15
#define MAX_CHANNELS 200
#define NUM_MUSIC 7
#define UPDATES_TO_RUN 25

typedef FMOD::Sound* SoundClass;

struct CarSound {
    unsigned int currentSoundPos;
    glm::vec3 currentPosition;
    glm::vec3 lastPosition;
    glm::vec3 velocity;
    FMOD::Sound *sound=0;
    FMOD::Channel *channel=0;
    bool changedDirection = false;
    bool reversing = false;
};


//struct HeavySounds {
//    const char *idle = "Content/Sounds/Truck/idle.mp3";
//    const char *accelerate = "Content/Sounds/Truck/accelerate.mp3";
//    const char *reverse = "Content/Sounds/Truck/reverse.mp3";
//    const char *skid = "Content/Sounds/Truck/reverse.mp3";
//    const char *spinAccelerate = "";
//};
//
//struct MediumSounds {
//    const char *idle = "Content/Sounds/Truck/idle.mp3";
//    const char *accelerate = "Content/Sounds/Truck/accelerate.mp3";
//    const char *reverse = "Content/Sounds/Truck/reverse.mp3";
//    const char *skid = "Content/Sounds/Truck/reverse.mp3";
//    const char *spinAccelerate = "";
//};
//
//struct LightSounds {
//    const char *idle = "Content/Sounds/Truck/idle.mp3";
//    const char *accelerate = "Content/Sounds/Truck/accelerate.mp3";
//    const char *reverse = "Content/Sounds/Truck/reverse.mp3";
//    const char *skid = "Content/Sounds/Truck/reverse.mp3";
//    const char *spinAccelerate = "";
//
//};
//
//struct CarSounds {
//    HeavySounds heavy;
//    MediumSounds medium;
//    LightSounds light;
//};



class Audio : public System {
public:
    float musicVolume = 0.085f;
    float aiSoundVolume = 0.25f;
    float playerSoundVolume = 0.05f;

    // Access the singleton instance
    static Audio& Instance();
    ~Audio();

    void Initialize();

    void Update() override;
    void PlayAudio(const char *filename);
    void PlayAudio(const char *filename, float volume);
    void PlayAudio(const char *filename, glm::vec3 position, glm::vec3 velocity);
    void PlayAudio(const char *filename, glm::vec3 position, glm::vec3 velocity, float volume);
    void PlayMusic(const char *filename);
    void PlayAudio2D(const char *filename);
    void PlayAudio3D(const char *filename, glm::vec3 position, glm::vec3 velocity);
    void PlayAudio3D(const char *filename, glm::vec3 position, glm::vec3 velocity, float volume);

    int PlaySound(const char* filename);
    void StopSound(int index);
	int PlaySound3D(const char* filename, glm::vec3 position, glm::vec3 velocity, float volume);
	void StopSound3D(int index);

private:
    bool gameStarted = false;
    int currentMusicIndex = 0;
    const char *musicPlaylist[NUM_MUSIC] = {
        "Content/Music/unity.mp3",
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
    
    FMOD::Sound* soundArray[100];
    FMOD::Channel* channelArray[100];
    bool availableSound[100];

	FMOD::Sound* soundArray3D[100];
	FMOD::Channel* channelArray3D[100];
	bool availableSound3D[100];

    std::vector<CarSound> carSounds;
    unsigned int gameMusicPosition, gameMusicLength;
    FMOD::Sound *music;
    FMOD::Channel *musicChannel;

    FMOD::Sound *sound, *sound3d, *soundToPlay, *soundToPlay3d;
    FMOD::Channel *channel = 0, *channel3d = 0;
    FMOD_RESULT result;
    unsigned int version;
    int numsubsounds;
    int numsubsounds3d;

	void PauseSounds();
	void ResumeSounds();
    void MenuMusicControl();
    void UpdateListeners();
    void UpdateRunningCars();
    void StartCars();
    void PauseCars(bool paused);
    void UpdateCars();
    void StopCars();
    void ReleaseSounds();
    void CheckMusic();

    // No instantiation or copying
    Audio();
    Audio(const Audio&) = delete;
    Audio& operator= (const Audio&) = delete;
};


/*
todo:
reduce volume on multiple hits
reduce bullet sound
increase railgun shoot sound
solve sound being loud when stopped

missle:
more uniform launch sound
louder explosion

*/
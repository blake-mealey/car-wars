#pragma once
#include "PowerUp.h"
#include "../../Systems/Audio.h"

PowerUp::PowerUp(Time a_duration) : duration(a_duration) {}

void PowerUp::Collect(PlayerData* player) {
    Audio& audioManager = Audio::Instance();
    audioManager.PlayAudio("Content/Sounds/powerup.mp3");

    collectedTime = StateManager::gameTime;
    player->activePowerUp = this;
}

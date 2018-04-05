#pragma once
#include "PowerUp.h"
#include "../../Systems/Audio.h"
#include "PennerEasing/Quint.h"
#include "../../Systems/Effects.h"
#include "../GuiComponents/GuiComponent.h"
#include "../SpotLightComponent.h"

PowerUp::~PowerUp() {
    Effects::Instance().DestroyTween("Headlight" + std::to_string(player->id));
    Effects::Instance().DestroyTween("PowerUpTweenIn" + std::to_string(player->id));
    Effects::Instance().DestroyTween("PowerUpTweenOut" + std::to_string(player->id));
}

PowerUp::PowerUp(Time a_duration) : duration(a_duration) {}

void PowerUp::Collect(PlayerData* a_player) {
    //Audio& audioManager = Audio::Instance();
	//audioManager.PlayAudio("Content/Sounds/powerup.mp3");
	Entity *vehicle = a_player->vehicleEntity;
	Audio::Instance().PlayAudio3D("Content/Sounds/powerup.mp3", vehicle->transform.GetGlobalPosition(), glm::vec3(0.f, 0.f, 0.f));



    collectedTime = StateManager::gameTime;
    a_player->activePowerUp = this;
    player = a_player;
}

void PowerUp::TweenVignette() const {
    HumanData* human = Game::GetHumanFromEntity(player->vehicleEntity);
    if (!human) return;
    
    Entity* guiRoot = human->camera->GetGuiRoot();
    Entity* guiEntity = EntityManager::FindFirstChild(guiRoot, GetGuiName());
    GuiComponent* gui = guiEntity->GetComponent<GuiComponent>();

    auto tweenIn = Effects::Instance().CreateTween<float, easing::Quint::easeOut>(0.f, 1.f, 0.25, StateManager::gameTime);
    tweenIn->SetTag("PowerUpTweenIn" + std::to_string(human->id));
    tweenIn->SetUpdateCallback([gui](float &value) mutable {
        gui->SetTextureOpacity(value);
        gui->transform.SetScale(glm::mix(glm::vec3(100.f, 100.f, 0.f), glm::vec3(0.f, 0.f, 0.f), value));
    });
    tweenIn->Start();

    const Time delay = duration - 1.0;

    auto tweenOut = Effects::Instance().CreateTween<float, easing::Quint::easeOut>(1.f, 0.f, duration - delay, StateManager::gameTime);
    tweenOut->SetTag("PowerUpTweenOut" + std::to_string(human->id));
    tweenOut->SetUpdateCallback([gui](float& value) mutable {
        gui->SetTextureOpacity(value);
        gui->transform.SetScale(glm::mix(glm::vec3(100.f, 100.f, 0.f), glm::vec3(0.f, 0.f, 0.f), value));
    });
    tweenOut->SetDelay(delay);
    tweenOut->Start();
}

void PowerUp::Remove(bool force) {
    if (!force && StateManager::gameTime < collectedTime + duration) return;

    if (force) {
        HumanData* human = Game::GetHumanFromEntity(player->vehicleEntity);
        if (human) {
            Effects::Instance().DestroyTween("PowerUpTweenIn" + std::to_string(human->id));
            Effects::Instance().DestroyTween("PowerUpTweenOut" + std::to_string(human->id));
            
            Entity* guiRoot = human->camera->GetGuiRoot();
            Entity* guiEntity = EntityManager::FindFirstChild(guiRoot, GetGuiName());
            GuiComponent* gui = guiEntity->GetComponent<GuiComponent>();

            auto tweenOut = Effects::Instance().CreateTween<float, easing::Quint::easeOut>(gui->GetTextureOpacity(), 0.f, 0.25, StateManager::gameTime);
            tweenOut->SetUpdateCallback([gui](float& value) mutable {
                gui->SetTextureOpacity(value);
                gui->transform.SetScale(mix(glm::vec3(100.f, 100.f, 0.f), glm::vec3(0.f, 0.f, 0.f), value));
            });
            tweenOut->Start();
        }
    }

	if (!force) {
		RemoveInternal();
		std::vector<Entity*> headlights = EntityManager::FindChildren(player->vehicleEntity, "HeadLamp");
		for (Entity* entity : headlights) {
			SpotLightComponent* light = entity->GetComponent<SpotLightComponent>();
            MeshComponent* mesh = entity->GetComponent<MeshComponent>();
			
			const glm::vec3 start = light->GetColor();
			const glm::vec3 end = glm::vec3(1.f);
			auto tween = Effects::Instance().CreateTween<float, easing::Quint::easeOut>(0.f, 1.f, 2.0, StateManager::gameTime);
			tween->SetTag("Headlight" + std::to_string(player->id));
            PlayerData* thePlayer = player;
			tween->SetUpdateCallback([thePlayer, start, end, light, mesh](float& value) {
                if (!thePlayer->alive) return;
				light->SetColor(glm::mix(start, end, value));
                mesh->GetMaterial()->diffuseColor = glm::vec4(mix(mix(start, end, 0.25f), end, value), 1.f);
			});
			tween->Start();
		}
	}

    player->activePowerUp = nullptr;
    delete this;
}

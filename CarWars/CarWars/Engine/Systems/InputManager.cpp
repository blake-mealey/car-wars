#include "InputManager.h"

#include "Graphics.h"
#include "Game.h"
#include "../Systems/StateManager.h"
#include "../Entities/EntityManager.h"
#include "../Components/CameraComponent.h"
#include "../Components/RigidbodyComponents/VehicleComponent.h"

#include "PxPhysicsAPI.h"

#include "../Components/WeaponComponents/RailGunComponent.h"
#include "../Components/GuiComponents/GuiComponent.h"
#include "../Components/GuiComponents/GuiHelper.h"
#include "Content/ContentManager.h"

#include "../Systems/Physics/CollisionGroups.h"
#include "../Systems/Physics/RaycastGroups.h"
#include "../Systems/Audio.h"
#include "Effects.h"
#include "PennerEasing/Quint.h"
#include "PennerEasing/Quad.h"
#include "PennerEasing/Circ.h"
#include "PennerEasing/Expo.h"
#include "PennerEasing/Back.h"
#include <bitset>

vector<XboxController*> InputManager::xboxControllers;

InputManager &InputManager::Instance() {
	for (int i = 0; i < XUSER_MAX_COUNT; i++) {
		xboxControllers.push_back(new XboxController(i + 1));
	}

	static InputManager instance;
	return instance;
}

void InputManager::Update() {
	HandleMouse();
	HandleKeyboard();
	HandleController();
}


void InputManager::HandleMouse() {
	//Mouse Inputs
	//Get Graphics Instance
	Graphics& graphicsInstance = Graphics::Instance();

	if (StateManager::GetState() == GameState_Playing) {
		glfwSetInputMode(graphicsInstance.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		HumanData& player = Game::humanPlayers[0];
		if (!player.alive) return;
		VehicleComponent* vehicle = player.vehicleEntity->GetComponent<VehicleComponent>();
		WeaponComponent* weapon = player.vehicleEntity->GetComponent<WeaponComponent>();
		CameraComponent* cameraC = player.camera;

		//Shoot Weapon
		float rayLength = 100.0f;
		if (Mouse::ButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
			weapon->Charge();
		} else if (Mouse::ButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {
			PxScene* scene = &Physics::Instance().GetScene();
			glm::vec3 cameraDirection = glm::normalize(cameraC->GetTarget() - cameraC->GetPosition());

			float lowestDot = 999.0f;
			Entity* closestAimVehicle = nullptr;
			glm::vec3 cameraHit;
			for (Component* component : EntityManager::GetComponents(ComponentType_Vehicle)) {
				VehicleComponent* vehicleComponent = static_cast<VehicleComponent*>(component);
				Entity* vehicleEntity = vehicleComponent->GetEntity();
				if ((vehicleEntity->GetId() != vehicle->GetEntity()->GetId()) && (Game::GetPlayerFromEntity(vehicleEntity)->teamIndex != Game::GetPlayerFromEntity(vehicle->GetEntity())->teamIndex)) {
					glm::vec3 otherVehiclePos = vehicleEntity->transform.GetGlobalPosition();
					glm::vec3 dirToOtherVehicle = otherVehiclePos - vehicle->GetEntity()->transform.GetGlobalPosition();
					if (glm::length(dirToOtherVehicle) < 40.0f) {
						if (glm::dot(cameraDirection, glm::normalize(dirToOtherVehicle)) < lowestDot) {
							closestAimVehicle = vehicleEntity;
							lowestDot = glm::dot(cameraDirection, glm::normalize(dirToOtherVehicle));
						}
					}
				}
			}
			if (closestAimVehicle && acos(lowestDot) < (M_PI_4 / 16.0f)) {
				cameraHit = closestAimVehicle->transform.GetGlobalPosition();
			} else {
				PxQueryFilterData filterData;
				filterData.data.word0 = RaycastGroups::GetGroupsMask(vehicle->GetRaycastGroup());
				cameraHit = cameraC->CastRay(rayLength, filterData);
			}
			weapon->Shoot(cameraHit);
		} else if (Mouse::ButtonReleased(GLFW_MOUSE_BUTTON_LEFT)) {
			if(weapon->GetType() == ComponentType_RailGun)
				static_cast<RailGunComponent*>(weapon)->ChargeRelease();
		}

		if (Mouse::ButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
			cameraC->follow = !cameraC->follow;
		}

		//Cursor Inputs
		//glfwSetInputMode(graphicsInstance.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		//glfwSetInputMode(graphicsInstance.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glm::vec2 windowSize = graphicsInstance.GetWindowSize();
		double xPos, yPos;
		Mouse::GetCursorPosition(graphicsInstance.GetWindow(), &xPos, &yPos);

		//Get Camera Component
		glm::vec2 angleDiffs = 10.f * (windowSize*0.5f - glm::vec2(xPos, yPos)) / windowSize;
		angleDiffs.x = -angleDiffs.x;
		cameraC->UpdateCameraPosition(player.vehicleEntity, angleDiffs.x, angleDiffs.y);

		//Set Cursor to Middle
		glfwSetCursorPos(graphicsInstance.GetWindow(), windowSize.x / 2, windowSize.y / 2);
	} else {
		//Set Cursor Visible
		glfwSetInputMode(graphicsInstance.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void UpdateVehicleStats(const int playerIndex) {
    HumanData& player = Game::humanPlayers[playerIndex];
    Entity* stats = EntityManager::FindEntities("CharacterMenu_Stats")[playerIndex];
    EntityManager::DestroyChildren(stats);
    for (size_t i = 0; i < VehicleType::STAT_COUNT; ++i) {
        Entity* stat = ContentManager::LoadEntity("Menu/CharacterMenuStat.json", stats);
        GuiHelper::SetGuiPositions(stat, glm::vec3(-25.f, -10.f + i*20.f, 0.f));
        GuiHelper::SetFirstGuiText(stat, VehicleType::statDisplayNames[i]);
        GuiHelper::SetSecondGuiText(stat, VehicleType::statValues[player.vehicleType][i]);
    }
}

void UpdateWeaponStats(const int playerIndex) {
    HumanData& player = Game::humanPlayers[playerIndex];
    Entity* stats = EntityManager::FindEntities("CharacterMenu_Stats")[playerIndex];
    EntityManager::DestroyChildren(stats);
    for (size_t i = 0; i < WeaponType::STAT_COUNT; ++i) {
        Entity* stat = ContentManager::LoadEntity("Menu/CharacterMenuStat.json", stats);
        GuiHelper::SetGuiPositions(stat, glm::vec3(-25.f, -10.f + i*20.f, 0.f));
        GuiHelper::SetFirstGuiText(stat, WeaponType::statDisplayNames[i]);
        GuiHelper::SetSecondGuiText(stat, WeaponType::statValues[player.weaponType][i]);
    }
}

void CreateStartMenuOption(Entity* container, std::string optionName, std::string optionValue, float index, float count) {
    Entity* option = ContentManager::LoadEntity("Menu/MenuOption.json", container);
    GuiHelper::AddGuiPositions(option, glm::vec3(0.f, 70.f * (index - (count*0.5f)), 0.f));
    GuiHelper::SetFirstGuiText(option ,optionName);
    GuiHelper::SetSecondGuiText(option, optionValue);
    if (index == 0) GuiHelper::SetGuisSelected(option, true);
}

void CreateStartMenu() {
    Entity* optionsContainer = EntityManager::FindEntities("StartMenu_Options")[0];
    const size_t optionsCount = 7;
    CreateStartMenuOption(optionsContainer, "number of lives", to_string(Game::gameData.numberOfLives), 0, optionsCount);
    CreateStartMenuOption(optionsContainer, "number of ai", to_string(Game::gameData.aiCount), 1, optionsCount);
	CreateStartMenuOption(optionsContainer, "ai difficulty", to_string(Game::gameData.aiDifficulty), 2, optionsCount);
    CreateStartMenuOption(optionsContainer, "map", MapType::displayNames[Game::gameData.map], 3, optionsCount);
    CreateStartMenuOption(optionsContainer, "time limit", to_string(Game::gameData.timeLimitMinutes) + " minutes", 4, optionsCount);
    CreateStartMenuOption(optionsContainer, "kill limit", to_string(Game::gameData.killLimit), 5, optionsCount);
    CreateStartMenuOption(optionsContainer, "game mode", GameModeType::displayNames[Game::gameData.gameMode], 6, optionsCount);
}

void NextNumberOption(Entity* option, int dir, size_t &value, size_t min, size_t max, std::string unit = "") {
    if (!dir) return;
    dir = dir == 0 ? 0 : dir < 0 ? -1 : 1;
    if (value == min && dir < 0) value = max;
    else if (value == max && dir > 0) value = min;
    else value += dir;
    GuiHelper::SetSecondGuiText(option, to_string(value) + unit);

    GuiComponent* arrow = dir < 0 ? GuiHelper::GetThirdGui(option) : GuiHelper::GetFourthGui(option);
    GuiHelper::OpacityEffect(arrow, 0.2, 0.5f, 0.1, 0.1);
}

void NextEnumOption(Entity* option, int dir, size_t &value, size_t count, const std::string* displayNames) {
    if (!dir) return;
    dir = dir == 0 ? 0 : dir < 0 ? -1 : 1;
    if (value == 0 && dir < 0) value = count - 1;
    else if (value == count - 1 && dir > 0) value = 0;
    else value += dir;
    GuiHelper::SetSecondGuiText(option, displayNames[value]);

    GuiComponent* arrow = dir < 0 ? GuiHelper::GetThirdGui(option) : GuiHelper::GetFourthGui(option);
    GuiHelper::OpacityEffect(arrow, 0.2, 0.5f, 0.1, 0.1);
}

void UpdateLeaderboardMenu(Entity* leaderboard, int playerIndex) {
	Entity* container = EntityManager::FindFirstChild(leaderboard, "LeaderboardRows");
	std::vector<PlayerData> allPlayers;
	
	for (size_t i = 0; i < Game::gameData.humanCount; ++i) {
		allPlayers.push_back(Game::humanPlayers[i]);
	}
	for (AiData& ai : Game::ais) {
		allPlayers.push_back(ai);
	}

	std::sort(allPlayers.begin(), allPlayers.end());

    if (playerIndex > -1) {
        HumanData& thisPlayer = Game::humanPlayers[playerIndex];
        for (size_t i = 0; i < allPlayers.size(); ++i) {
            PlayerData& player = allPlayers[i];
            if (player.id != thisPlayer.id) continue;

            if (i > 9) allPlayers[9] = player;
            break;
        }
    }

	std::vector<Entity*> rows = EntityManager::GetChildren(container);
	for (size_t i = 0; i < rows.size(); ++i) {
		PlayerData& player = allPlayers[i];
		Entity* row = rows[i];
		GuiHelper::SetFirstGuiText(row, player.name);
		GuiHelper::SetSecondGuiText(row, to_string(player.killCount));
	}
}

void CreateLeaderboardMenu(Entity* leaderboard, int playerIndex) {
	Entity* container = EntityManager::FindFirstChild(leaderboard, "LeaderboardRows");

    if (EntityManager::GetChildren(container).size() > 0) return;

	size_t count = Game::gameData.humanCount + Game::gameData.aiCount;
	count = count < 10 ? count : 10;
	for (size_t i = 0; i < count; ++i) {
		Entity* row = ContentManager::LoadEntity("Menu/LeaderboardRow.json", container);
		GuiHelper::SetGuiPositions(row, glm::vec3(0.f, -85.f + (i * 30.f), 0.f));
	}

	UpdateLeaderboardMenu(leaderboard, playerIndex);
}

Entity* OpenMenu(int playerIndex, std::string menuName, std::string prefabName, glm::vec2 dims) {
    Entity* guiRoot;
    if (playerIndex < 0) {
        guiRoot = EntityManager::FindEntities("GuiRoot")[0];
    } else {
        guiRoot = Game::humanPlayers[playerIndex].camera->GetGuiRoot();
    }
    Entity* menu = EntityManager::FindFirstChild(guiRoot, menuName);

    Effects::Instance().DestroyTween(menuName + "Out" + to_string(playerIndex));
    Effects::Instance().DestroyTween(menuName + "OutS" + to_string(playerIndex));

    if (!menu) {
        menu = ContentManager::LoadEntity(prefabName, guiRoot);
        GuiHelper::SetOpacityRecursive(menu, 0.f);
    }

    GuiComponent* background = GuiHelper::GetFirstGui(menu);
    background->SetOpacity(1.f);

    auto tween = GuiHelper::TweenOpacityRecursive<easing::Quad::easeOut>(menu, 1.f, 0.1, StateManager::globalTime, { background });
    tween->SetDelay(0.05);
    tween->SetTag(menuName + "In" + to_string(playerIndex));
    tween->Start();

    const glm::vec3 start = background->transform.GetLocalScale();
    const glm::vec3 end = glm::vec3(dims.x, dims.y, 0);
    auto tween2 = Effects::Instance().CreateTween<glm::vec3, easing::Back::easeOut>(start, end, 0.4, StateManager::globalTime);
    tween2->SetUpdateCallback([background](glm::vec3& value) mutable {
        background->transform.SetScale(value);
    });
    tween2->SetTag(menuName + "InS" + to_string(playerIndex));
    tween2->Start();

    return menu;
}

void CloseMenu(int playerIndex, std::string menuName) {
    Entity* guiRoot = Game::humanPlayers[playerIndex].camera->GetGuiRoot();
    Entity* menu = EntityManager::FindFirstChild(guiRoot, menuName);
    if (!menu) return;

    Effects::Instance().DestroyTween(menuName + "In" + to_string(playerIndex));
    Effects::Instance().DestroyTween(menuName + "InS" + to_string(playerIndex));

    GuiComponent* background = GuiHelper::GetFirstGui(menu);

    auto tween = GuiHelper::TweenOpacityRecursive<easing::Quad::easeOut>(menu, 0.f, 0.1, StateManager::globalTime, { background });
    tween->SetTag(menuName + "Out" + to_string(playerIndex));
    tween->SetDelay(0.2);
    tween->Start();

    const glm::vec3 start = background->transform.GetLocalScale();
    const glm::vec3 end;
    auto tween2 = Effects::Instance().CreateTween<glm::vec3, easing::Back::easeIn>(start, end, 0.4, StateManager::globalTime);
    tween2->SetUpdateCallback([background](glm::vec3& value) mutable {
        background->transform.SetScale(value);
    });
    tween2->SetFinishedCallback([menu](glm::vec3& value) mutable {
        EntityManager::DestroyEntity(menu);
    });
    tween2->SetTag(menuName + "OutS" + to_string(playerIndex));
    tween2->Start();
}

void InputManager::NavigateGuis(GuiNavData navData) {
    // If there was no navigation, do nothing
    if (!navData.Valid()) return;
    
    if (StateManager::GetState() != GameState_Playing)
        if (navData.back)
            Audio::Instance().PlayAudio("Content/Sounds/menu/jig1.wav", 0.1f);
        else if (navData.enter)
            Audio::Instance().PlayAudio("Content/Sounds/menu/eshop.wav", 0.1f);
        else
            Audio::Instance().PlayAudio("Content/Sounds/menu/jig0.wav", 0.1f);


	// Normalize directional inputs
	navData.NormalizeInputs();

    // Get the player for the current controller
    HumanData& player = Game::humanPlayers[navData.playerIndex];
	const GameState gameState = StateManager::GetState();
	// Navigate buttons up/down
	if (navData.vertDir) {
		std::string buttonGroupName;
		bool noNavigation = false;
        bool entityNavigation = false;
		switch (gameState) {
		case GameState_Menu:
			buttonGroupName = "MainMenu_Buttons";
			break;
		case GameState_Menu_Start:
			buttonGroupName = "StartMenu_Options";
            entityNavigation = true;
			break;
		case GameState_Menu_Settings:
			buttonGroupName = "OptionsMenu_Buttons";
			break;
        case GameState_Paused:
            buttonGroupName = "PauseMenuButtons";
            break;
		default:
			noNavigation = true;
		}

		if (!noNavigation) {
			Entity* buttonGroup = EntityManager::FindEntities(buttonGroupName)[0];
            if (!entityNavigation) {
                GuiHelper::SelectNextGui(buttonGroup, -navData.vertDir);
            } else {
                GuiHelper::SelectNextEntity(buttonGroup, -navData.vertDir);
            }
		}
	}

	if (navData.horizDir) {
        if (gameState == GameState_Menu_Start) {
            Entity* selected = GuiHelper::GetSelectedEntity("StartMenu_Options");
            if (GuiHelper::FirstGuiContainsText(selected, "lives")) {
                NextNumberOption(selected, navData.horizDir, Game::gameData.numberOfLives, GameData::MIN_NUMBER_OF_LIVES, GameData::MAX_NUMBER_OF_LIVES);
            } else if (GuiHelper::FirstGuiContainsText(selected, "number of ai")) {
                NextNumberOption(selected, navData.horizDir, Game::gameData.aiCount, GameData::MIN_AI_COUNT, GameData::MAX_AI_COUNT);
            } else if (GuiHelper::FirstGuiContainsText(selected, "map")) {
                NextEnumOption(selected, navData.horizDir, Game::gameData.map, MapType::Count, MapType::displayNames);
            } else if (GuiHelper::FirstGuiContainsText(selected, "time")) {
                NextNumberOption(selected, navData.horizDir, Game::gameData.timeLimitMinutes, GameData::MIN_TIME_LIMIT_MINUTES, GameData::MAX_TIME_LIMIT_MINUTES, " minutes");
            } else if (GuiHelper::FirstGuiContainsText(selected, "kill")) {
                NextNumberOption(selected, navData.horizDir, Game::gameData.killLimit, GameData::MIN_KILL_LIMIT, GameData::MAX_KILL_LIMIT);
            } else if (GuiHelper::FirstGuiContainsText(selected, "game")) {
                NextEnumOption(selected, navData.horizDir, Game::gameData.gameMode, GameModeType::Count, GameModeType::displayNames);
			} else if (GuiHelper::FirstGuiContainsText(selected, "ai difficulty")) {
				NextNumberOption(selected, navData.horizDir, Game::gameData.aiDifficulty, GameData::MIN_AI_DIFFICULTY, GameData::MAX_AI_DIFFICULTY);
			}
        } else if (gameState == GameState_Menu_Start_CharacterSelect) {
            if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "vehicle", navData.playerIndex)) {
                player.vehicleType = (player.vehicleType + navData.horizDir) % VehicleType::Count;
                if (player.vehicleType < 0) player.vehicleType += VehicleType::Count;
                const string text = VehicleType::displayNames[player.vehicleType];
                GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", text, navData.playerIndex);

                GuiComponent* arrow = navData.horizDir < 0 ? GuiHelper::GetFirstGui("CharacterMenu_Arrows", navData.playerIndex) : GuiHelper::GetSecondGui("CharacterMenu_Arrows", navData.playerIndex);
                GuiHelper::OpacityEffect(arrow, 0.2, 0.5f, 0.1, 0.1);

                Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[navData.playerIndex];
                EntityManager::DestroyEntity(EntityManager::FindFirstChild(vehicleBox, "Vehicle"));
                ContentManager::LoadEntity(VehicleType::prefabPaths[player.vehicleType], vehicleBox);

                UpdateVehicleStats(navData.playerIndex);
            } else if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "weapon", navData.playerIndex)) {
				player.weaponType = (player.weaponType + navData.horizDir) % WeaponType::Count;
				if (player.weaponType < 0) player.weaponType += WeaponType::Count;
				const string text = WeaponType::displayNames[player.weaponType];
				GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", text, navData.playerIndex);

                GuiComponent* arrow = navData.horizDir < 0 ? GuiHelper::GetFirstGui("CharacterMenu_Arrows", navData.playerIndex) : GuiHelper::GetSecondGui("CharacterMenu_Arrows", navData.playerIndex);
                GuiHelper::OpacityEffect(arrow, 0.2, 0.5f, 0.1, 0.1);

                Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[navData.playerIndex];
                Entity* vehicle = EntityManager::FindFirstChild(vehicleBox, "Vehicle");
                EntityManager::DestroyEntity(EntityManager::FindFirstChild(vehicle, "GunTurret"));
                Entity* weapon = ContentManager::LoadEntity(WeaponType::turretPrefabPaths[player.weaponType], vehicle);
                weapon->transform.SetPosition(EntityManager::FindFirstChild(vehicle, "GunTurretBase")->transform.GetLocalPosition());
				
                UpdateWeaponStats(navData.playerIndex);
			}
		}
	}

	if (navData.enter) {
		if (gameState == GameState_Menu) {
			GuiComponent *selected = GuiHelper::GetSelectedGui("MainMenu_Buttons");
			if (selected->HasText("start")) {
				StateManager::SetState(GameState_Menu_Start);
                CreateStartMenu();
			} else if (selected->HasText("options")) {
				StateManager::SetState(GameState_Menu_Settings);
			} else if (selected->HasText("exit")) {
				StateManager::SetState(GameState_Exit);
			}
		}
		else if (gameState == GameState_Menu_Settings) {
			GuiComponent *selected = GuiHelper::GetSelectedGui("OptionsMenu_Buttons");
			if (selected->HasText("back")) {
				StateManager::SetState(GameState_Menu);
			}
		}
		else if (gameState == GameState_Menu_Start) {
			Entity* selected = GuiHelper::GetSelectedEntity("StartMenu_Options");
			if (GuiHelper::FirstGuiContainsText(selected, "back")) {
				StateManager::SetState(GameState_Menu);
			} else {
				StateManager::SetState(GameState_Menu_Start_CharacterSelect);
			}
		}
		else if (gameState == GameState_Menu_Start_CharacterSelect) {
			GuiComponent *selected = GuiHelper::GetSecondGui("GamepadJoinControl", navData.playerIndex);
			if (selected->ContainsText("join")) {
                Game::gameData.humanCount++;

				GuiHelper::SetGuisEnabled("CharacterMenu_Arrows", true, navData.playerIndex);
				GuiHelper::SetGuisEnabled("CharacterMenu_Stats", true, navData.playerIndex);
				GuiHelper::SetFirstGuiText("CharacterMenu_Title", "vehicle selection", navData.playerIndex);
				GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", VehicleType::displayNames[player.vehicleType], navData.playerIndex);

                Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[navData.playerIndex];
                ContentManager::LoadEntity(VehicleType::prefabPaths[player.vehicleType], vehicleBox);

                UpdateVehicleStats(navData.playerIndex);

				selected->SetText("continue");
			} else {
                if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "vehicle", navData.playerIndex)) {
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "weapon selection", navData.playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", WeaponType::displayNames[player.weaponType], navData.playerIndex);

                    Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[navData.playerIndex];
                    Entity* vehicle = EntityManager::FindFirstChild(vehicleBox, "Vehicle");
                    Entity* weapon = ContentManager::LoadEntity(WeaponType::turretPrefabPaths[player.weaponType], vehicle);
                    weapon->transform.SetPosition(EntityManager::FindFirstChild(vehicle, "GunTurretBase")->transform.GetLocalPosition());

                    UpdateWeaponStats(navData.playerIndex);
                } else {
                    GuiHelper::SetGuisEnabled("CharacterMenu_Arrows", false, navData.playerIndex);
					GuiHelper::SetGuisEnabled("CharacterMenu_Stats", false, navData.playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "", navData.playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", "", navData.playerIndex);
                    selected->SetText("Ready");

                    Entity* stats = EntityManager::FindEntities("CharacterMenu_Stats")[navData.playerIndex];
                    EntityManager::DestroyChildren(stats);

                    player.ready = true;
                    bool allReady = true;
                    for (int i = 0; i < Game::gameData.humanCount; ++i) {
                        allReady = Game::humanPlayers[i].ready;
                        if (!allReady) break;
                    }
                    // TODO: Countdown?
                    if (allReady) StateManager::SetState(GameState_Playing);
                }
			}
		} else if (gameState == GameState_Paused) {
            GuiComponent* selected = GuiHelper::GetSelectedGui("PauseMenuButtons", navData.playerIndex);
            if (selected->ContainsText("resume")) {
                StateManager::SetState(GameState_Playing);
                CloseMenu(navData.playerIndex, "PauseMenu");
            } else if (selected->ContainsText("exit")) {
                Game::Instance().FinishGame();
            }
		} else if (gameState == GameState_Menu_GameEnd) {
            GuiComponent* selected = GuiHelper::GetSelectedGui("Buttons");
            if (selected) {
                std::vector<Entity*> entities = EntityManager::FindEntities("LeaderboardMenu");
                if (entities.size() > 0) {
                    Game::Instance().ResetGame();
                    StateManager::SetState(GameState_Menu);
                } else {
                    GuiComponent* winnerTitle = GuiHelper::GetFirstGui("WinnerTitle");
                    
                    auto tween = Effects::Instance().CreateTween<float, easing::Expo::easeOut>(0.f, 1.f, 1.0, StateManager::globalTime);
					tween->SetTag("WinnerTitle");
                    tween->SetUpdateCallback([winnerTitle](float& value) mutable {
                        winnerTitle->SetFontSize(glm::mix(128.f, 64.f, value));
                        winnerTitle->SetScaledPosition(glm::mix(glm::vec2(0.5f, 0.5f), glm::vec2(0.5f, 0.f), value));
                        winnerTitle->transform.SetPosition(glm::mix(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 100.f, 0.f), value));
                    });
                    tween->Start();

                    Entity* leaderboard = OpenMenu(-1, "LeaderboardMenu", "Menu/LeaderboardMenu.json", glm::vec2(800.f, 550.f));
                    CreateLeaderboardMenu(leaderboard, -1);
                }
            }
		}
	}

	if (navData.back) {
		if (gameState == GameState_Menu_Settings) {
			StateManager::SetState(GameState_Menu);
		}
		else if (gameState == GameState_Menu_Start) {
			StateManager::SetState(GameState_Menu);
		}
		else if (gameState == GameState_Menu_Start_CharacterSelect) {
			GuiComponent *selected = GuiHelper::GetSecondGui("GamepadJoinControl", navData.playerIndex);
            if (selected->ContainsText("join")) {
                StateManager::SetState(GameState_Menu_Start);
                CreateStartMenu();
            } else {
                if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "vehicle", navData.playerIndex)) {
                    GuiHelper::SetGuisEnabled("CharacterMenu_Arrows", false, navData.playerIndex);
					GuiHelper::SetGuisEnabled("CharacterMenu_Stats", false, navData.playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "", navData.playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", "", navData.playerIndex);

                    Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[navData.playerIndex];
                    EntityManager::DestroyEntity(EntityManager::FindFirstChild(vehicleBox, "Vehicle"));

                    Entity* stats = EntityManager::FindEntities("CharacterMenu_Stats")[navData.playerIndex];
                    EntityManager::DestroyChildren(stats);

                    Game::gameData.humanCount--;
                    selected->SetText("join");
                } else if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "weapon", navData.playerIndex)) {
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "vehicle selection", navData.playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", VehicleType::displayNames[player.vehicleType], navData.playerIndex);

                    Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[navData.playerIndex];
                    Entity* vehicle = EntityManager::FindFirstChild(vehicleBox, "Vehicle");
                    EntityManager::DestroyEntity(EntityManager::FindFirstChild(vehicle, "GunTurret"));

                    UpdateVehicleStats(navData.playerIndex);
                } else {
                    GuiHelper::SetGuisEnabled("CharacterMenu_Arrows", true, navData.playerIndex);
					GuiHelper::SetGuisEnabled("CharacterMenu_Stats", true, navData.playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "weapon selection", navData.playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", WeaponType::displayNames[player.weaponType], navData.playerIndex);
                    selected->SetText("continue");

                    UpdateWeaponStats(navData.playerIndex);

                    player.ready = false;
                }
            }
		}
        else if (gameState == GameState_Paused) {
            StateManager::SetState(GameState_Playing);
            CloseMenu(navData.playerIndex, "PauseMenu");
        }
	}

    if (navData.escape) {
        if (gameState == GameState_Playing) {
            StateManager::SetState(GameState_Paused);
            OpenMenu(navData.playerIndex, "PauseMenu", "Menu/PauseMenu.json", glm::vec2(580.f, 370.f));
            if (navData.tabHeld) navData.tabReleased = true;
        } else if (gameState == GameState_Paused) {
            StateManager::SetState(GameState_Playing);
            CloseMenu(navData.playerIndex, "PauseMenu");
        }
    }

	if (gameState == GameState_Playing) {
		if (navData.tabPressed) {
            Entity* leaderboard = OpenMenu(navData.playerIndex, "LeaderboardMenu", "Menu/LeaderboardMenu.json", glm::vec2(800.f, 550.f));
            CreateLeaderboardMenu(leaderboard, navData.playerIndex);
		} else if (navData.tabReleased) {
            CloseMenu(navData.playerIndex, "LeaderboardMenu");
		} else if (navData.tabHeld) {
			Entity* guiRoot = Game::humanPlayers[navData.playerIndex].camera->GetGuiRoot();
			Entity* leaderboard = EntityManager::FindFirstChild(guiRoot, "LeaderboardMenu");
			if (leaderboard) {
				UpdateLeaderboardMenu(leaderboard, navData.playerIndex);
			}
		}
	}
}

void InputManager::HandleKeyboard() {
	//Keyboard Inputs
    const GameState gameState = StateManager::GetState();

	GuiNavData navData;
	navData.playerIndex = 0;

	navData.enter = Keyboard::KeyPressed(GLFW_KEY_ENTER);
	navData.back = Keyboard::KeyPressed(GLFW_KEY_ESCAPE);
		navData.escape = navData.back;
	navData.tabPressed = Keyboard::KeyPressed(GLFW_KEY_TAB);
	navData.tabHeld = Keyboard::KeyDown(GLFW_KEY_TAB);
	navData.tabReleased = Keyboard::KeyReleased(GLFW_KEY_TAB);
	
	navData.vertDir = Keyboard::KeyPressed(GLFW_KEY_UP) || Keyboard::KeyPressed(GLFW_KEY_W) ? 1
        : Keyboard::KeyPressed(GLFW_KEY_DOWN) || Keyboard::KeyPressed(GLFW_KEY_S) ? -1 : 0;
	navData.horizDir = Keyboard::KeyPressed(GLFW_KEY_RIGHT) || Keyboard::KeyPressed(GLFW_KEY_D) ? 1
        : Keyboard::KeyPressed(GLFW_KEY_LEFT) || Keyboard::KeyPressed(GLFW_KEY_A) ? -1 : 0;

    NavigateGuis(navData);

	if (gameState == GameState_Playing) {
		//Get Vehicle Component
		HumanData& player = Game::humanPlayers[0];
		if (!player.alive) return;

		VehicleComponent* vehicle = player.vehicleEntity->GetComponent<VehicleComponent>();
		WeaponComponent* weapon = player.vehicleEntity->GetComponent<WeaponComponent>();
		CameraComponent* cameraC = player.camera;

		float forwardPower = 0;
		float backwardPower = 0;
		glm::vec3 boostDir = glm::vec3();

		if (Keyboard::KeyDown(GLFW_KEY_W)) {
			forwardPower = 1;
			if (Keyboard::KeyDown(GLFW_KEY_SPACE)) {
				boostDir = boostDir - player.vehicleEntity->transform.GetUp();
			}
		}
		if (Keyboard::KeyDown(GLFW_KEY_S)) {
			backwardPower = 1;
			if (Keyboard::KeyDown(GLFW_KEY_SPACE)) {
				boostDir = boostDir + player.vehicleEntity->transform.GetUp();
			}
		}


		float steer = 0;
		if (Keyboard::KeyDown(GLFW_KEY_A)) { //Steer Left
			steer += 1;
			if (Keyboard::KeyDown(GLFW_KEY_SPACE)) {
				boostDir = boostDir - player.vehicleEntity->transform.GetRight();
			}
		}
		if (Keyboard::KeyDown(GLFW_KEY_D)) { //Steer Right
			steer += -1;
			if (Keyboard::KeyDown(GLFW_KEY_SPACE)) {
				boostDir = boostDir + player.vehicleEntity->transform.GetRight();
			}
		}

		float handbrake = 0;
		if (Keyboard::KeyDown(GLFW_KEY_SPACE)) {
			handbrake = 1;
		}

		vehicle->Boost(boostDir);
		vehicle->HandleAcceleration(forwardPower, backwardPower);
		vehicle->Handbrake(handbrake);
		vehicle->Steer(steer);
	}
}

void InputManager::HandleVehicleControllerInput(size_t controllerNum, int &leftVibrate, int &rightVibrate) {

	XboxController *controller = xboxControllers[controllerNum];

	if (controllerNum > Game::gameData.humanCount - 1) return;
	//if (!Game::humanPlayers[controllerNum].alive) return;

	bool active = controller->GetPreviousState().Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD ||
		controller->GetPreviousState().Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD ||
		abs(controller->GetPreviousState().Gamepad.sThumbLX) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
		abs(controller->GetPreviousState().Gamepad.sThumbLY) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
		abs(controller->GetPreviousState().Gamepad.sThumbRX) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
		abs(controller->GetPreviousState().Gamepad.sThumbRY) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
		controller->GetState().Gamepad.wButtons ||
		controller->GetState().Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD ||
		controller->GetState().Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD ||
		abs(controller->GetState().Gamepad.sThumbLX) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
		abs(controller->GetState().Gamepad.sThumbLY) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
		abs(controller->GetState().Gamepad.sThumbRX) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
		abs(controller->GetState().Gamepad.sThumbRY) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
		controller->GetPreviousState().Gamepad.wButtons;

	if (active) {
		// -------------------------------------------------------------------------------------------------------------- //
		// Get Components
		// -------------------------------------------------------------------------------------------------------------- //
		HumanData& player = Game::humanPlayers[controllerNum];
		if (!player.alive) return;
		VehicleComponent* vehicle = player.vehicleEntity->GetComponent<VehicleComponent>();
		WeaponComponent* weapon = player.vehicleEntity->GetComponent<WeaponComponent>();
		CameraComponent* cameraC = player.camera;

		// -------------------------------------------------------------------------------------------------------------- //
		// Get Button States
		// -------------------------------------------------------------------------------------------------------------- //
		int heldButtons = controller->GetState().Gamepad.wButtons & controller->GetPreviousState().Gamepad.wButtons;
		int pressedButtons = (controller->GetState().Gamepad.wButtons ^ controller->GetPreviousState().Gamepad.wButtons) & controller->GetState().Gamepad.wButtons;
		int releasedButtons = (controller->GetState().Gamepad.wButtons ^ controller->GetPreviousState().Gamepad.wButtons) & controller->GetPreviousState().Gamepad.wButtons;
		
		// -------------------------------------------------------------------------------------------------------------- //
		// Manage Speed
		// -------------------------------------------------------------------------------------------------------------- //
		float forwardPower = 0; 
		float backwardPower = 0; 
		if (abs(controller->GetState().Gamepad.bRightTrigger) >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
			forwardPower = controller->GetState().Gamepad.bRightTrigger / 255.f;
		}
		if (abs(controller->GetState().Gamepad.bLeftTrigger) >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
			backwardPower = controller->GetState().Gamepad.bLeftTrigger / 255.f;
		}

		// -------------------------------------------------------------------------------------------------------------- //
		// Manage Steering
		// -------------------------------------------------------------------------------------------------------------- //
		float steer = 0;
		if (abs(controller->GetState().Gamepad.sThumbLX) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
			steer = -controller->GetState().Gamepad.sThumbLX / 32768.0f;
		}

		// -------------------------------------------------------------------------------------------------------------- //
		// Manage Camera
		// -------------------------------------------------------------------------------------------------------------- //
		float cameraX = 0;
		float cameraY = 0;
	
		// an attempt to reset camera behind the vehicle
		if (pressedButtons & XINPUT_GAMEPAD_RIGHT_THUMB) {
			cameraC->follow = !cameraC->follow;
		}

		if (abs(controller->GetState().Gamepad.sThumbRX) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && abs(controller->GetPreviousState().Gamepad.sThumbRX) < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
			//TODO: add timer for player
		}

		//TODO: use timer and clamps to control speed
		if (abs(controller->GetState().Gamepad.sThumbRX) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
			cameraX += (static_cast<float>(controller->GetState().Gamepad.sThumbRX) / 32768.0f) * .4f; 
		}
		if (abs(controller->GetState().Gamepad.sThumbRY) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
			cameraY += (static_cast<float>(controller->GetState().Gamepad.sThumbRY) / 32768.0f) * .4f;
		}

		if (cameraX > M_PI) cameraX -= M_PI * 2;
		if (cameraX < -M_PI) cameraX += M_PI * 2;


		// -------------------------------------------------------------------------------------------------------------- //
		// Manage Boost
		// -------------------------------------------------------------------------------------------------------------- //
		glm::vec3 boostDir = glm::vec3();

		if (heldButtons & XINPUT_GAMEPAD_DPAD_LEFT) {
			boostDir = boostDir - player.vehicleEntity->transform.GetRight();
		}

		if (heldButtons & XINPUT_GAMEPAD_DPAD_RIGHT) {
			boostDir = boostDir + player.vehicleEntity->transform.GetRight();
		}

		if (heldButtons & XINPUT_GAMEPAD_DPAD_UP) {
			boostDir = boostDir - player.vehicleEntity->transform.GetUp();
		}

		if (heldButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
			boostDir = boostDir + player.vehicleEntity->transform.GetUp();
		}

		// -------------------------------------------------------------------------------------------------------------- //
		// Manage Handbrake
		// -------------------------------------------------------------------------------------------------------------- //
		//LEFT-SHOULDER
		float handbrake = 0;
		if (heldButtons & XINPUT_GAMEPAD_A) {
			handbrake = 1;
		}

		// -------------------------------------------------------------------------------------------------------------- //
		// Manage Shooting
		// -------------------------------------------------------------------------------------------------------------- //
		if (pressedButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) {
			weapon->Charge();
		}
		if (heldButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) {
			glm::vec3 cameraDirection = glm::normalize(cameraC->GetTarget() - cameraC->GetPosition());
			float lowestDot = 999.0f;
			Entity* closestAimVehicle = nullptr;
			glm::vec3 cameraHit;
			for (Component* component : EntityManager::GetComponents(ComponentType_Vehicle)) {
				VehicleComponent* vehicleComponent = static_cast<VehicleComponent*>(component);
				Entity* vehicleEntity = vehicleComponent->GetEntity();
				if ((vehicleEntity->GetId() != vehicle->GetEntity()->GetId()) && (Game::GetPlayerFromEntity(vehicleEntity)->teamIndex != Game::GetPlayerFromEntity(vehicle->GetEntity())->teamIndex)) {
					glm::vec3 otherVehiclePos = vehicleEntity->transform.GetGlobalPosition();
					glm::vec3 dirToOtherVehicle = otherVehiclePos - vehicle->GetEntity()->transform.GetGlobalPosition();
					if (glm::length(dirToOtherVehicle) < 150.0f) {
						if (glm::dot(cameraDirection, glm::normalize(dirToOtherVehicle)) < lowestDot) {
							closestAimVehicle = vehicleEntity;
							lowestDot = glm::dot(cameraDirection, glm::normalize(dirToOtherVehicle));
						}
					}
				}
			}
			if (closestAimVehicle && acos(lowestDot) < (6.0f / glm::length(closestAimVehicle->transform.GetGlobalPosition() - vehicle->GetEntity()->transform.GetGlobalPosition()))) {
				cameraHit = closestAimVehicle->transform.GetGlobalPosition();
			} else {
				PxQueryFilterData filterData;
				filterData.data.word0 = RaycastGroups::GetGroupsMask(vehicle->GetRaycastGroup());
				cameraHit = cameraC->CastRay(100.0f, filterData);
			}
			weapon->Shoot(cameraHit);
			vehicle->GetEntity()->GetComponent<WeaponComponent>()->Shoot(cameraHit);
	 	}
		if (releasedButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) {
			if (weapon->GetType() == ComponentType_RailGun) {
				static_cast<RailGunComponent*>(weapon)->ChargeRelease();
			}
		}
		
		// -------------------------------------------------------------------------------------------------------------- //
		// Update
		// -------------------------------------------------------------------------------------------------------------- //
		vehicle->Boost(boostDir);
		vehicle->HandleAcceleration(forwardPower, backwardPower);
		vehicle->Handbrake(handbrake);
		vehicle->Steer(steer);

		cameraC->UpdateCameraPosition(vehicle->GetEntity(), cameraX, cameraY);
	}
}

void InputManager::HandleController() {
	//Iterate through each controller
	for (auto it = xboxControllers.begin(); it != xboxControllers.end(); ++it) {
		XboxController *controller = *it;
        if (!controller->IsConnected()) continue;
        const size_t controllerNum = controller->GetControllerNumber();

        int leftVibrate = 0;
        int rightVibrate = 0;

		//Manage Button States
		int heldButtons = controller->GetState().Gamepad.wButtons & controller->GetPreviousState().Gamepad.wButtons;
		int pressedButtons = (controller->GetState().Gamepad.wButtons ^ controller->GetPreviousState().Gamepad.wButtons) & controller->GetState().Gamepad.wButtons;
		int releasedButtons = (controller->GetState().Gamepad.wButtons ^ controller->GetPreviousState().Gamepad.wButtons) & controller->GetPreviousState().Gamepad.wButtons;
	
		GuiNavData navData;
		navData.playerIndex = controllerNum;

		navData.enter = pressedButtons & XINPUT_GAMEPAD_A;
		navData.back = pressedButtons & XINPUT_GAMEPAD_B;
		navData.escape = pressedButtons & XINPUT_GAMEPAD_START;
		navData.tabPressed = pressedButtons & XINPUT_GAMEPAD_BACK;
		navData.tabHeld = heldButtons & XINPUT_GAMEPAD_BACK;
		navData.tabReleased = releasedButtons & XINPUT_GAMEPAD_BACK;
	    
		if (abs(controller->GetPreviousState().Gamepad.sThumbLY) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && abs(controller->GetState().Gamepad.sThumbLY) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
			navData.vertDir = controller->GetState().Gamepad.sThumbLY / abs(controller->GetState().Gamepad.sThumbLY);
		}
		navData.vertDir = pressedButtons & XINPUT_GAMEPAD_DPAD_DOWN ? -1 : pressedButtons & XINPUT_GAMEPAD_DPAD_UP ? 1 : navData.vertDir;

		if (abs(controller->GetPreviousState().Gamepad.sThumbLX) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && abs(controller->GetState().Gamepad.sThumbLX) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
			navData.horizDir = controller->GetState().Gamepad.sThumbLX / abs(controller->GetState().Gamepad.sThumbLX);
		}
		navData.horizDir = pressedButtons & XINPUT_GAMEPAD_DPAD_LEFT ? -1 : pressedButtons & XINPUT_GAMEPAD_DPAD_RIGHT ? 1 : navData.horizDir;
			
		NavigateGuis(navData);

        
	    if (StateManager::GetState() == GameState_Playing) {
            HandleVehicleControllerInput(controllerNum, leftVibrate, rightVibrate);
        }

		//TODO: Get vibrate

        //Vibrate Controller
        controller->Vibrate(leftVibrate, rightVibrate);

        //Update Previous Controller State
        controller->SetPreviousState(controller->GetState());
	}
}

size_t InputManager::GetControllerCount() {
    size_t count = 0;
    for (XboxController *controller : xboxControllers) {
        if (controller->IsConnected()) {        // TODO: Fix this function
            if (++count == 4) break;
        }
    }
    return 1;
    return count;
}

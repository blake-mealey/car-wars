#pragma once

#include <iostream>
#include <string>

#include "../Systems/Time.h"

/*
Manu Navigation

- Save Settings and Return
	Keyboard:
		(Enter Key)
	Controller:
		(Start Button)

- Return Without Saving
	Keyboard:
		(Escape Key)
	Controller:
		(B Button)

- Change Setting Values
	Keyboard:
		(Left Arrow) / (Right Arrow)
		(A Key) / (D Key)
	Controller:
		(Left Joystick Left) / (Left Joystick Right)
		(D-Pad Left) / (D-Pad Right)

-Change Between Settings
	Keyboard:
		(Up Arrow) / (Down Arrow)
		(W Key) / (S Key)
	Controller:
		(Left Joystick Up) / (Left Joystick Down)
		(D-Pad Up) / (D-Pad Down)
*/

enum GameState {
	//Main Menu Screen
	GameState_Menu = 0,
	/*
	Index:
	0: Start			(Goes to Game Options Screen)
	1: Settings			(Goes to Settings Pop-Up)
	2: Exit				(Closes the Game)
	*/

	//Game Options Screen
	GameState_Menu_Start,
	/*
	Index:
	0: Map				(Cylinder, Etc...)
	1: GameMode			(Team, FFA)
	2: Number AI		(1s)
	3: Number Lives		(1s)
	4: Kill Limit		(1s, 5s)
	5: Time Limit		(1m, 5m)
	*/

	//Settings Pop-Up
	GameState_Menu_Settings,
	/*
	Index:
	0: Music Volume		(5%)
	1: Sound Volume		(5%)
	2: Full Screen		(Yes / No)
	*/

	//Character Select Screen
	GameState_Menu_Start_CharacterSelect,
	/*
	Index:
	0: Vehicle			(Light, Medium, Heavy, Random)
	1: Color			(Red, Orange, Yellow, Green, Blue, Violet, Pink, White)
	2: Weapon			(Machine Gun, Rocket Launcher, Rail Gun, Random)
	3: Driver			(Driver 1, Driver 2, Driver 3, Random)

	(Either (A) to Ready, or (Start) to Continue)
	*/

	//Game Playing
	GameState_Playing,
	/*
	(Start)				(Goes to Pause Pop-Up)
	*/

	//Pause Pop-Up
	GameState_Paused,
	/*
	Index:
	0: Resume			(Goes to Game Playing)
	1: Options			(Goes to Settings Pop-Up)
	2: Exit				(Goes to Main Menu)
	*/
};

class StateManager {
public:
	//Manage States
	static GameState GetState();
	static void SetState(GameState state);
	static bool IsState(GameState state);

	static std::string GameStateToString();

	//Get/Set deltaTime and gameTime
	static Time deltaTime;
	static Time gameTime;

	//Indices for Menu Navigation
	static size_t menuIndex;
private:
	static GameState currentState;		//Current State in Game
};
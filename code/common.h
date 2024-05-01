#pragma once

#include <unordered_map>
#include <vector>

#define OLC_PGE_APPLICATION
#include "engine.h"

#define OLC_PGEX_NETWORK
#include "network.h"

enum class GameMsg : int
{
	Server_GetStatus,
	Server_GetPing,

	Client_Accepted,
	Client_AssignID,
	Client_RegisterWithServer,
	Client_UnregisterWithServer,

	Game_AddPlayer,
	Game_RemovePlayer,
	Game_UpdateGame,
};

struct sPlayerDescription
{
	int nUniqueID = 0;
	int nAvatarID = 0;
	int x_position = 3840;
	int y_position = 1260;
	int voted = 0;
	bool traitor = false;
};

struct sGameDescription
{
	std::unordered_map<int, sPlayerDescription> mapObjects;
	bool switches[15] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	bool voting = false;
	int traitor = 0;
	int player_count = 0;
	int votes_left = 5;
	int player_id[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};
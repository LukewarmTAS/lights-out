#include <iostream>
#include <unordered_map>

#include "common.h"

class Server : public olc::net::server_interface<GameMsg>
{
public:
	Server(uint16_t nPort) : olc::net::server_interface<GameMsg>(nPort)
	{
	}

	sGameDescription descGame;
	std::vector<uint32_t> m_vGarbageIDs;

protected:
	bool OnClientConnect(std::shared_ptr<olc::net::connection<GameMsg>> client) override
	{
		return true;
		std::cout << "Client Connected!\n";
	}

	void OnClientValidated(std::shared_ptr<olc::net::connection<GameMsg>> client) override
	{
		// Client passed validation check, so send them a message informing
		// them they can continue to communicate
		olc::net::message<GameMsg> msg;
		msg.header.id = GameMsg::Client_Accepted;
		client->Send(msg);
		std::cout << "Connection Validated\n";
	}

	void OnClientDisconnect(std::shared_ptr<olc::net::connection<GameMsg>> client) override
	{
		if (client)
		{
			if (descGame.mapObjects.find(client->GetID()) == descGame.mapObjects.end())
			{
				
			}
			else
			{
				auto& pd = descGame.mapObjects[client->GetID()];
				std::cout << "Disconnected:" + std::to_string(pd.nUniqueID) + "\n";
				descGame.mapObjects.erase(client->GetID());
				m_vGarbageIDs.push_back(client->GetID());
			}
		}

	}

	void OnMessage(std::shared_ptr<olc::net::connection<GameMsg>> client, olc::net::message<GameMsg>& msg) override
	{
		if (!m_vGarbageIDs.empty())
		{
			for (auto pid : m_vGarbageIDs)
			{
				olc::net::message<GameMsg> m;
				m.header.id = GameMsg::Game_RemovePlayer;
				m << pid;
				std::cout << "Disconnecting: " << pid << "\n";
				MessageAllClients(m);
			}
			m_vGarbageIDs.clear();
		}



		switch (msg.header.id)
		{
		case GameMsg::Client_RegisterWithServer:
		{
			sPlayerDescription desc;
			msg >> desc;
			desc.nUniqueID = client->GetID();
			descGame.mapObjects.insert_or_assign(desc.nUniqueID, desc);

			olc::net::message<GameMsg> msgSendID;
			msgSendID.header.id = GameMsg::Client_AssignID;
			msgSendID << desc.nUniqueID;
			MessageClient(client, msgSendID);

			olc::net::message<GameMsg> msgAddPlayer;
			msgAddPlayer.header.id = GameMsg::Game_AddPlayer;
			msgAddPlayer << desc;
			MessageAllClients(msgAddPlayer);

			for (const auto& player : descGame.mapObjects)
			{
				olc::net::message<GameMsg> msgAddOtherPlayers;
				msgAddOtherPlayers.header.id = GameMsg::Game_AddPlayer;
				msgAddOtherPlayers << player.second;
				MessageClient(client, msgAddOtherPlayers);
			}

			std::cout << "Connection Registered" << std::to_string(desc.nUniqueID) << "\n";
			break;
		}

		case GameMsg::Client_UnregisterWithServer:
		{
			break;
		}

		case GameMsg::Game_UpdateGame:
		{
			MessageAllClients(msg, client);
			break;
		}

		}

	}

};

int main()
{
	Server server(60000);
	server.Start();
	while (true)
	{
		server.Update(-1, true);
	}
	return 0;
}
#include <unordered_map>
#include <iostream>
#include <string>

#include "bitmap.h"
#include "common.h"
#include "engine.h"

std::string mapname = "assets/spookyhouse.bmp";
std::string playerfile = "assets/player.bmp";
bitmap_image map(mapname);
bitmap_image player(playerfile);
image_drawer switcher(map);
bool switched = false;
int x_offset = 3200;
int y_offset = 900;
int speed = 20;

int arguments[15][8] = {
	{980,  1620, 1400, 1460, 3360, 3680, 4000, 4380},
	{1820, 2240, 1960, 2040, 4060, 4140, 4520, 4680},
	{1480, 1940, 1660, 1720, 2520, 2680, 3100, 3280},
	{1080, 1420, 1200, 1280, 2140, 2240, 2520, 2620},
	{1460, 2440, 1880, 2040, 2000, 2020, 2180, 2220},
	{1700, 2120, 1840, 2000, 3360, 3380, 3620, 3660},
	{2520, 3020, 2740, 2820, 2140, 2240, 2540, 2660},
	{2200, 2520, 2280, 2340, 2760, 2900, 3280, 3520},
	{2380, 3080, 2640, 2800, 3600, 3680, 3900, 4080},
	{2820, 3100, 2880, 2940, 4240, 4420, 4760, 4960},
	{2460, 3060, 2700, 2760, 5140, 5320, 5500, 5700},
	{1800, 2420, 2040, 2100, 4740, 4880, 5180, 5300},
	{1380, 2300, 1700, 1780, 5380, 5420, 5640, 5700},
	{1500, 1760, 1580, 1660, 4460, 4660, 4960, 5120},
	{1100, 1460, 1220, 1300, 4660, 4700, 4980, 5020}
};

class Client : public olc::PixelGameEngine, olc::net::client_interface<GameMsg> {
	public:
		Client()
		{
			sAppName = "Lights Out";
		}
	private:
		uint32_t nPlayerID = 0;
		sGameDescription descGame;
		sPlayerDescription descPlayer;
		bool bWaitingForConnection = true;
		void toggle_pen(bool condition) {
			if (condition) {
				switcher.pen_color(255, 255, 255);
			} else {
				switcher.pen_color(127, 127, 127);
			}
		}
		void toggle_room(int room_id) {
			toggle_pen(descGame.switches[room_id]);
			for (int i = arguments[room_id][0]; i <= arguments[room_id][1]; i++) {
				if (i >= arguments[room_id][2] && i <= arguments[room_id][3]) {
					switcher.horiztonal_line_segment(arguments[room_id][4], arguments[room_id][5], i);
					switcher.horiztonal_line_segment(arguments[room_id][6], arguments[room_id][7], i);
				} else {
					switcher.horiztonal_line_segment(arguments[room_id][4], arguments[room_id][7], i);
				}
			}
		}
		void self_toggle_room(int room_id) {
			if ((x_offset + 640) >= arguments[room_id][4] && (x_offset + 640) <= arguments[room_id][7] && (y_offset + 360) >= arguments[room_id][0] && (y_offset + 360) <= arguments[room_id][1]) {
				descGame.switches[room_id] = !descGame.switches[room_id];
				toggle_room(room_id);
			}
		}
		void toggle_lights() {
			for (int id = 0; id < 15; id++) toggle_room(id);
		}
		void self_toggle_lights() {
			for (int id = 0; id < 15; id++) self_toggle_room(id);
		}
		bool not_black(int x_speed, int y_speed) {
			return !(!map.red_channel(x_offset + x_speed + 640, y_offset + y_speed + 360) && !map.green_channel(x_offset + x_speed + 640, y_offset + y_speed + 360) && !map.blue_channel(x_offset + x_speed + 640, y_offset + y_speed + 360));
		}
		bool OnUserCreate() override {
			return Connect("192.168.1.57", 60000);
		}
		bool OnUserUpdate(float fElapsedTime) override {
			if (GetKey(olc::Key::ESCAPE).bHeld) exit(0);
			if (IsConnected()) {
				while (!Incoming().empty()) {
					auto msg = Incoming().pop_front().msg;
					switch (msg.header.id) {
						case(GameMsg::Client_Accepted):
						{
							olc::net::message<GameMsg> msg;
							msg.header.id = GameMsg::Client_RegisterWithServer;
							msg << descPlayer;
							Send(msg);
							break;
						}
						case(GameMsg::Client_AssignID):
						{
							msg >> nPlayerID;
							for (int i = 0; i < 20; i++) {
								if (descGame.player_id[i] == 0) {
									descGame.player_id[i] = nPlayerID;
									break;
								}
							}
							break;
						}

						case(GameMsg::Game_AddPlayer):
						{
							sPlayerDescription desc;
							msg >> desc;
							descGame.mapObjects.insert_or_assign(desc.nUniqueID, desc);

							if (desc.nUniqueID == nPlayerID)
							{
								bWaitingForConnection = false;
							}
							break;
						}

						case(GameMsg::Game_RemovePlayer):
						{
							descGame.player_count--;
							int nRemovalID = 0;
							msg >> nRemovalID;
							descGame.mapObjects.erase(nRemovalID);
							int idx = 0;
							while (idx < 20) {
								if (descGame.player_id[idx] == nRemovalID) {
									descGame.player_id[idx] = 0;
									break;
								} ++idx;
							}
							while (idx < 20) {
								if (idx == 20) descGame.player_id[idx] = 0;
								else descGame.player_id[idx] = descGame.player_id[idx + 1];
								++idx;
							}
							break;
						}

						case(GameMsg::Game_UpdateGame):
						{
							msg >> descGame;
							break;
						}

					}
				}
			}
			if (bWaitingForConnection) {
				Clear(olc::BLACK);
				DrawString({ 10,10 }, "Waiting To Connect...", olc::WHITE);
				return true;
			}
			if (!GetKey(olc::Key::SPACE).bHeld && switched == true) switched = false;
			if (GetKey(olc::Key::W).bHeld && !GetKey(olc::Key::S).bHeld && not_black(0, -speed) && y_offset > speed - 1) y_offset -= speed;
			if (GetKey(olc::Key::A).bHeld && !GetKey(olc::Key::D).bHeld && not_black(-speed, 0) && x_offset > speed - 1) x_offset -= speed;
			if (GetKey(olc::Key::S).bHeld && !GetKey(olc::Key::W).bHeld && not_black(0,  speed) && y_offset < map.height() - (ScreenHeight() + speed)) y_offset += speed;
			if (GetKey(olc::Key::D).bHeld && !GetKey(olc::Key::A).bHeld && not_black(speed,  0) && x_offset < map.width() - (ScreenWidth() + speed)) x_offset += speed;
			if (GetKey(olc::Key::SPACE).bHeld && switched == false) {
				self_toggle_lights();
				switched = true;
			}
			toggle_lights();
			for (int x = 0; x < ScreenWidth(); x++) {
				for (int y = 0; y < ScreenHeight(); y++) {
					Draw(x, y, olc::Pixel(map.red_channel(x + x_offset, y + y_offset), map.green_channel(x + x_offset, y + y_offset), map.blue_channel(x + x_offset, y + y_offset)));
				}
			}
			descGame.mapObjects[nPlayerID].x_position = x_offset + 640;
			descGame.mapObjects[nPlayerID].y_position = y_offset + 360;
			for (int i = 0; i < 20; i++) {
				if (descGame.player_id[i] == nPlayerID) {
					for (int x = 0; x < 64; x++) {
						for (int y = 0; y < 64; y++) {
							if (!(player.red_channel(x, y) == 255 && player.green_channel(x, y) == 255 && player.blue_channel(x, y) == 255)) {
								Draw(x + 608, y + 328, olc::Pixel(player.red_channel(x, y), player.green_channel(x, y), player.blue_channel(x, y)));
							}
						}
					}
				} else if (descGame.player_id[i] != 0) {
					int x_edge = descGame.mapObjects[descGame.player_id[i]].x_position - x_offset - 32;
					int y_edge = descGame.mapObjects[descGame.player_id[i]].y_position - y_offset - 32;
					for (int x = 0; x < 64; x++) {
						for (int y = 0; y < 64; y++) {
							if (!(player.red_channel(x - 608, y - 328) == 255 && player.green_channel(x - 608, y - 328) == 255 && player.blue_channel(x - 608, y - 328) == 255)) {
								if ((x + x_edge) >= 0 && (x + x_edge) < ScreenWidth() && (y + y_edge) >= 0 && (y + y_edge) < ScreenHeight()) {
									Draw(x + x_edge, y + y_edge, olc::Pixel(player.red_channel(x - 608, y - 328), player.green_channel(x - 608, y - 328), player.blue_channel(x - 608, y - 328)));
								}
							}
						}
					}
				} else break;
			}
			olc::net::message<GameMsg> msg;
			msg.header.id = GameMsg::Game_UpdateGame;
			msg << descGame;
			Send(msg);
			return true;
		}
};

int main()
{
	Client client;
	if (client.Construct(1280, 720, 1, 1, 1, 1)) client.Start();
}
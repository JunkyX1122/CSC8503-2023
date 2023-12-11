#pragma once
#include "NetworkBase.h"
#include <stdint.h>
#include <thread>
#include <atomic>

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		class GameClient : public NetworkBase {
		public:
			GameClient(std::function<void(int)> onOtherPlayerConnect, std::function<void(int)> onOtherPlayerDisconnect);
			~GameClient();

			bool Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum);

			void SendPacket(GamePacket&  payload);

			void UpdateClient();

			void Disconnect();
		protected:	
			_ENetPeer*	netPeer;
			std::function<void(int)> otherPlayerConnect;
			std::function<void(int)> otherPlayerDisconnect;
		};
	}
}


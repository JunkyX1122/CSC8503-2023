#pragma once
#include "NetworkBase.h"

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class GameServer : public NetworkBase {
		public:
			GameServer(int onPort, int maxClients, std::function<void(int)> onPlayerConnect, std::function<void(int)> onPlayerDisconnect);
			~GameServer();

			bool Initialise();
			void Shutdown();

			void SetGameWorld(GameWorld &g);

			bool SendGlobalPacket(int msgID);
			bool SendGlobalPacket(GamePacket& packet);
			bool SendPacketToPeer(GamePacket& packet, int peerID);
			virtual void UpdateServer();
			
			int GetPeerCount();
		protected:
			int			port;
			int			clientMax;
			int			clientCount;
			GameWorld*	gameWorld;

			int incomingDataRate;
			int outgoingDataRate;

			std::function<void(int)> playerConnect;
			std::function<void(int)> playerDisconnect;
		};
	}
}

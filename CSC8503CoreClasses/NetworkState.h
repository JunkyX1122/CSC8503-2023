#pragma once

namespace NCL {
	using namespace Maths;
	namespace CSC8503 {
		//class GameObject;
		class NetworkState	{
		public:
			NetworkState();
			virtual ~NetworkState();

			bool		isEnabled;
			Vector3		position;
			Quaternion	orientation;
			int			stateID;
		};
	}
}


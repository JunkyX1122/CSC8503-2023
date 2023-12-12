#pragma once

const int LAYER_DEFAULT = 0;
const int LAYER_TERRAIN = 1;
const int LAYER_PLAYER = 2;
const int LAYER_ENEMY = 3;
const int LAYER_ITEM = 4;

namespace NCL {
	enum class VolumeType {
		AABB	= 1,
		OBB		= 2,
		Sphere	= 4, 
		Mesh	= 8,
		Capsule = 16,
		Compound= 32,
		Invalid = 256
	};

	class CollisionVolume
	{
	public:
		CollisionVolume() {
			type = VolumeType::Invalid;
		}
		~CollisionVolume() {}

		VolumeType type;
		int collisionLayer;
		bool isCollidable = true;

	};
}
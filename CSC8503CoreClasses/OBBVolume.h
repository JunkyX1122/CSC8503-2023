#pragma once
#include "CollisionVolume.h"

namespace NCL {
	class OBBVolume : CollisionVolume
	{
	public:
		OBBVolume(const Maths::Vector3& halfDims, int layer = LAYER_DEFAULT, bool isC = true) {
			type		= VolumeType::OBB;
			halfSizes	= halfDims;
			collisionLayer = layer;
			isCollidable = isC;
		}
		~OBBVolume() {}

		Maths::Vector3 GetHalfDimensions() const {
			return halfSizes;
		}
	protected:
		Maths::Vector3 halfSizes;
	};
}


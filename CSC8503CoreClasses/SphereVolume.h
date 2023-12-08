#pragma once
#include "CollisionVolume.h"

namespace NCL {
	class SphereVolume : CollisionVolume
	{
	public:
		SphereVolume(float sphereRadius = 1.0f, int layer = LAYER_DEFAULT, bool isC = true) {
			type	= VolumeType::Sphere;
			radius	= sphereRadius;
			collisionLayer = layer;
			isCollidable = isC;
		}
		~SphereVolume() {}

		float GetRadius() const {
			return radius;
		}
	protected:
		float	radius;
	};
}


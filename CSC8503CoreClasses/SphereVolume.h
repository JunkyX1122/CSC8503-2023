#pragma once
#include "CollisionVolume.h"

namespace NCL {
	class SphereVolume : CollisionVolume
	{
	public:
		SphereVolume(float sphereRadius = 1.0f, int layer = LAYER_DEFAULT) {
			type	= VolumeType::Sphere;
			radius	= sphereRadius;
			collisionLayer = layer;
		}
		~SphereVolume() {}

		float GetRadius() const {
			return radius;
		}
	protected:
		float	radius;
	};
}


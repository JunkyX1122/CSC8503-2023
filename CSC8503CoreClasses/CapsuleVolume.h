#pragma once
#include "CollisionVolume.h"
namespace NCL {
    class CapsuleVolume : public CollisionVolume
    {
    public:
        CapsuleVolume(float halfHeight, float radius, int layer = LAYER_DEFAULT, bool isC = true) {
            this->halfHeight    = halfHeight;
            this->radius        = radius;
            this->type          = VolumeType::Capsule;
            this->collisionLayer = layer;
            this->isCollidable = isC;
        };
        ~CapsuleVolume() {

        }
        float GetRadius() const {
            return radius;
        }

        float GetHalfHeight() const {
            return halfHeight;
        }
    protected:
        float radius;
        float halfHeight;
    };
}


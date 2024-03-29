#pragma once
#include "Transform.h"
#include "CollisionVolume.h"
#include "NavigationGrid.h"
#include "StateMachine.h"
using std::vector;

namespace NCL::CSC8503 {
	class NetworkObject;
	class RenderObject;
	class PhysicsObject;

	class GameObject	{
	public:
		GameObject(const std::string& name = "");
		~GameObject();

		void SetBoundingVolume(CollisionVolume* vol) {
			boundingVolume = vol;
		}

		const CollisionVolume* GetBoundingVolume() const {
			return boundingVolume;
		}

		bool IsActive() const {
			return isActive;
		}

		
		Transform& GetTransform() {
			return transform;
		}

		RenderObject* GetRenderObject() const {
			return renderObject;
		}

		PhysicsObject* GetPhysicsObject() const {
			return physicsObject;
		}

		NetworkObject* GetNetworkObject() const {
			return networkObject;
		}

		void SetRenderObject(RenderObject* newObject) {
			renderObject = newObject;
		}

		void SetPhysicsObject(PhysicsObject* newObject) {
			physicsObject = newObject;
		}

		const std::string& GetName() const {
			return name;
		}

		void SetName(std::string n)
		{
			name = n;
		}

		virtual void OnCollisionBegin(GameObject* otherObject) {
			//isColliding = true;
			//std::cout << "OnCollisionBegin event occured!\n";
		}

		virtual void OnCollisionEnd(GameObject* otherObject) {
			//isColliding = false;
			//std::cout << "OnCollisionEnd event occured!\n";
		}

		bool GetBroadphaseAABB(Vector3&outsize) const;

		void UpdateBroadphaseAABB();

		void SetWorldID(int newID) {
			worldID = newID;
		}

		int		GetWorldID() const {
			return worldID;
		}

		bool IsColliding() const {
			return isColliding;
		}

		void SetColliding(bool b)
		{
			isColliding = b;
		}
		void DrawHitbox();

		void AddToIgnoreList(GameObject* g)
		{
			ignoreList.push_back(g);
		}
		std::vector<GameObject*> GetObjectIgnoreList()
		{
			return ignoreList;
		}

	protected:
		Transform			transform;

		CollisionVolume*	boundingVolume;
		PhysicsObject*		physicsObject;
		RenderObject*		renderObject;
		NetworkObject*		networkObject;

		bool		isActive;
		int			worldID;
		bool isColliding;
		std::string	name;

		Vector3 broadphaseAABB;

		std::vector<GameObject*> ignoreList = std::vector<GameObject*>{};
	};

	class PlayerObject : public GameObject
	{
	public:
		void SetGrapplePoint(Vector3 v)
		{
			grapplePoint = v;
		}
		Vector3 GetGrapplePoint() { return grapplePoint; }
		bool IsGrappling() { return isGrappling; }
		void SetGrappling(bool b) { isGrappling = b; }
		Vector3 GetRespawnPoint() { return respawnPoint; }
		void SetRespawnPoint(Vector3 v) { respawnPoint = v; }
	protected:
		bool isGrappling;
		Vector3 grapplePoint;
		Vector3 respawnPoint;
	};
}


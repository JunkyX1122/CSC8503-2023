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
		void SetNetworkObject(NetworkObject* newObject)
		{
			networkObject = newObject;
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

		void SetPositionToDampenTo(Vector3 v) { positionToDampenTo = v; }
		Vector3 GetPositionToDampenTo() { return positionToDampenTo; }

		void ResetCollidingList()
		{
			isCollidingWith.clear();
		}
		void AddToCollidingList(GameObject* o)
		{
			isCollidingWith.push_back(o);
		}
		std::vector<GameObject*> GetCollidingList()
		{
			return isCollidingWith;
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
		Vector3 positionToDampenTo;
		std::vector<GameObject*> ignoreList = std::vector<GameObject*>{};
		std::vector<GameObject*> isCollidingWith = std::vector<GameObject*>{};
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
		float GetDashTimer() { return dashTimer; }
		void SetDashTimer(float i) { dashTimer = i; }
		float GetJumpTimer() { return jumpTimer; }
		void SetJumpTimer(float i) { jumpTimer = i; }
		int GetScore() { return score; }
		void SetScore(int i) { score = i; }
		std::map<int, GameObject*> GetItemsCollected() { return itemsHolding; }
		void SetItemsCollected(std::map<int, GameObject*> v) { itemsHolding = v; }
		void AddToItemCollected(int i, GameObject* g) { itemsHolding.insert({ i,g }); }
		void RemoveItemCollected(int i) { itemsHolding.erase(i); }
	protected:
		bool isGrappling;
		Vector3 grapplePoint;
		Vector3 respawnPoint;
		float dashTimer = 0;
		float jumpTimer = 0;
		std::map<int,GameObject*> itemsHolding = std::map<int, GameObject*>{};
		int score = 0;
	};

	class BonusObject : public GameObject
	{
	public:
		void SetCollectedBy(GameObject* g) { collectedBy = g; }
		GameObject* GetCollectedBy() { return collectedBy; }
		void SetItemID(int i) { itemID = i; }
		int GetItemID() { return itemID; }
	protected:
		GameObject* collectedBy = nullptr;
		int itemID = 0;
	};
}


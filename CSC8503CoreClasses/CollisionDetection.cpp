#include "CollisionDetection.h"
#include "CollisionVolume.h"
#include "AABBVolume.h"
#include "OBBVolume.h"
#include "SphereVolume.h"
#include "Window.h"
#include "Maths.h"
#include "Debug.h"

using namespace NCL;

bool CollisionDetection::RayPlaneIntersection(const Ray&r, const Plane&p, RayCollision& collisions) {
	float ln = Vector3::Dot(p.GetNormal(), r.GetDirection());

	if (ln == 0.0f) {
		return false; //direction vectors are perpendicular!
	}
	
	Vector3 planePoint = p.GetPointOnPlane();

	Vector3 pointDir = planePoint - r.GetPosition();

	float d = Vector3::Dot(pointDir, p.GetNormal()) / ln;

	collisions.collidedAt = r.GetPosition() + (r.GetDirection() * d);

	return true;
}

bool CollisionDetection::RayIntersection(const Ray& r,GameObject& object, RayCollision& collision) {
	bool hasCollided = false;

	const Transform& worldTransform = object.GetTransform();
	const CollisionVolume* volume	= object.GetBoundingVolume();

	if (!volume) {
		return false;
	}

	switch (volume->type) {
		case VolumeType::AABB:		hasCollided = RayAABBIntersection(r, worldTransform, (const AABBVolume&)*volume	, collision); break;
		case VolumeType::OBB:		hasCollided = RayOBBIntersection(r, worldTransform, (const OBBVolume&)*volume	, collision); break;
		case VolumeType::Sphere:	hasCollided = RaySphereIntersection(r, worldTransform, (const SphereVolume&)*volume	, collision); break;

		case VolumeType::Capsule:	hasCollided = RayCapsuleIntersection(r, worldTransform, (const CapsuleVolume&)*volume, collision); break;
	}

	return hasCollided;
}

bool CollisionDetection::RayBoxIntersection(const Ray& r, const Vector3& boxPos, const Vector3& boxSize, RayCollision& collision)
{
	//return true;
	Vector3 boxMin = boxPos - boxSize;
	Vector3 boxMax = boxPos + boxSize;

	Vector3 rayPos = r.GetPosition();
	Vector3 rayDir = r.GetDirection();

	Vector3 tVals(-1, -1, -1);

	for (int i = 0; i < 3; i++)
	{
		if (rayDir[i] > 0)
		{
			tVals[i] = (boxMin[i] - rayPos[i]) / rayDir[i];
		}
		else if (rayDir[i] < 0)
		{
			tVals[i] = (boxMax[i] - rayPos[i]) / rayDir[i];
		}
	}
	float bestT = tVals.GetMaxElement();
	if (bestT < 0.0f) return false;

	Vector3 intersection = rayPos + (rayDir * bestT);
	const float epsilon = 0.0001f;
	for (int i = 0; i < 3; ++i) 
	{
		if (intersection[i] + epsilon < boxMin[i] ||
			intersection[i] - epsilon > boxMax[i]) 
		{
			return false; 
		}
	}

	collision.collidedAt = intersection;
	collision.rayDistance = bestT;

	return true;
}

bool CollisionDetection::RayAABBIntersection(const Ray& r, const Transform& worldTransform, const AABBVolume& volume, RayCollision& collision)
{
	//return true;
	Vector3 boxPos = worldTransform.GetPosition();
	Vector3 boxSize = volume.GetHalfDimensions();

	return RayBoxIntersection(r, boxPos, boxSize, collision);
}

bool CollisionDetection::RayOBBIntersection(const Ray& r, const Transform& worldTransform, const OBBVolume& volume, RayCollision& collision)
{
	//return true;
	Quaternion orientation = worldTransform.GetOrientation();
	Vector3 position = worldTransform.GetPosition();

	Matrix3 transform = Matrix3(orientation);
	Matrix3 invTransform = Matrix3(orientation.Conjugate());

	Vector3 localRayPos = r.GetPosition() - position;

	Ray tempRay(invTransform * localRayPos, invTransform * r.GetDirection());

	bool collided = RayBoxIntersection(tempRay, Vector3(), volume.GetHalfDimensions(), collision);

	if (collided) collision.collidedAt = transform * collision.collidedAt + position;

	return collided;
}

bool CollisionDetection::RaySphereIntersection(const Ray& r, const Transform& worldTransform, const SphereVolume& volume, RayCollision& collision)
{
	//GitTest
	Vector3 spherePos = worldTransform.GetPosition();
	float sphereRadius = volume.GetRadius();

	//Debug::DrawLine(r.GetPosition(), r.GetDirection() * 1000.0f, Vector4(0, 0, 1, 1),100.0f);
	
	Vector3 dir = (spherePos - r.GetPosition());

	float sphereProj = Vector3::Dot(dir, r.GetDirection());
	if (sphereProj < 0.0f) return false;
	
	Vector3 point = r.GetPosition() + (r.GetDirection() * sphereProj);

	float sphereDist = (point - spherePos).Length();
	
	if (sphereDist > sphereRadius) return false;

	float offset = sqrt((sphereRadius * sphereRadius) - (sphereDist * sphereDist));

	collision.rayDistance = sphereProj - (offset);

	collision.collidedAt = r.GetPosition() + (r.GetDirection() * collision.rayDistance);
	return true;
}

bool CollisionDetection::RayCapsuleIntersection(const Ray& r, const Transform& worldTransform, const CapsuleVolume& volume, RayCollision& collision) 
{
	Vector3 capsuleCentre = worldTransform.GetPosition();
	Vector3 capsuleDir = GetCapsuleDirection(worldTransform);

	Vector3 capsuleBottom = capsuleCentre - capsuleDir * volume.GetHalfHeight() / 2;
	Vector3 capsuleTop = capsuleCentre + capsuleDir * volume.GetHalfHeight() / 2;
	Vector3 rayStart = r.GetPosition();
	Vector3 rayEnd = r.GetDirection() * 10000.0f;

	float closeCap = 0.0f;
	float closeRay = 0.0f;

	ClosestPointsTwoLines(&closeCap, &closeRay, capsuleBottom, capsuleTop, rayStart, rayEnd);

	Vector3 capsulePoint = capsuleBottom + (capsuleTop - capsuleBottom) * closeCap;
	Vector3 rayPoint = rayStart + (rayEnd - rayStart) * closeRay;
	
	Transform endTransform;
	endTransform.SetPosition(capsulePoint);

	//Debug::DrawSphereLines(capsulePoint, worldTransform.GetOrientation(), volume.GetRadius() / 2);
	bool collided = RaySphereIntersection(r, endTransform, volume.GetRadius()/2, collision);

	return collided;
}

void CollisionDetection::ClosestPointsTwoLines(float* ratio1, float* ratio2, Vector3 firstLineStart, Vector3 firstLineEnd, Vector3 secondLineStart, Vector3 secondLineEnd)
{
	//Debug::DrawLine(firstLineStart, firstLineEnd, Vector4(1,0,0,1));
	//Debug::DrawLine(secondLineStart, secondLineEnd, Vector4(1, 0, 0, 1));
	Vector3 firstLineVector = firstLineEnd - firstLineStart;
	Vector3 secondLineVector = secondLineEnd - secondLineStart;

	Vector3 SMinusF = secondLineStart - firstLineStart;

	float dotSS = Vector3::Dot(secondLineVector, secondLineVector);
	float dotFF = Vector3::Dot(firstLineVector, firstLineVector);
	float dotSF = Vector3::Dot(secondLineVector, firstLineVector);
	float dotSMF_F = Vector3::Dot(SMinusF, firstLineVector);
	float dotSMF_S = Vector3::Dot(SMinusF, secondLineVector);

	float dotSF2 = dotSF * dotSF;
	float dotSSdotFF = dotSS * dotFF;
	float denom = (dotSF2 - dotSSdotFF);


	if (denom == 0)
	{
		*ratio1 = 0.0f;
		*ratio2 = (dotFF * (*ratio1) - dotSMF_F) / dotSF;
	}
	else
	{
		*ratio1 = (dotSMF_S * dotSF - dotSS * dotSMF_F) / denom;
		*ratio2 = (-dotSMF_F * dotSF + dotFF * dotSMF_S) / denom;
	}

	*ratio1 = std::clamp((*ratio1), 0.0f, 1.0f);
	*ratio2 = std::clamp((*ratio2), 0.0f, 1.0f);
	//Debug::DrawLine(firstLineStart + firstLineVector * (*ratio1), secondLineStart + secondLineVector * (*ratio2), Vector4(0, 1, 0, 1));
}
void CollisionDetection::ClosestPointsPointLine(float* lineRatio, Vector3 point, Vector3 lineStart, Vector3 lineEnd)
{
	Vector3 heading = (lineEnd - lineStart);
	float magnitudeMax = heading.Length();
	heading.Normalise();

	Vector3 lhs = point - lineStart;
	float dotP = Vector3::Dot(lhs, heading) / magnitudeMax;


	*lineRatio = std::clamp((dotP), 0.0f, 1.0f);
}
Vector3 CollisionDetection::ClosestPointAABBPoint(Vector3 point, Vector3 AABBPos, Vector3 halfSizes)
{
	return Vector3(0,0,0);
}

bool CollisionDetection::ObjectIntersection(GameObject* a, GameObject* b, CollisionInfo& collisionInfo) {
	const CollisionVolume* volA = a->GetBoundingVolume();
	const CollisionVolume* volB = b->GetBoundingVolume();

	if (!volA || !volB) {
		return false;
	}

	collisionInfo.a = a;
	collisionInfo.b = b;

	Transform& transformA = a->GetTransform();
	Transform& transformB = b->GetTransform();

	VolumeType pairType = (VolumeType)((int)volA->type | (int)volB->type);

	//Two AABBs
	if (pairType == VolumeType::AABB) {
		return AABBIntersection((AABBVolume&)*volA, transformA, (AABBVolume&)*volB, transformB, collisionInfo);
	}
	//Two Spheres
	if (pairType == VolumeType::Sphere) {
		return SphereIntersection((SphereVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	//Two OBBs
	if (pairType == VolumeType::OBB) {
		return OBBIntersection((OBBVolume&)*volA, transformA, (OBBVolume&)*volB, transformB, collisionInfo);
	}
	//Two Capsules
	if (pairType == VolumeType::Capsule) {
		return CapsuleIntersection((CapsuleVolume&)*volA, transformA, (CapsuleVolume&)*volB, transformB, collisionInfo);
	}
	//AABB vs Sphere pairs
	if (volA->type == VolumeType::AABB && volB->type == VolumeType::Sphere) {
		return AABBSphereIntersection((AABBVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::AABB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return AABBSphereIntersection((AABBVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}

	//OBB vs sphere pairs
	if (volA->type == VolumeType::OBB && volB->type == VolumeType::Sphere) {
		return OBBSphereIntersection((OBBVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::OBB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return OBBSphereIntersection((OBBVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}
	//Capsule vs other interactions
	if (volA->type == VolumeType::Capsule && volB->type == VolumeType::Sphere) {
		return SphereCapsuleIntersection((CapsuleVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::Capsule) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return SphereCapsuleIntersection((CapsuleVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}

	if (volA->type == VolumeType::Capsule && volB->type == VolumeType::AABB) {
		return AABBCapsuleIntersection((CapsuleVolume&)*volA, transformA, (AABBVolume&)*volB, transformB, collisionInfo);
	}
	if (volB->type == VolumeType::Capsule && volA->type == VolumeType::AABB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return AABBCapsuleIntersection((CapsuleVolume&)*volB, transformB, (AABBVolume&)*volA, transformA, collisionInfo);
	}


	if (volA->type == VolumeType::Capsule && volB->type == VolumeType::OBB) {
		return OBBCapsuleIntersection((CapsuleVolume&)*volA, transformA, (OBBVolume&)*volB, transformB, collisionInfo);
	}
	if (volB->type == VolumeType::Capsule && volA->type == VolumeType::OBB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return OBBCapsuleIntersection((CapsuleVolume&)*volB, transformB, (OBBVolume&)*volA, transformA, collisionInfo);
	}

	return false;
}

bool CollisionDetection::AABBTest(const Vector3& posA, const Vector3& posB, const Vector3& halfSizeA, const Vector3& halfSizeB) {
	Vector3 delta = posB - posA;
	Vector3 totalSize = halfSizeA + halfSizeB;

	if (abs(delta.x) < totalSize.x &&
		abs(delta.y) < totalSize.y &&
		abs(delta.z) < totalSize.z) {
		return true;
	}
	return false;
}

//AABB/AABB Collisions
bool CollisionDetection::AABBIntersection(const AABBVolume& volumeA, const Transform& worldTransformA,
	const AABBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) 
{
	Vector3 boxAPos = worldTransformA.GetPosition();
	Vector3 boxBPos = worldTransformB.GetPosition();
	
	Vector3 boxASize = volumeA.GetHalfDimensions();
	Vector3 boxBSize = volumeB.GetHalfDimensions();
	
	bool overlap = AABBTest(boxAPos, boxBPos, boxASize, boxBSize);

	if (overlap)
	{
		static const Vector3 faces[6] =
		{
			Vector3(-1 , 0 , 0), Vector3(1 , 0 , 0),
			Vector3(0 , -1 , 0), Vector3(0 , 1 , 0),
			Vector3(0 , 0 , -1), Vector3(0 , 0 , 1),
		};

		Vector3 maxA = boxAPos + boxASize;
		Vector3 minA = boxAPos - boxASize;
		
		Vector3 maxB = boxBPos + boxBSize;
		Vector3 minB = boxBPos - boxBSize;

		float distances[6] =
		{
			(maxB.x - minA.x),
			(maxA.x - minB.x),
			(maxB.y - minA.y),
			(maxA.y - minB.y),
			(maxB.z - minA.z),
			(maxA.z - minB.z)
		};

		float penetration = FLT_MAX;
		Vector3 bestAxis;

		for (int i = 0; i < 6; i++)
		{
			if (distances[i] < penetration)
			{
				penetration = distances[i];
				bestAxis = faces[i];
			}
		}
		collisionInfo.AddContactPoint(Vector3(), Vector3(), bestAxis, penetration);
		return true;
	}
	return false;
}

//Sphere / Sphere Collision
bool CollisionDetection::SphereIntersection(const SphereVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) 
{
	float radii = volumeA.GetRadius() + volumeB.GetRadius();
	Vector3 delta = worldTransformB.GetPosition() - worldTransformA.GetPosition();
	//Debug::DrawSphereLines(worldTransformA.GetPosition(), worldTransformA.GetOrientation(), volumeA.GetRadius());
	//Debug::DrawSphereLines(worldTransformB.GetPosition(), worldTransformB.GetOrientation(), volumeB.GetRadius());
	float deltaLength = delta.Length();

	if (deltaLength <= radii)
	{
		float penetration = (radii - deltaLength);
		Vector3 normal = delta.Normalised();
		Vector3 localA = normal * volumeA.GetRadius();
		Vector3 localB = -normal * volumeB.GetRadius();

		collisionInfo.AddContactPoint(localA, localB, normal, penetration);
		return true;
	}
	return false;
}

//AABB - Sphere Collision
bool CollisionDetection::AABBSphereIntersection(const AABBVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) 
{
	
	Vector3 boxSize = volumeA.GetHalfDimensions();

	Vector3 delta = worldTransformB.GetPosition() - worldTransformA.GetPosition();

	Vector3 closestPointOnBox = Vector3::Clamp(delta, -boxSize, boxSize);

	Vector3 localPoint = delta - closestPointOnBox;
	float distance = localPoint.Length();

	if (distance <= volumeB.GetRadius())
	{
		Vector3 collisionNormal = localPoint.Normalised();
		float penetration = (volumeB.GetRadius() - distance);

		Vector3 localA = Vector3();
		Vector3 localB = -collisionNormal * volumeB.GetRadius();

		collisionInfo.AddContactPoint(localA, localB, collisionNormal, penetration);
		return true;
	}
	return false;
}

bool  CollisionDetection::OBBSphereIntersection(const OBBVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) 
{
	Quaternion orientation = worldTransformA.GetOrientation(); //OBB
	Vector3 position = worldTransformA.GetPosition(); //OBB

	Matrix3 transform = Matrix3(orientation);
	Matrix3 invTransform = Matrix3(orientation.Conjugate());

	Vector3 localSpherePos = invTransform * (worldTransformB.GetPosition() - position);
	Quaternion localSphereOri = invTransform * worldTransformB.GetOrientation();

	SphereVolume tempSphere(volumeB.GetRadius());
	Transform tempWorldSphereTransform = worldTransformB;
	tempWorldSphereTransform.SetPosition(localSpherePos + worldTransformA.GetPosition());
	tempWorldSphereTransform.SetOrientation(localSphereOri);


	AABBVolume tempCube(volumeA.GetHalfDimensions());
	Transform tempWorldCubeTransform = worldTransformA;

	bool collided = AABBSphereIntersection(tempCube, tempWorldCubeTransform, tempSphere, tempWorldSphereTransform, collisionInfo);
	//Debug::DrawLine(collisionInfo.point.localA, collisionInfo.point.localB, Vector4(1, 0, 0, 1));
	if (collided)
	{
		collisionInfo.point.normal = transform * collisionInfo.point.normal;
		collisionInfo.point.localB = transform * collisionInfo.point.localB;
		//Debug::DrawLine(collisionInfo.point.localA, collisionInfo.point.localB, Vector4(0, 0, 1, 1));
	}
	return collided;
}


//CAPSULE
bool CollisionDetection::CapsuleIntersection(
	const CapsuleVolume& volumeA, const Transform& worldTransformA,
	const CapsuleVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo)
{
	Vector3 capsuleCentreA = worldTransformA.GetPosition();
	Vector3 capsuleDirA = GetCapsuleDirection(worldTransformA);
	Vector3 capsuleCentreB = worldTransformB.GetPosition();
	Vector3 capsuleDirB = GetCapsuleDirection(worldTransformB);

	Vector3 capsuleBottomA = capsuleCentreA - capsuleDirA * volumeA.GetHalfHeight() / 2;
	Vector3 capsuleTopA = capsuleCentreA + capsuleDirA * volumeA.GetHalfHeight() / 2;

	Vector3 capsuleBottomB = capsuleCentreB - capsuleDirB * volumeB.GetHalfHeight() / 2;
	Vector3 capsuleTopB = capsuleCentreB + capsuleDirB * volumeB.GetHalfHeight() / 2;

	float closeCapA = 0.0f;
	float closeCapB = 0.0f;

	ClosestPointsTwoLines(&closeCapA, &closeCapB, capsuleBottomA, capsuleTopA, capsuleBottomB, capsuleTopB);
	float closeCapA_Hold = closeCapA;
	float closeCapB_Hold = closeCapB;
	Vector3 capsulePointA = capsuleBottomA + (capsuleTopA - capsuleBottomA) * closeCapA;
	Vector3 capsulePointB = capsuleBottomB + (capsuleTopB - capsuleBottomB) * closeCapB;
	float distTestBase = (capsulePointA - capsulePointB).Length();

	Vector3 testEndsA[2] = { capsuleBottomA, capsuleTopA };
	Vector3 testEndsB[2] = { capsuleBottomB, capsuleTopB };
	float testEndsRatio[2] = { 0.0f, 1.0f };
	for (int a = 0; a < 2; a++)
	{
		float testB_pointA = 0.0f;
		ClosestPointsPointLine(&testB_pointA, testEndsA[a], capsuleBottomB, capsuleTopB);
		Vector3 capsulePointB_Test = capsuleBottomB + (capsuleTopB - capsuleBottomB) * testB_pointA;
		if ((capsulePointB_Test - testEndsA[a]).Length() < distTestBase)
		{
			distTestBase = (capsulePointB_Test - testEndsA[a]).Length();
			closeCapB = testB_pointA;
			closeCapA = testEndsRatio[a];
		}
	}

	for (int b = 0; b < 2; b++)
	{
		float testA_pointB = 0.0f;
		ClosestPointsPointLine(&testA_pointB, testEndsB[b], capsuleBottomA, capsuleTopA);
		Vector3 capsulePointA_Test = capsuleBottomA + (capsuleTopA - capsuleBottomA) * testA_pointB;
		if ((capsulePointA_Test - testEndsB[b]).Length() < distTestBase)
		{
			distTestBase = (capsulePointA_Test - testEndsB[b]).Length();
			closeCapA = testA_pointB;
			closeCapB = testEndsRatio[b];
		}
	}
	capsulePointA = capsuleBottomA + (capsuleTopA - capsuleBottomA) * closeCapA;
	capsulePointB = capsuleBottomB + (capsuleTopB - capsuleBottomB) * closeCapB;
	//Debug::DrawLine(capsulePointA, capsulePointB);

	SphereVolume tempSphereA(volumeA.GetRadius() / 2); 
	Transform tempWorldTransformA = worldTransformA;
	tempWorldTransformA.SetPosition(capsulePointA);
	tempWorldTransformA.SetOrientation(worldTransformA.GetOrientation()); 

	SphereVolume tempSphereB(volumeB.GetRadius() / 2);
	Transform tempWorldTransformB = worldTransformB;
	tempWorldTransformB.SetPosition(capsulePointB);
	tempWorldTransformB.SetOrientation(worldTransformB.GetOrientation());

	bool collision = SphereIntersection(tempSphereA, tempWorldTransformA, tempSphereB, tempWorldTransformB, collisionInfo);
	return collision;
}


bool CollisionDetection::AABBCapsuleIntersection(
	const CapsuleVolume& volumeA, const Transform& worldTransformA,
	const AABBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) 
{
	//Debug::DrawLine(worldTransformB.GetPosition()+volumeB.GetHalfDimensions(), worldTransformB.GetPosition() - volumeB.GetHalfDimensions(), Vector4(1, 0, 1, 1));
	//Debug::DrawLine(worldTransformA.GetPosition(), worldTransformB.GetPosition(), Vector4(1, 0.5f, 0.5f, 1));
	Vector3 capsuleCentre = worldTransformA.GetPosition();
	Vector3 capsuleDir = GetCapsuleDirection(worldTransformA);
	capsuleDir = capsuleDir.Normalised();
	Vector3 capsuleBottom = capsuleCentre - capsuleDir * volumeA.GetHalfHeight() / 2;
	Vector3 capsuleTop = capsuleCentre + capsuleDir * volumeA.GetHalfHeight() / 2;
	//Debug::DrawLine(capsuleBottom, capsuleTop, Vector4(1, 1, 1, 1));
	//std::cout << capsuleDir << "\n";
	float maxDist = FLT_MAX;

	Vector3 testPoints[3] = {capsuleBottom, capsuleTop, capsuleCentre};
	Vector3 capsulePoint = Vector3(0,0,0);
	Vector3 closestPointOnBox = Vector3(0, 0, 0);
	for (int i = 0; i < 3; i++)
	{
		Vector3 boxSize = volumeB.GetHalfDimensions();
		Vector3 delta = testPoints[i] - worldTransformB.GetPosition();
		Vector3 closestPointOnBoxTest = worldTransformB.GetPosition() + Vector3::Clamp(delta, -boxSize, boxSize);
		//Debug::DrawLine(closestPointOnBoxTest, testPoints[i], Vector4(1, 1, 0.5f, 1));

		float closeCap = 0.0f;
		ClosestPointsPointLine(&closeCap, closestPointOnBoxTest, capsuleBottom, capsuleTop);
		Vector3 capsulePointTest = capsuleBottom + (capsuleTop - capsuleBottom) * closeCap;

		float distTest = (capsulePointTest - closestPointOnBoxTest).Length();
		if (distTest < maxDist)
		{
			capsulePoint = capsulePointTest;
			closestPointOnBox = closestPointOnBoxTest;
			maxDist = distTest;
		}

		//Debug::DrawLine(closestPointOnBoxTest, capsulePointTest, Vector4(1, 0.5f, 0.5f, 1));

	}
	SphereVolume tempSphere(volumeA.GetRadius() / 2);
	Transform tempWorldTransform = worldTransformA;
	tempWorldTransform.SetPosition(capsulePoint);
	tempWorldTransform.SetOrientation(worldTransformA.GetOrientation());
	//Debug::DrawLine(tempWorldTransform.GetPosition(), worldTransformB.GetPosition(), Vector4(1, 0.5f, 0.5f, 1));
	int maxLines = 0;
	for (int i = 0; i < maxLines; i++)
	{
		float pitch = (2 * PI) / 64 * i;
		float yaw = (2 * PI * 5) / 64 * i;
		float x = tempSphere.GetRadius() * cos(pitch) * cos(yaw);
		float y = tempSphere.GetRadius() * sin(yaw);
		float z = tempSphere.GetRadius() * sin(pitch) * cos(yaw);
		//Debug::DrawLine(capsulePoint, capsulePoint + Vector3(x, y, z), Vector4(0, 1, 0, 1));
	}

	Vector3 localPoint = closestPointOnBox - capsulePoint;
	float distance = localPoint.Length();

	if (distance <= volumeA.GetRadius()/2)
	{
		Vector3 collisionNormal = localPoint.Normalised();
		float penetration = (volumeA.GetRadius()/2 - distance);

		Vector3 localA = collisionNormal * volumeA.GetRadius()/2;
		Vector3 localB = Vector3();

		collisionInfo.AddContactPoint(localA, localB, collisionNormal, penetration);
		return true;
	}
	return false;
}

bool CollisionDetection::SphereCapsuleIntersection(
	const CapsuleVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) 
{
	Vector3 capsuleCentre = worldTransformA.GetPosition();
	Vector3 capsuleDir = GetCapsuleDirection(worldTransformA);

	Vector3 capsuleBottom = capsuleCentre - capsuleDir * volumeA.GetHalfHeight() / 2;
	Vector3 capsuleTop = capsuleCentre + capsuleDir * volumeA.GetHalfHeight() / 2;

	float closeCap = 0.0f;
	
	ClosestPointsPointLine(&closeCap, worldTransformB.GetPosition(), capsuleBottom, capsuleTop);

	Vector3 capsulePoint = capsuleBottom + (capsuleTop - capsuleBottom) * closeCap;

	SphereVolume tempSphere(volumeA.GetRadius()/2);
	Transform tempWorldTransform = worldTransformA;
	tempWorldTransform.SetPosition(capsulePoint);
	tempWorldTransform.SetOrientation(worldTransformA.GetOrientation());

	bool collision = SphereIntersection(tempSphere, tempWorldTransform, volumeB, worldTransformB, collisionInfo);
	return collision;
}

bool CollisionDetection::OBBCapsuleIntersection(
	const CapsuleVolume& volumeA, const Transform& worldTransformA,
	const OBBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo)
{
	//Debug::DrawLine(worldTransformA.GetPosition(), worldTransformB.GetPosition(), Vector4(0, 0, 1, 1));

	Quaternion orientation = worldTransformB.GetOrientation(); //OBB
	//std::cout << orientation.ToEuler() << "\n";
	Vector3 position = worldTransformB.GetPosition(); //OBB

	Matrix3 transform = Matrix3(orientation);
	Matrix3 invTransform = Matrix3(orientation.Conjugate());

	//std::cout << invTransform << "\n";
	Vector3 localCapsulePos = invTransform * (worldTransformA.GetPosition());
	Vector3 localCapsuleAng = invTransform * (worldTransformA.GetOrientation()).ToEuler();
	Quaternion localCapsuleOri = Quaternion::EulerAnglesToQuaternion(localCapsuleAng.x, localCapsuleAng.y, localCapsuleAng.z);
	//std::cout << localCapsuleOri.ToEuler() << "\n";

	CapsuleVolume tempCapsule(volumeA.GetHalfHeight(),volumeA.GetRadius());
	Transform tempWorldCapsuleTransform;
	tempWorldCapsuleTransform.SetPosition(localCapsulePos);
	tempWorldCapsuleTransform.SetOrientation(localCapsuleOri);
	/*
	std::cout << orientation.ToEuler() << "\n";
	std::cout << tempWorldCapsuleTransform.GetOrientation().ToEuler() << "\n";
	std::cout << worldTransformA.GetOrientation().ToEuler() << "\n";
	std::cout << worldTransformA.GetMatrix() << "\n";
	std::cout << tempWorldCapsuleTransform.GetMatrix() << "\n";
	*/
	//std::cout << GetCapsuleDirection(worldTransformA) << "\n";
	//std::cout << GetCapsuleDirection(tempWorldCapsuleTransform ) << "\n";

	AABBVolume tempCube(volumeB.GetHalfDimensions());
	Transform tempWorldCubeTransform = worldTransformB;

	//Debug::DrawLine(tempWorldCapsuleTransform.GetPosition(), tempWorldCubeTransform.GetPosition(), Vector4(0, 0, 1, 1));

	bool collided = AABBCapsuleIntersection(tempCapsule, tempWorldCapsuleTransform, tempCube, tempWorldCubeTransform, collisionInfo);
	//Debug::DrawLine(collisionInfo.point.localA, collisionInfo.point.localB, Vector4(1, 0, 0, 1));
	if (collided)
	{
		collisionInfo.point.localA = transform * collisionInfo.point.localA;
		collisionInfo.point.localB = transform * collisionInfo.point.localB;
		collisionInfo.point.normal = transform * collisionInfo.point.normal;
		//Debug::DrawLine(collisionInfo.point.localA, collisionInfo.point.localB, Vector4(1, 0, 1, 1));
	}
	
	return collided;
}




bool CollisionDetection::OBBIntersection(const OBBVolume& volumeA, const Transform& worldTransformA,
	const OBBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
	return false;
}

Vector3 CollisionDetection::OBBSupport(const Transform& worldTransform, Vector3 worldDir) 
{
	Vector3 localDir = worldTransform.GetOrientation().Conjugate() * worldDir;
	Vector3 vertex;
	vertex.x = localDir.x < 0 ? -0.5f : 0.5f;
	vertex.y = localDir.y < 0 ? -0.5f : 0.5f;
	vertex.z = localDir.z < 0 ? -0.5f : 0.5f;

	return worldTransform.GetMatrix() * vertex;
}

Matrix4 GenerateInverseView(const Camera &c) {
	float pitch = c.GetPitch();
	float yaw	= c.GetYaw();
	Vector3 position = c.GetPosition();

	Matrix4 iview =
		Matrix4::Translation(position) *
		Matrix4::Rotation(-yaw, Vector3(0, -1, 0)) *
		Matrix4::Rotation(-pitch, Vector3(-1, 0, 0));

	return iview;
}

Matrix4 GenerateInverseProjection(float aspect, float fov, float nearPlane, float farPlane) {
	float negDepth = nearPlane - farPlane;

	float invNegDepth = negDepth / (2 * (farPlane * nearPlane));

	Matrix4 m;

	float h = 1.0f / tan(fov*PI_OVER_360);

	m.array[0][0] = aspect / h;
	m.array[1][1] = tan(fov * PI_OVER_360);
	m.array[2][2] = 0.0f;

	m.array[2][3] = invNegDepth;//// +PI_OVER_360;
	m.array[3][2] = -1.0f;
	m.array[3][3] = (0.5f / nearPlane) + (0.5f / farPlane);

	return m;
}

Vector3 CollisionDetection::Unproject(const Vector3& screenPos, const PerspectiveCamera& cam) {
	Vector2i screenSize = Window::GetWindow()->GetScreenSize();

	float aspect = Window::GetWindow()->GetScreenAspect();
	float fov		= cam.GetFieldOfVision();
	float nearPlane = cam.GetNearPlane();
	float farPlane  = cam.GetFarPlane();

	//Create our inverted matrix! Note how that to get a correct inverse matrix,
	//the order of matrices used to form it are inverted, too.
	Matrix4 invVP = GenerateInverseView(cam) * GenerateInverseProjection(aspect, fov, nearPlane, farPlane);

	Matrix4 proj  = cam.BuildProjectionMatrix(aspect);

	//Our mouse position x and y values are in 0 to screen dimensions range,
	//so we need to turn them into the -1 to 1 axis range of clip space.
	//We can do that by dividing the mouse values by the width and height of the
	//screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0 to 2.0)
	//and then subtracting 1 (-1.0 to 1.0).
	Vector4 clipSpace = Vector4(
		(screenPos.x / (float)screenSize.x) * 2.0f - 1.0f,
		(screenPos.y / (float)screenSize.y) * 2.0f - 1.0f,
		(screenPos.z),
		1.0f
	);

	//Then, we multiply our clipspace coordinate by our inverted matrix
	Vector4 transformed = invVP * clipSpace;

	//our transformed w coordinate is now the 'inverse' perspective divide, so
	//we can reconstruct the final world space by dividing x,y,and z by w.
	return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}

Ray CollisionDetection::BuildRayFromMouse(const PerspectiveCamera& cam) {
	Vector2 screenMouse = Window::GetMouse()->GetAbsolutePosition();
	Vector2i screenSize = Window::GetWindow()->GetScreenSize();

	//We remove the y axis mouse position from height as OpenGL is 'upside down',
	//and thinks the bottom left is the origin, instead of the top left!
	Vector3 nearPos = Vector3(screenMouse.x,
		screenSize.y - screenMouse.y,
		-0.99999f
	);

	//We also don't use exactly 1.0 (the normalised 'end' of the far plane) as this
	//causes the unproject function to go a bit weird. 
	Vector3 farPos = Vector3(screenMouse.x,
		screenSize.y - screenMouse.y,
		0.99999f
	);

	Vector3 a = Unproject(nearPos, cam);
	Vector3 b = Unproject(farPos, cam);
	Vector3 c = b - a;

	c.Normalise();

	return Ray(cam.GetPosition(), c);
}

Ray CollisionDetection::BuildRayFromCentre(const PerspectiveCamera& cam) {
	
	Vector2i screenSize = Window::GetWindow()->GetScreenSize();
	Vector2 screenMouse = Vector2(screenSize.x/2, screenSize.y/2);
	//We remove the y axis mouse position from height as OpenGL is 'upside down',
	//and thinks the bottom left is the origin, instead of the top left!
	Vector3 nearPos = Vector3(screenMouse.x,
		screenSize.y - screenMouse.y,
		-0.99999f
	);

	//We also don't use exactly 1.0 (the normalised 'end' of the far plane) as this
	//causes the unproject function to go a bit weird. 
	Vector3 farPos = Vector3(screenMouse.x,
		screenSize.y - screenMouse.y,
		0.99999f
	);

	Vector3 a = Unproject(nearPos, cam);
	Vector3 b = Unproject(farPos, cam);
	Vector3 c = b - a;

	c.Normalise();

	return Ray(cam.GetPosition(), c);
}

//http://bookofhook.com/mousepick.pdf
Matrix4 CollisionDetection::GenerateInverseProjection(float aspect, float fov, float nearPlane, float farPlane) {
	Matrix4 m;

	float t = tan(fov*PI_OVER_360);

	float neg_depth = nearPlane - farPlane;

	const float h = 1.0f / t;

	float c = (farPlane + nearPlane) / neg_depth;
	float e = -1.0f;
	float d = 2.0f*(nearPlane*farPlane) / neg_depth;

	m.array[0][0] = aspect / h;
	m.array[1][1] = tan(fov * PI_OVER_360);
	m.array[2][2] = 0.0f;

	m.array[2][3] = 1.0f / d;

	m.array[3][2] = 1.0f / e;
	m.array[3][3] = -c / (d * e);

	return m;
}

/*
And here's how we generate an inverse view matrix. It's pretty much
an exact inversion of the BuildViewMatrix function of the Camera class!
*/
Matrix4 CollisionDetection::GenerateInverseView(const Camera &c) {
	float pitch = c.GetPitch();
	float yaw	= c.GetYaw();
	Vector3 position = c.GetPosition();

	Matrix4 iview =
		Matrix4::Translation(position) *
		Matrix4::Rotation(yaw, Vector3(0, 1, 0)) *
		Matrix4::Rotation(pitch, Vector3(1, 0, 0));

	return iview;
}


/*
If you've read through the Deferred Rendering tutorial you should have a pretty
good idea what this function does. It takes a 2D position, such as the mouse
position, and 'unprojects' it, to generate a 3D world space position for it.

Just as we turn a world space position into a clip space position by multiplying
it by the model, view, and projection matrices, we can turn a clip space
position back to a 3D position by multiply it by the INVERSE of the
view projection matrix (the model matrix has already been assumed to have
'transformed' the 2D point). As has been mentioned a few times, inverting a
matrix is not a nice operation, either to understand or code. But! We can cheat
the inversion process again, just like we do when we create a view matrix using
the camera.

So, to form the inverted matrix, we need the aspect and fov used to create the
projection matrix of our scene, and the camera used to form the view matrix.

*/
Vector3	CollisionDetection::UnprojectScreenPosition(Vector3 position, float aspect, float fov, const PerspectiveCamera& c) {
	//Create our inverted matrix! Note how that to get a correct inverse matrix,
	//the order of matrices used to form it are inverted, too.
	Matrix4 invVP = GenerateInverseView(c) * GenerateInverseProjection(aspect, fov, c.GetNearPlane(), c.GetFarPlane());


	Vector2i screenSize = Window::GetWindow()->GetScreenSize();

	//Our mouse position x and y values are in 0 to screen dimensions range,
	//so we need to turn them into the -1 to 1 axis range of clip space.
	//We can do that by dividing the mouse values by the width and height of the
	//screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0 to 2.0)
	//and then subtracting 1 (-1.0 to 1.0).
	Vector4 clipSpace = Vector4(
		(position.x / (float)screenSize.x) * 2.0f - 1.0f,
		(position.y / (float)screenSize.y) * 2.0f - 1.0f,
		(position.z) - 1.0f,
		1.0f
	);

	//Then, we multiply our clipspace coordinate by our inverted matrix
	Vector4 transformed = invVP * clipSpace;

	//our transformed w coordinate is now the 'inverse' perspective divide, so
	//we can reconstruct the final world space by dividing x,y,and z by w.
	return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}


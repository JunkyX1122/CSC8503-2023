#include "CourseworkGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"

#include "Assets.h"
#include <fstream>

using namespace NCL;
using namespace CSC8503;

CourseworkGame::CourseworkGame() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	useGravity		= false;
	inSelectionMode = false;
	
	testStateObject = nullptr;
	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void CourseworkGame::InitialiseAssets() {
	cubeMesh	= renderer->LoadMesh("cube.msh");
	sphereMesh	= renderer->LoadMesh("sphere.msh");
	charMesh	= renderer->LoadMesh("goat.msh");
	enemyMesh	= renderer->LoadMesh("Keeper.msh");
	bonusMesh	= renderer->LoadMesh("apple.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex	= renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	InitWorld();
}

CourseworkGame::~CourseworkGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void CourseworkGame::UpdateGame(float dt) {
	
	if (!inSelectionMode) {
		world->GetMainCamera().UpdateCamera(dt);
	}
	
	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
	}

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}

	//Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));
	if (!inPlayerMode)
	{
		SelectObject();
		MoveSelectedObject();
	}
	else
	{
		AttachCameraPlayer();
		MovePlayerObject(dt);
	}
	UpdateKeys();
	
	if (testStateObject)
	{
		Debug::DrawLine(Vector3(0, 0, 0), testStateObject->GetTransform().GetPosition(), Vector4(0, 0, 1, 1));
		testStateObject->Update(dt);
	}
	Vector3 speed = playerObject->GetPhysicsObject()->GetLinearVelocity();
	//Debug::Print("PlayerSpeed:" + std::to_string(speed.x) + " " + std::to_string(speed.y) + " " + std::to_string(speed.z) + "\n", Vector2(5, 50));

	UpdatePathFindings(dt);
	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);
	renderer->Render();

	

	Debug::UpdateRenderables(dt);
}

void CourseworkGame::UpdatePathFindings(float dt)
{
	for (EnemyObject* enemy : enemyObjects)
	{

		if(enemy->GetStateMachine()) enemy->GetStateMachine()->Update(dt);
		/*
		if (enemy->IsNavigationSet())
		{
			Vector3 target = playerObject->GetTransform().GetPosition();
			target.y = 0;
			//Debug::Print("PlayerPos:" + std::to_string(target.x) + " " + std::to_string(target.y) + " " + std::to_string(target.z) + "\n", Vector2(5, 50));
			enemy->SetTargetDestination(target);
			//target = enemy->GetTargetDestination();
			//Debug::Print("Target:" + std::to_string(target.x) + " " + std::to_string(target.y) + " " + std::to_string(target.z) + "\n", Vector2(5, 60));
			enemy->FindPath(enemy->GetTargetDestination());
			enemy->DrawNavigationPath();

			Vector3 direction = (enemy->GetNextPathNode() - enemy->GetTransform().GetPosition()).Normalised();
			enemy->GetPhysicsObject()->AddForce(direction * 6.0f * dt);
		}
		*/
	}
}

void CourseworkGame::UpdateKeys() 
{
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::P)) {
		inPlayerMode = !inPlayerMode;
	}
	
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
		world->ShuffleObjects(false);
	}
	
	if (lockedObject) {
		//LockedObjectMovement();
	}
	else {
		//DebugObjectMovement();
	}
}

void CourseworkGame::LockedObjectMovement() {
	Matrix4 view = world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();


	if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::SPACE)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
	}
}

void CourseworkGame::AttachCameraPlayer()
{
	Vector3 objPos = playerObject->GetTransform().GetPosition();

	Vector3 camOrientation = playerCameraRotation->ToEuler();
	float pitch = camOrientation.x;
	float yaw = camOrientation.y;

	pitch	-= controller.GetNamedAxis("YLook");
	yaw -= controller.GetNamedAxis("XLook");

	pitch = std::min(pitch, 90.0f);
	pitch = std::max(pitch, -90.0f);

	if (yaw < 0) {
		yaw += 360.0f;
	}
	if (yaw > 360.0f) {
		yaw -= 360.0f;
	}

	*playerCameraRotation = Quaternion::EulerAnglesToQuaternion(pitch, yaw, camOrientation.z);

	Vector3 camPos = (
		Matrix4::Translation(objPos + Vector3(0,10,0)) *
		Matrix4(*playerCameraRotation) *
		Matrix4::Translation(Vector3(0,0,20))
		).GetPositionVector();

	Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

	Matrix4 modelMat = temp.Inverse();
	//*

	
	//*/
	Quaternion q(modelMat);
	Vector3 angles = q.ToEuler(); //nearly there now!

	world->GetMainCamera().SetPosition(camPos);
	world->GetMainCamera().SetPitch(angles.x);
	world->GetMainCamera().SetYaw(angles.y);
}

void CourseworkGame::MovePlayerObject(float dt) {
	Matrix4 view = world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	//*
	
	//*/
	float spd = 20.0f * dt;

	if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) {
		playerObject->GetPhysicsObject()->AddForce(fwdAxis * spd);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) {
		playerObject->GetPhysicsObject()->AddForce(-fwdAxis * spd);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) {
		playerObject->GetPhysicsObject()->AddForce(-rightAxis * spd);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) {
		playerObject->GetPhysicsObject()->AddForce(rightAxis * spd);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
		//std::cout << "JUMP\n";

		playerObject->GetPhysicsObject()->ApplyLinearImpulse(Vector3(0, 0.1f, 0));
	}
	
}

void CourseworkGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

void CourseworkGame::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(-15.0f);
	world->GetMainCamera().SetYaw(315.0f);
	world->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void CourseworkGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();
	
	enemyObjects.clear();

	playerObject = AddPlayerToWorld(Vector3(20*8, 20, 20*9));
	
	
	//testStateObject = AddStateObjectToWorld(Vector3(0, 200, 0));
	GenerateLevel();
	for (int i = 0; i < 1; i++)
	{
		enemyObjects.push_back(AddEnemyToWorld(Vector3(18 * 20, 20, (1+i * 3) * 20)));
	}
	//InitMixedGridWorld(15, 15, 3.5f, 3.5f);
	//BridgeConstraintTest();
	//InitGameExamples();
	//InitDefaultFloor();
	
}

void CourseworkGame::GenerateLevel()
{
	levelData = new LevelData("TestGrid1.txt");
	int nodeSize = levelData->GetNodeSize();
	float nodeHeight = nodeSize * 0.25f;
	for (int i = 0; i < levelData->GetGridSize(); i++)
	{
		LevelGridUnit lgu = levelData->GetAllGridUnits()[i];
		int type = lgu.type;
		if (isdigit(type))
		{
			float unitHeight = nodeHeight * (float(type) - 48);
			AddCubeToWorld(lgu.position + Vector3(0, unitHeight,0), Vector3(1 * nodeSize / 2, unitHeight, 1 * nodeSize / 2) , 0);
		}
	}
	AddFloorToWorld(Vector3(levelData->GetGridDimentions().x / 2, -0.25, levelData->GetGridDimentions().y / 2) * nodeSize - Vector3(1 * nodeSize / 2, 0, 1 * nodeSize / 2)
		, Vector3(levelData->GetGridDimentions().x / 2 * nodeSize, 5, levelData->GetGridDimentions().y * nodeSize / 2));
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* CourseworkGame::AddFloorToWorld(const Vector3& position, Vector3 dimensions) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = dimensions;
	//AABBVolume* volume = new AABBVolume(floorSize);
	OBBVolume* volume = new OBBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position)
		.SetOrientation(Quaternion::EulerAnglesToQuaternion(0,0,0))
		;

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));
	floor->GetPhysicsObject()->SetElasticity(0.0f);
	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/



GameObject* CourseworkGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();
	sphere->GetPhysicsObject()->SetElasticity(1.0f);
	world->AddGameObject(sphere);

	return sphere;
}

GameObject* CourseworkGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* CourseworkGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius , float inverseMass) {
	GameObject* capsule = new GameObject();

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetPosition(position)
		.SetScale(Vector3(radius, halfHeight, radius));

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(capsule);

	return capsule;
}

GameObject* CourseworkGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize		= 2.0f;
	float inverseMass	= 50.0f;

	GameObject* character = new GameObject();
	CapsuleVolume* volume  = new CapsuleVolume(meshSize, meshSize);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), charMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();
	character->GetPhysicsObject()->SetElasticity(0.0f);

	world->AddGameObject(character);
	playerCameraRotation = new Quaternion(Vector3(0, 0, 0), 0);
	return character;
}

EnemyObject* CourseworkGame::AddEnemyToWorld(const Vector3& position) {

	float meshSize = 2.0f;
	float inverseMass = 50.0f;
	EnemyObject* character = new EnemyObject(levelData);
	CapsuleVolume* volume = new CapsuleVolume(meshSize, meshSize);

	character->SetPlayerObjectTarget(playerObject);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	


	

	
	world->AddGameObject(character);

	return character;
}

GameObject* CourseworkGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), cubeMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

StateGameObject* CourseworkGame::AddStateObjectToWorld(const Vector3& position)
{
	StateGameObject* apple = new StateGameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), cubeMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

void CourseworkGame::InitDefaultFloor() {
	//AddSphereToWorld(Vector3(50, 25, 0), 1.0f * 10.0f);
	//AddFloorToWorld(Vector3(0, -20, 0));
}

void CourseworkGame::InitGameExamples() {
	AddPlayerToWorld(Vector3(0, 5, 0));
	AddEnemyToWorld(Vector3(5, 5, 0));
	AddBonusToWorld(Vector3(10, 5, 0));
}

void CourseworkGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	//AddFloorToWorld(Vector3(0, -2, 0));
}

void CourseworkGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);
	AddCapsuleToWorld(Vector3(0, 75, 0), sphereRadius * 30.0f, sphereRadius * 30.0f);
	//AddSphereToWorld(Vector3(50, 25, 0), sphereRadius * 10.0f);
	//AddCapsuleToWorld(Vector3(0, 0, 0), cubeDims.y * 20, sphereRadius * 20);
	numRows = 10;
	for (int x = 0; x < numRows; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
				//AddCapsuleToWorld(position, cubeDims.y, sphereRadius);
			}
		}
	}
}

void CourseworkGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool CourseworkGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) 
			{
				Debug::DrawLine(ray.GetPosition(), closestCollision.collidedAt, Vector4(0, 1, 0, 1), 500.0f);
				selectionObject = (GameObject*)closestCollision.node;

				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				//Debug::DrawLine(ray.GetPosition(), ray.GetDirection() * 10000.0f, Vector4(1, 0, 0, 1), 500.0f);
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void CourseworkGame::MoveSelectedObject() {
	Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}

void CourseworkGame::BridgeConstraintTest() 
{
	std::cout << "THE BRIDGE\n";
	Vector3 cubeSize = Vector3(3, 3, 3);
	
	float invCubeMass = 5;
	int numLinks = 20;
	float maxDistance = 15; 
	float cubeDistance = 10; 
	
	Vector3 startPos = Vector3(100, 100, 100);
	
	GameObject * start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject * end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);
	
	GameObject * previous = start;
	
	for (int i = 0; i < numLinks; ++i) 
	{
		//Debug::DrawLine(Vector3(0, 0, 0), startPos + Vector3((i + 1) * cubeDistance, 0, 0), Vector4(0, 0, 1, 1),100.0f);
		GameObject * block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint * constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
	
}

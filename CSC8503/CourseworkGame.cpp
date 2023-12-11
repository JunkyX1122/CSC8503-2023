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

int BUTTON_SPACE = 0;
int BUTTON_W = 1;
int BUTTON_A = 2;
int BUTTON_S = 3;
int BUTTON_D = 4;
int BUTTON_F = 5;
int MOUSE_LEFT = 6;
int MOUSE_RIGHT = 7;

char IS_UP = '0';
char IS_PRESSED = '1';
char IS_DOWN = '2';

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
	useGravity		= true;
	physics->UseGravity(useGravity);
	inSelectionMode = false;
	
	testStateObject = nullptr;
	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");

	//InitialiseAssets();
}
void CourseworkGame::InitialiseGame()
{
	
}
void CourseworkGame::InitialiseGameAsServer()
{

	std::cout << "Started World As Server\n";
	
	cubeMesh = renderer->LoadMesh("cube.msh");
	sphereMesh = renderer->LoadMesh("sphere.msh");
	charMesh = renderer->LoadMesh("goat.msh");
	enemyMesh = renderer->LoadMesh("Keeper.msh");
	bonusMesh = renderer->LoadMesh("apple.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex = renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");
	

	NetworkBase::Initialise();
	int port = NetworkBase::GetDefaultPort();
	gameServer = new GameServer(port, 4, [&](int peerId) { OnPlayerConnect(peerId); }, [&](int peerId) { OnPlayerDisconnect(peerId); });
	
	gameServer->RegisterPacketHandler(Received_State, this);

	EraseWorld();
	InitWorld();
}

void CourseworkGame::OnPlayerConnect(int peerID)
{
	std::cout << "Player ID " << peerID << " has connected!\n";
	numberOfActivePlayers++;
}
void CourseworkGame::OnPlayerDisconnect(int peerID)
{
	std::cout << "Player ID " << peerID << " has disconnected!\n";
	numberOfActivePlayers--;
}
void CourseworkGame::InitialiseGameAsClient()
{
	
	
	std::cout << "Started World As Client\n";
	cubeMesh = renderer->LoadMesh("cube.msh");
	sphereMesh = renderer->LoadMesh("sphere.msh");
	charMesh = renderer->LoadMesh("goat.msh");
	enemyMesh = renderer->LoadMesh("Keeper.msh");
	bonusMesh = renderer->LoadMesh("apple.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex = renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");
	
	NetworkBase::Initialise();
	int port = NetworkBase::GetDefaultPort();

	gameClient = new GameClient([&](int peerId) { OnOtherPlayerConnect(peerId); }, [&](int peerId) { OnOtherPlayerDisconnect(peerId); });
	gameClient->RegisterPacketHandler(Delta_State, this);
	gameClient->RegisterPacketHandler(Full_State, this);
	gameClient->RegisterPacketHandler(Player_Connected, this);
	gameClient->RegisterPacketHandler(Player_Disconnected, this);
	gameClient->RegisterPacketHandler(Player_Info, this);
	bool canConnect = gameClient->Connect(127, 0, 0, 1, port);


	EraseWorld();
	InitWorld();
	InitCamera();
}
void CourseworkGame::OnOtherPlayerConnect(int peerID)
{

}
void CourseworkGame::OnOtherPlayerDisconnect(int peerID)
{

}
void CourseworkGame::EraseWorld()
{
	world->ClearAndErase();
	physics->Clear();
}
/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void CourseworkGame::InitialiseAssets() 
{

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
	
	if (gameClient) delete gameClient;
	if (gameServer) delete gameServer;
}

void CourseworkGame::UpdateGame(float dt)
{
	//UpdateAsServer(dt);
	//UpdateAsClient(dt);
}

void CourseworkGame::UpdateAsServer(float dt)
{
	for (auto pO : playerObject)
	{
		GamePacket* newPacket = nullptr;

		PlayerInfoPacket* pp = new PlayerInfoPacket();

		pp->yourAssignedObject = pO.first;
	
		newPacket = pp;

		gameServer->SendPacketToPeer(*newPacket, pO.first);
		delete newPacket;
		
	}
	for (auto pI : playerInputs)
	{
		for (int i = 0; i < sizeof(ClientPacket::buttonstates); i++)
		{
			//pI.second[i] = '0';
		}
	}
	for (auto pR : playerRotation)
	{
		//pR.second = Quaternion::EulerAnglesToQuaternion(0, 0, 0);
	}
	
	gameServer->UpdateServer();
	
	//Window::GetWindow()->ShowOSPointer(true);
	//Window::GetWindow()->LockMouseToWindow(false);
	if (playerObject.size() > 0)
	{
		for (auto pO : playerObject)
		{
			int playerID = pO.first;
			Vector3 playerPos = pO.second->GetTransform().GetPosition();
			if (playerPos.x < outOfBounds[0] || playerPos.z < outOfBounds[1] ||
				playerPos.x > outOfBounds[2] || playerPos.z > outOfBounds[3])
			{
				pO.second->GetTransform().SetPosition(pO.second->GetRespawnPoint());
				pO.second->GetPhysicsObject()->SetLinearVelocity(Vector3());
			}
			AttachCameraPlayer(true, pO.second, playerID);
			MovePlayerObject(dt, pO.second, playerID);

			if (pO.second && playerGroundedCollider[playerID])
			{
				playerGroundedCollider[playerID]->GetTransform().SetPosition(pO.second->GetTransform().GetPosition() + Vector3(0, -1.75f, 0));
			}
		}
	}


	UpdateKeys();

	UpdatePathFindings(dt);
	world->UpdateWorld(dt);
	//renderer->Update(dt);
	physics->Update(dt);
	if (true)
	{
		//world->GetMainCamera().UpdateCamera(dt);
		//Debug::UpdateRenderables(dt);
		//renderer->Render();
	}
	// Here we need to send the packets to the client. For now, I don't care about the client sending inputs, I just want the objects
	// to display on the client's screen.
	BroadcastSnapshot(false);
	//
	//std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void CourseworkGame::BroadcastSnapshot(bool deltaFrame) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;

	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		//TODO - you'll need some way of determining
		//when a player has sent the server an acknowledgement
		//and store the lastID somewhere. A map between player
		//and an int could work, or it could be part of a 
		//NetworkPlayer struct. 

		int playerState = 0;
		GamePacket* newPacket = nullptr;

		if (o->WritePacket(&newPacket, deltaFrame, playerState)) 
		{
			//std::cout << o->GetNetworkID() << "\n";
			gameServer->SendGlobalPacket(*newPacket);
			
			delete newPacket;
		}
	}
}
void CourseworkGame::UpdateAsClient(float dt)
{
	// Here we need to recieve the packets from the server.
	//world->ClearAndErase();
	//world->GetMainCamera().UpdateCamera(dt);
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM3))
	{
		gameClient->Disconnect();
	}
	if (gameClient)
	{
		gameClient->UpdateClient();
		ClientSendInputs();
		if (selfClientID >= 0)
		{
			AttachCameraPlayer(false, playerObject[selfClientID], selfClientID);
		}
		world->UpdateWorld(dt);
		renderer->Update(dt);
		renderer->Render();
		Debug::UpdateRenderables(dt);
	}
	//std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
void CourseworkGame::ClientSendInputs()
{
	ClientPacket newPacket;
	bool clientLastStateUpdate = false;
	for (int i = 0; i < sizeof(ClientPacket::buttonstates); i++)
	{
		newPacket.buttonstates[i] = '0';
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
		newPacket.buttonstates[0] = '1';
		clientLastStateUpdate = true;
	}
	else if (Window::GetKeyboard()->KeyDown(KeyCodes::SPACE))
	{
		newPacket.buttonstates[0] = '2';
		clientLastStateUpdate = true;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::W)) {
		newPacket.buttonstates[1] = '1';
		clientLastStateUpdate = true;
	}
	else if (Window::GetKeyboard()->KeyDown(KeyCodes::W))
	{
		newPacket.buttonstates[1] = '2';
		clientLastStateUpdate = true;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::A)) {
		newPacket.buttonstates[2] = '1';
		clientLastStateUpdate = true;
	}
	else if (Window::GetKeyboard()->KeyDown(KeyCodes::A))
	{
		newPacket.buttonstates[2] = '2';
		clientLastStateUpdate = true;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::S)) {
		newPacket.buttonstates[3] = '1';
		clientLastStateUpdate = true;
	}
	else if (Window::GetKeyboard()->KeyDown(KeyCodes::S))
	{
		newPacket.buttonstates[3] = '2';
		clientLastStateUpdate = true;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::D)) {
		newPacket.buttonstates[4] = '1';
		clientLastStateUpdate = true;
	}
	else if (Window::GetKeyboard()->KeyDown(KeyCodes::D))
	{
		newPacket.buttonstates[4] = '2';
		clientLastStateUpdate = true;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F)) {
		newPacket.buttonstates[5] = '1';
		clientLastStateUpdate = true;
	}
	else if (Window::GetKeyboard()->KeyDown(KeyCodes::F))
	{
		newPacket.buttonstates[5] = '2';
		clientLastStateUpdate = true;
	}

	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Left)) {
		newPacket.buttonstates[6] = '1';
		clientLastStateUpdate = true;
	}
	else if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left))
	{
		newPacket.buttonstates[6] = '2';
		clientLastStateUpdate = true;
	}

	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
		newPacket.buttonstates[7] = '1';
		clientLastStateUpdate = true;
	}
	else if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Right))
	{
		newPacket.buttonstates[7] = '2';
		clientLastStateUpdate = true;
	}
	
		newPacket.camPitch = world->GetMainCamera().GetPitch();
		newPacket.camYaw = world->GetMainCamera().GetYaw();
		newPacket.lastID = 0;
		gameClient->SendPacket(newPacket);
	
	
	
	
}
void CourseworkGame::ReceivePacket(int type, GamePacket* payload, int source)
{
	if (gameClient)
	{
		if (type == BasicNetworkMessages::Player_Info)
		{
			PlayerInfoPacket* infoPacket = (PlayerInfoPacket*)payload;
			//std::cout << infoPacket->yourAssignedObject << "\n";
			selfClientID = infoPacket->yourAssignedObject;
		}
		else
		{
			//std::cout << selfClientID << "\n";
			if (world->GetAllObjects().size() > 0)
			{
				for (GameObject* gb : world->GetAllObjects())
				{
					if (gb->GetNetworkObject())
					{
						if (gb->GetNetworkObject()->ReadPacket(*payload)) return;
					}
				}
			}
		}
	}
	if (gameServer)
	{
		int playerID = source;
		ClientPacket* clientPacket = (ClientPacket*)payload;
		//std::cout << "Player ID: " << playerID << "\n";
		for (int i = 0; i < sizeof(ClientPacket::buttonstates); i++)
		{
			playerInputs[playerID][i] = clientPacket->buttonstates[i];

			//std::cout << playerInputs[playerID][i];
		}
		//std::cout << "\n";
		playerRotation[playerID] = Quaternion::EulerAnglesToQuaternion(clientPacket->camPitch, clientPacket->camYaw, 0);
		
	}
}
void CourseworkGame::UpdateOuter(float dt)
{
	renderer->Update(dt);
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
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::O)) {
		inPlayerMode = !inPlayerMode;
		Window::GetWindow()->ShowOSPointer(!inPlayerMode);
		Window::GetWindow()->LockMouseToWindow(inPlayerMode);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F3)) {
		physics->ToggleDrawHitboxes();
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

void CourseworkGame::AttachCameraPlayer(bool asServer, PlayerObject* pO, int playerID)
{
	
	Vector3 objPos = pO->GetTransform().GetPosition();

	Vector3 camOrientation = playerCameraRotation[playerID]->ToEuler();
	
	//camOrientation = playerObject->GetTransform().GetOrientation().ToEuler();
	float pitch = camOrientation.x;
	float yaw = camOrientation.y;
	if (asServer)
	{
		*playerCameraRotation[playerID] = playerRotation[playerID];
	}
	else
	{
		float pitch = camOrientation.x;
		float yaw = camOrientation.y;

		pitch -= controller.GetNamedAxis("YLook");
		yaw -= controller.GetNamedAxis("XLook");

		pitch = std::min(pitch, 89.0f);
		pitch = std::max(pitch, -89.0f);

		if (yaw < 0) {
			yaw += 360.0f;
		}
		if (yaw > 360.0f) {
			yaw -= 360.0f;
		}
		camOrientation.x = pitch;
		camOrientation.y = yaw;
		*playerCameraRotation[playerID] = Quaternion::EulerAnglesToQuaternion(pitch, yaw, camOrientation.z);
	}
	
	Vector3 offset(0, 4, 0);
	Vector3 camPos = (
		Matrix4::Translation(objPos + offset) *
		Matrix4(*playerCameraRotation[playerID]) *
		Matrix4::Translation(Vector3(0,0,13))
		).GetPositionVector();

	Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos + offset, Vector3(0, 1, 0));

	Matrix4 modelMat = temp.Inverse();
	
	pO->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0, camOrientation.y, 0));
	
	//*

	
	//*/
	Quaternion q(modelMat);
	Vector3 angles = q.ToEuler(); //nearly there now!

	float dampen = 0.3;
	
	world->GetMainCamera().SetPosition(world->GetMainCamera().GetPosition() * (1.0f - dampen) + camPos * dampen);
	world->GetMainCamera().SetPitch(angles.x);
	world->GetMainCamera().SetYaw(angles.y);
	
	
}

void CourseworkGame::MovePlayerObject(float dt, PlayerObject* pO, int playerID) 
{
	
	Quaternion camQuat = Quaternion::EulerAnglesToQuaternion(world->GetMainCamera().GetPitch(), world->GetMainCamera().GetYaw(), 0);
	Debug::DrawSphereLines(
		(Matrix4::Translation(world->GetMainCamera().GetPosition()) * Matrix4(camQuat) * Matrix4::Translation(Vector3(0, 0, -5))).GetPositionVector(),
		camQuat,
		0.05f);
	if (playerInputs[playerID][MOUSE_RIGHT] == IS_DOWN)
	{


		Ray ray = CollisionDetection::BuildRayFromCentre(world->GetMainCamera());

		RayCollision closestCollision;
		std::vector<int> ignoreList = { LAYER_PLAYER };
		if (world->Raycast(ray, closestCollision, true, nullptr, ignoreList))
		{
			//Debug::DrawLine(ray.GetPosition(), closestCollision.collidedAt, Vector4(0, 1, 0, 1), 500.0f);
			//selectionObject = (GameObject*)closestCollision.node;
			if (!pO->IsGrappling())
			{
				pO->SetGrapplePoint(closestCollision.collidedAt);
				pO->SetGrappling(true);
				Debug::DrawLine(pO->GetGrapplePoint(), pO->GetTransform().GetPosition(), Vector4(0.75, 0, 1, 1));
			}
			//selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
			//return true;
		}
		else
		{
			pO->SetGrapplePoint(Vector3(0,0,0));
			pO->SetGrappling(false);
		}

	}
	if (playerInputs[playerID][MOUSE_RIGHT] == IS_UP)
	{
		pO->SetGrapplePoint(Vector3(0, 0, 0));
		pO->SetGrappling(false);
	}
	if (pO->IsGrappling())
	{
		float grappleForce = 1.0f;
		Debug::DrawLine(pO->GetGrapplePoint(), pO->GetTransform().GetPosition(), Vector4(0.5f, 0, 1, 1));
		Vector3 direction = (pO->GetGrapplePoint() - pO->GetTransform().GetPosition()).Normalised();
		pO->GetPhysicsObject()->AddForce(direction * grappleForce);
	}

	Matrix4 view = world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	Vector3 fwdAxisCamera = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxisCamera.Normalise();

	



	float spd = 20.0f * dt ;

	Vector3 moveDirection = Vector3(0, 0, 0);
	if (playerInputs[playerID][BUTTON_W] == IS_DOWN) {
		moveDirection = (fwdAxis);
	}

	if (playerInputs[playerID][BUTTON_S] == IS_DOWN) {
		moveDirection = (-fwdAxis);
	}

	if (playerInputs[playerID][BUTTON_A] == IS_DOWN) {
		moveDirection = (-rightAxis);
	}

	if (playerInputs[playerID][BUTTON_D] == IS_DOWN) {
		moveDirection = (rightAxis);
	}
	pO->GetPhysicsObject()->AddForce(moveDirection * spd);
	
	if (playerInputs[playerID][BUTTON_F] == IS_DOWN)
	{
		pO->GetPhysicsObject()->ApplyLinearImpulse(camQuat*Vector3(0,0,-1) * 128.0f * dt);
	}

	//std::cout << playerGroundedCollider->IsColliding() << "\n";
	if (playerInputs[playerID][BUTTON_SPACE] == IS_PRESSED && playerGroundedCollider[playerID]->IsColliding()) {
		//std::cout << "JUMP\n";

		pO->GetPhysicsObject()->ApplyLinearImpulse(Vector3(0, 32.0f, 0) * dt);
	}
	playerGroundedCollider[playerID]->GetPhysicsObject()->SetLinearVelocity(pO->GetPhysicsObject()->GetLinearVelocity());
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
	//playerObject = AddPlayerToWorld(Vector3(20 * 8, 5, 20 * 9));
	//playerObject->SetRespawnPoint(Vector3(20 * 8, 5, 20 * 9));
	//playerGroundedCollider = AddSphereToWorld(playerObject->GetTransform().GetPosition(), 1.0f, 0.1f, LAYER_DEFAULT, false, false);
	//playerGroundedCollider->AddToIgnoreList(playerObject);
	if(gameClient)
	{ 
		for (int i = 0; i < 4; i++)
		{
			InitialisePlayerAsClient(i);
		}
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			InitialisePlayerAsServer(i);
		}
	}
	if (true)
	{
		GenerateLevel();
		for (int i = 0; i < 5; i++)
		{
			//enemyObjects.push_back(AddEnemyToWorld(Vector3(18 * 20, 10, (1 + i * 3) * 20)));
		}
	}
	else
	{
		InitMixedGridWorld(15, 15, 3.5f, 3.5f);
		BridgeConstraintTest();
		InitDefaultFloor();
	}
	
}

void CourseworkGame::InitialisePlayerAsServer(int playerID)
{
	playerObject.insert({ playerID, AddPlayerToWorld(playerID, Vector3(20 * 8 + 15 * playerID, 5, 20 * 9)) });
	playerObject[playerID]->SetRespawnPoint(Vector3(20 * 8, 5, 20 * 9));
	playerGroundedCollider.insert({ playerID, AddSphereToWorld(playerObject[playerID]->GetTransform().GetPosition(), 1.0f, 0.1f, LAYER_DEFAULT, false, false) });
	playerGroundedCollider[playerID]->AddToIgnoreList(playerObject[playerID]);
}
void CourseworkGame::InitialisePlayerAsClient(int playerID)
{
	playerObject.insert({playerID, AddPlayerToWorld(playerID, Vector3(20 * 8 + 15 * playerID, 5, 20 * 9))});
	playerObject[playerID]->SetRespawnPoint(Vector3(20 * 8, 5, 20 * 9));
	playerGroundedCollider.insert({ playerID, AddSphereToWorld(playerObject[playerID]->GetTransform().GetPosition(), 1.0f, 0.1f, LAYER_DEFAULT, false, false)});
	playerGroundedCollider[playerID]->AddToIgnoreList(playerObject[playerID]);
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

	outOfBounds[0] = -nodeSize / 2;
	outOfBounds[1] = -nodeSize / 2;

	outOfBounds[2] = levelData->GetGridDimentions().x * nodeSize + nodeSize / 2;
	outOfBounds[3] = levelData->GetGridDimentions().y * nodeSize + nodeSize / 2;
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
GameObject* CourseworkGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass, int collisionLayer, bool isCollidable, bool rendered) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius, collisionLayer, isCollidable);
	
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	
	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);


	if(rendered) sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();
	sphere->GetPhysicsObject()->SetElasticity(1.0f);
	world->AddGameObject(sphere);

	return sphere;
}

GameObject* CourseworkGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, int collisionLayer, bool isCollidable, bool rendered) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions, collisionLayer, isCollidable);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	if (rendered) cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* CourseworkGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius , float inverseMass, int collisionLayer, bool isCollidable, bool rendered) {
	GameObject* capsule = new GameObject();

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius, collisionLayer, isCollidable);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetPosition(position)
		.SetScale(Vector3(radius, halfHeight, radius));

	if (rendered) capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(capsule);

	return capsule;
}

PlayerObject* CourseworkGame::AddPlayerToWorld(int playerID, const Vector3& position) {
	float meshSize		= 2.0f;
	float inverseMass	= 50.0f;

	PlayerObject* character = new PlayerObject();
	character->SetName("Player");
	CapsuleVolume* volume  = new CapsuleVolume(meshSize, meshSize, LAYER_PLAYER);
	
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), charMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));
	character->SetNetworkObject(new NetworkObject(*character, playerID));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();
	character->GetPhysicsObject()->SetElasticity(0.0f);
	world->AddGameObject(character);
	playerCameraRotation[playerID] = new Quaternion(Vector3(0, 0, 0), 0);
	return character;
}

EnemyObject* CourseworkGame::AddEnemyToWorld(const Vector3& position) {

	float meshSize = 2.0f;
	float inverseMass = 50.0f;
	EnemyObject* character = new EnemyObject(levelData,world, "GenericEnemy");
	CapsuleVolume* volume = new CapsuleVolume(meshSize, meshSize, LAYER_ENEMY);

	character->SetPlayerObjectTarget(playerObject[selfClientID]);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));
	//character->SetNetworkObject(new NetworkObject((GameObject&)character, 1000+enemyObjects.size()));

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
	AddFloorToWorld(Vector3(0, -20, 0), Vector3(200,5,200));
}

void CourseworkGame::InitGameExamples() {
	//AddPlayerToWorld(Vector3(0, 5, 0));
	//AddEnemyToWorld(Vector3(5, 5, 0));
	//AddBonusToWorld(Vector3(10, 5, 0));
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
	if (true) {
		Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromCentre(world->GetMainCamera());

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

#include "NetworkObject.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

NetworkObject::NetworkObject(GameObject& o, int id) : object(o)	{
	deltaErrors = 0;
	fullErrors  = 0;
	networkID   = id;
}

NetworkObject::~NetworkObject()	{
}

bool NetworkObject::ReadPacket(GamePacket& p)
{
	//std::cout<<p.type << "\n";
	if (p.type == Delta_State)
	{
		//std::cout << "dt" << "\n";
		return ReadDeltaPacket((DeltaPacket&)p);
	}
	if (p.type == Full_State)
	{
		//std::cout << "fp" << "\n";
		return ReadFullPacket((FullPacket&)p);
	}

	return false;
}

bool NetworkObject::WritePacket(GamePacket** p, bool deltaFrame, int stateID) {
	if (deltaFrame) {
		if (!WriteDeltaPacket(p, stateID)) {
			return WriteFullPacket(p);
		}
	}
	return WriteFullPacket(p);
}
//Client objects recieve these packets
bool NetworkObject::ReadDeltaPacket(DeltaPacket &p) 
{
	if (p.fullID != lastFullState.stateID) return false;
	
	UpdateStateHistory(p.fullID);

	Vector3 fullPos = lastFullState.position;
	Quaternion fullOrientation = lastFullState.orientation;

	fullPos.x += p.pos[0];
	fullPos.y += p.pos[1];
	fullPos.z += p.pos[2];

	fullOrientation.x += ((float)p.orientation[0]) / 127.0f;
	fullOrientation.y += ((float)p.orientation[1]) / 127.0f;
	fullOrientation.z += ((float)p.orientation[2]) / 127.0f;
	fullOrientation.w += ((float)p.orientation[3]) / 127.0f;

	object.GetTransform().SetPosition(fullPos);
	object.GetTransform().SetOrientation(fullOrientation);

	return true;
}

bool NetworkObject::ReadFullPacket(FullPacket &p) 
{
	if (p.fullState.stateID < lastFullState.stateID)
	{
		return false;
	}

	if (p.objectID != object.GetNetworkObject()->GetNetworkID()) return false;

	lastFullState = p.fullState;
	//std::cout << p.fullState.position << "\n";
	//std::cout << lastFullState.position << "\n";
	object.GetTransform().SetPosition(lastFullState.position);
	object.GetTransform().SetOrientation(lastFullState.orientation);

	

	stateHistory.emplace_back(lastFullState);

	return true;
}

bool NetworkObject::WriteDeltaPacket(GamePacket**p, int stateID) 
{
	DeltaPacket* dp = new DeltaPacket();
	NetworkState state;
	if (!GetNetworkState(stateID, state)) return false;

	dp->fullID = stateID;
	dp->objectID = networkID;

	Vector3 currentPos = object.GetTransform().GetPosition();
	Quaternion currentOrientation = object.GetTransform().GetOrientation();

	currentPos -= state.position;
	currentOrientation -= state.orientation;

	dp->pos[0] = (char)currentPos.x;
	dp->pos[1] = (char)currentPos.y;
	dp->pos[2] = (char)currentPos.z;
	
	dp->orientation[0] = (char)(currentOrientation.x * 127.0f);
	dp->orientation[1] = (char)(currentOrientation.y * 127.0f);
	dp->orientation[2] = (char)(currentOrientation.z * 127.0f);
	dp->orientation[3] = (char)(currentOrientation.w * 127.0f);
	*p = dp;
	return true;
}

bool NetworkObject::WriteFullPacket(GamePacket**p) 
{
	FullPacket* fp = new FullPacket();
	
	fp->objectID = networkID;
	fp->fullState.position = object.GetTransform().GetPosition();
	//std::cout << fp->fullState.position << "\n";
	fp->fullState.orientation = object.GetTransform().GetOrientation();
	fp->fullState.stateID = lastFullState.stateID++;
	
	*p = fp;
	return true;
}

NetworkState& NetworkObject::GetLatestNetworkState() 
{
	return lastFullState;
}

bool NetworkObject::GetNetworkState(int stateID, NetworkState& state) 
{
	for (auto i = stateHistory.begin(); i < stateHistory.end(); i++)
	{
		if ((*i).stateID == stateID)
		{
			state = (*i);
			return true;
		}
	}
	return false;
}

void NetworkObject::UpdateStateHistory(int minID) 
{
	for (auto i = stateHistory.begin(); i < stateHistory.end();)
	{
		if ((*i).stateID < minID) i = stateHistory.erase(i);
		else i++;
	}
}
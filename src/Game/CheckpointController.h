#pragma once
#include <vector>
#include "../../Physics/World/TriggerBox.h"
#include "../../Physics/World/PhysicalObject.h"
#include "../../Platform/IO/JsonParser.h"

struct Checkpoint {

	TriggerBox* triggerBox;
	std::map<int, bool> passed;
	int indexCheckpoint;
	float distGround;
	DirectX::XMVECTOR direction;
	~Checkpoint() {
		delete triggerBox;
	}
};

class CheckpointController
{
	std::vector<Checkpoint*> m_checkpoints;
	std::vector<int> indexLastCP{};

public:
	CheckpointController() = default;	
	~CheckpointController() {
		deleteCheckpoints();
	}
	
	void addCheckpoint(PhysicalObject::fVec3 position, PhysicalObject::fVec3 scale, DirectX::XMVECTOR direction = DirectX::XMVECTOR(), float distGround = 0.f);

	void addCheckpointFromJson(const std::vector<FormatJson> & checkpointInfos);

	void validAllCheckpoints();
	void validNCheckpoints(int n);
	void validNextCheckpoints();

	void resetAllCheckpoints();
	void resetLastCheckpoint();
	void deleteCheckpoints();

	[[nodiscard]] bool allCheckpointsPassed(const int& id) const;

	[[nodiscard]] DirectX::XMVECTOR  getPositionLastCP(const int& id) const;
	[[nodiscard]] DirectX::XMVECTOR  getDirectionLastCP(const int& id) const;

	void addNewPlayer(int id) {
		indexLastCP.push_back(0);
		m_checkpoints[0]->passed[id] = true;
	}

	bool isCheckpointPassed(int id, int index) {
		return m_checkpoints[index]->passed[id];
	}
};


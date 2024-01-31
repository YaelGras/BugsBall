#include "CheckpointController.h"
#include <algorithm>
#include <ranges>
#include "GameLogic.h"
#include "../../Platform/IO/Sound.h"

void CheckpointController::addCheckpoint(PhysicalObject::fVec3 position, PhysicalObject::fVec3 scale, DirectX::XMVECTOR direction, float distGround)
{
	Checkpoint* cp = new Checkpoint{};
	cp->indexCheckpoint = static_cast<int>(m_checkpoints.size());
	cp->distGround = distGround;
	
	cp->direction = DirectX::XMVector3NormalizeEst(direction);
	cp->triggerBox = new TriggerBox{ position, scale };
	cp->triggerBox->setTriggerCallback(std::move([cp, this](int i) {
		if(!cp->passed[i] 
			&& cp->indexCheckpoint != 0
			&& m_checkpoints[cp->indexCheckpoint - 1]->passed[i])
		{
			cp->passed[i] = true;
			indexLastCP[i] = cp->indexCheckpoint;
			if(i == 0)
			{
				GameLogic::setCheckpointPassed();
				Sound::playSound(SoundType::Effect);
			}
		}
		else if (i == 0
			&& !cp->passed[i]
			&& cp->indexCheckpoint != 0
			&& !m_checkpoints[cp->indexCheckpoint - 1]->passed[i])
			GameLogic::setCheckpointMissed();
		}));
	cp->triggerBox->setName((cp->indexCheckpoint == 0 ?  std::string("Start") : std::string("Checkpoint_") + std::to_string(cp->indexCheckpoint)));
	
	m_checkpoints.push_back(cp);

}

void CheckpointController::addCheckpointFromJson(const std::vector<FormatJson>& checkpointInfos)
{
	std::ranges::for_each(checkpointInfos, [&](const FormatJson& infos) {
			PhysicalObject::fVec3 position{ DirectX::XMVectorGetX(infos.positionObj),  DirectX::XMVectorGetY(infos.positionObj),  DirectX::XMVectorGetZ(infos.positionObj) };
			PhysicalObject::fVec3 scale = { DirectX::XMVectorGetX(infos.scaleObj),  DirectX::XMVectorGetY(infos.scaleObj),  DirectX::XMVectorGetZ(infos.scaleObj) };

			addCheckpoint(position, scale, infos.forwardObj, DirectX::XMVectorGetW(infos.positionObj));
		});
}

void CheckpointController::validAllCheckpoints()
{
	for (Checkpoint* cp : m_checkpoints)
		cp->passed[0] = true;
	
}

void CheckpointController::validNCheckpoints(int n)
{
	for (Checkpoint* cp : m_checkpoints)
		if(cp->indexCheckpoint <= n)
			cp->passed[0] = true;
		else cp->passed[0] = false;

	indexLastCP[0] = n;
}

void CheckpointController::validNextCheckpoints()
{
	if(m_checkpoints.size() > indexLastCP[0] + 1)
	{
		indexLastCP[0]++;
		m_checkpoints[indexLastCP[0]]->passed[0] = true;
	}
}

void CheckpointController::resetAllCheckpoints()
{
	for (Checkpoint* cp : m_checkpoints)
		for (auto& [id, passed] : cp->passed)
			if (cp->indexCheckpoint != 0)
				passed = false;
			else
				passed = true;
	
	for(auto& index : indexLastCP)
		index = 0;
}

void CheckpointController::resetLastCheckpoint()
{
	if (indexLastCP[0] > 0)
		m_checkpoints[indexLastCP[0]--]->passed[0] = false;
	if (!m_checkpoints[indexLastCP[0]]->passed[0])
	{
		int i = 0; 
		while(m_checkpoints[i + 1]->passed[0])
			i++;
		indexLastCP[0] = i;
	}
}


void CheckpointController::deleteCheckpoints()
{
	for (Checkpoint* cp : m_checkpoints)
		delete cp;
	m_checkpoints.clear();
}

bool CheckpointController::allCheckpointsPassed(const int& id) const
{
	return std::ranges::all_of(m_checkpoints, [&](const Checkpoint* cp) { return cp->passed.at(id); });
}

DirectX::XMVECTOR CheckpointController::getPositionLastCP(const int& id) const
{
	DirectX::XMVECTOR pos = m_checkpoints[indexLastCP[id]]->triggerBox->getTransform().getPosition(); 
	return DirectX::XMVectorSet(DirectX::XMVectorGetX(pos), DirectX::XMVectorGetY(pos) - m_checkpoints[indexLastCP[id]]->distGround, DirectX::XMVectorGetZ(pos), 0);
}
DirectX::XMVECTOR CheckpointController::getDirectionLastCP(const int& id) const
{	
	return m_checkpoints[indexLastCP[id]]->direction;
}

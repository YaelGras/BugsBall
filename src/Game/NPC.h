#pragma once
#include "../../Physics/World/DynamicObject.h"
#include "Trace.h"

class NPC
{
	static int nextId;
	DynamicObject m_object;
	Trace m_trace{};
	int m_nextPoint = 1;
	float maxVelocity;
	int id;
	std::string name;

	Mesh m_mesh;

	int updateBeforeTP = 1000;
	int nbUpdate = 0;
	bool isRespawn = false;
	bool isUpdatable = true;

public:
	NPC();
	
	~NPC() = default;


	void swap(NPC& other) noexcept;
	NPC(NPC&& other) noexcept;
	NPC& operator=(NPC&& other) noexcept;

	void setTrace(const Trace& trace) { m_trace = trace; }
	void update();
	void setPosition(float x, float y, float z);
	void addBoost(float value);
	const Mesh& getMesh() const noexcept { return *(m_object.getMesh()); }
	std::string getName() const noexcept { return m_object.getPhysxName(); }

	void enableUpdate();
	void disableUpdate();

	static void resetId() { nextId = 1; }
};


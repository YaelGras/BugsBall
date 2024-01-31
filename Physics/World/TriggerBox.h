#pragma once
#include "StaticObject.h"
#include <functional>


class TriggerBox : public StaticObject
{

	std::function<void(int)> m_triggerCallback = std::function<void(int)>();
	Transform m_transform;
public:
	TriggerBox() = default;
	TriggerBox(fVec3 position, fVec3 scale);

	TriggerBox(group typeObject, fVec3 position = fVec3(0,0,0), fVec3 scale = fVec3(1,1,1));

	virtual void addShape(PxShape* shape);

	void setPosition(fVec3 pos = fVec3(0, 0, 0));

	void setTriggerCallback(std::function<void(int)> callback);

	void onTrigger(int id);

	virtual Transform& getTransform() override;
};


#pragma once

#include <concepts>
#include <memory>
#include <string>
#include <PxPhysicsAPI.h>
#include <vector>


#define PHYSIC_CALL_IMPL(fnName)\
static auto fnName(auto&& ...args)\
{\
	return implementation->##fnName(std::forward<decltype(args)>(args)...);\
}


class PhysicsEngine {
public:
	using fVec3 = physx::PxVec3;
	using Actor = physx::PxRigidActor;
	using Material = physx::PxMaterial;

	struct HeightMapData {
		unsigned int width, length;
		float scaleXZ, scaleY;
		std::vector<float> value;
		float maxHeight;
	};

	struct FilterGroup
	{
		enum Enum
		{
			ePlayer = (1 << 0),
			eCheckpoint = (1 << 1),
			eFinish = (1 << 2),
			eOther = (1 << 3),
			eAll = (1 << 4)
		};
	};


	struct _ImplPhysic
	{
		friend class PhysicsEngine;		

	protected:
		virtual bool changeScene(int numScene) = 0;
		virtual int addScene() = 0;
		virtual void resetScene() = 0;
		virtual void onInit() = 0;
		virtual void onUpdate(float deltaTime) = 0;
		virtual void cleanupPhysics(bool /* float restitution, interactive*/) = 0;
		virtual std::pair<fVec3, fVec3> getTransform(std::string id) = 0;
		virtual bool addActor(Actor* actor) = 0;
		virtual Actor* createStaticActor(const fVec3 position = fVec3{0,0,0}) = 0;
		virtual Actor* createDynamicActor(const fVec3 position = fVec3{0,0,0}) = 0;
		virtual Material* createMaterial(float restitution = 0.f, float staticFriction = 0.f, float dynamicFriction = 0.f) = 0;
		virtual void disableUpdate() = 0;
		virtual void enableUpdate() = 0;
	};

	PHYSIC_CALL_IMPL(changeScene);
	PHYSIC_CALL_IMPL(addScene);
	PHYSIC_CALL_IMPL(resetScene);
	PHYSIC_CALL_IMPL(onInit);
	PHYSIC_CALL_IMPL(cleanupPhysics);
	PHYSIC_CALL_IMPL(getTransform);
	PHYSIC_CALL_IMPL(addActor);
	PHYSIC_CALL_IMPL(createStaticActor);
	PHYSIC_CALL_IMPL(createDynamicActor);
	PHYSIC_CALL_IMPL(createMaterial);
	PHYSIC_CALL_IMPL(disableUpdate);
	PHYSIC_CALL_IMPL(enableUpdate);

	static auto onUpdate(float deltaTime)
	{
		if (!isRunning) return;
		return implementation->onUpdate(deltaTime);
	}

	static void setRunningState(bool runPhysics)
	{
		isRunning = runPhysics;
	}


	template<typename _PhysicEngineImplementation>
		requires (std::derived_from<_PhysicEngineImplementation, _ImplPhysic>)
	static void setImplementation()
	{
		implementation = std::make_unique<_PhysicEngineImplementation>();
	}

private:

	static bool isRunning;
	static std::unique_ptr<_ImplPhysic> implementation;
};
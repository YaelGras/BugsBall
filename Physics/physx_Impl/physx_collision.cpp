#include "physx_collision.h"
#include "../World/TriggerBox.h"
#include <iostream>
void Physx_Collision::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
    for (PxU32 i = 0; i < count; i++) {
        // Vérifiez si la trigger box est impliquée dans la paire
        if (pairs[i].triggerActor->is<PxRigidActor>()) {
            // Récupération des formes de l'acteur
            if(pairs[i].otherActor->is<PxRigidActor>()) 
                if (pairs[i].otherActor->userData && pairs[i].triggerActor->userData)
                {
                    try {
                        int id = *reinterpret_cast<int*>(pairs[i].otherActor->userData);
                        reinterpret_cast<TriggerBox*>(pairs[i].triggerActor->userData)->onTrigger(id);
                    }
                    catch (...) {
                        //std::cout << "Error onTrigger" << std::endl;
                    }
                }            
        }
    }
}

void Physx_Collision::onConstraintBreak(PxConstraintInfo* constraints, PxU32 count)
{
}

void Physx_Collision::onWake(PxActor** actors, PxU32 count)
{
}

void Physx_Collision::onSleep(PxActor** actors, PxU32 count)
{
}

void Physx_Collision::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{
}

void Physx_Collision::onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count)
{
}

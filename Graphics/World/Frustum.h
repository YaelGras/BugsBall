#pragma once

#define _XM_NO_INTRINSICS_
#include <directXmath.h>
#include "../abstraction/Camera.h"

struct AABB;

//=========================================================================================================================//

struct Plan {

    DirectX::XMVECTOR normal;
    float distanceToOrigin;

    //////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Constructors

    Plan() :
        normal(), distanceToOrigin(0)
    {}

    Plan(const DirectX::XMVECTOR& pl, const DirectX::XMVECTOR& norm) :
        normal(XMVector3Normalize(norm)),
        distanceToOrigin(XMVectorGetX(XMVector3Dot(normal, pl)))
    {}

    //////////////////////////////////////////////////////////////////////////////////////////////
	/// -- Operations

    inline float getSDToPlan(const DirectX::XMVECTOR& point) const {
        return XMVectorGetX(XMVector3Dot(normal, point)) - distanceToOrigin;
    }
};

//=========================================================================================================================//

struct Frustum {

    Plan topFace;
    Plan bottomFace;

    Plan rightFace;
    Plan leftFace;

    Plan farFace;
    Plan nearFace;

    //////////////////////////////////////////////////////////////////////////////////////////////
    /// -- Operations

    bool            isOnFrustum(const AABB& boundingBox) const;
    static Frustum  createFrustumFromPerspectiveCamera(const Camera& cam);
    static bool     isOnOrForwardPlan(const Plan& plan, const AABB& boundingBox);
};

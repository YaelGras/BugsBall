#include "Frustum.h"

#include "../../Utils/AABB.h"

Frustum Frustum::createFrustumFromPerspectiveCamera(const Camera& cam)
{

    const PerspectiveProjection& proj = cam.getProjection<PerspectiveProjection>();

    Frustum frustum;

    const float halfVSide = -proj.zfar * tanf(proj.fov * .5f);
    const float halfHSide = halfVSide * proj.aspectRatio;
    const XMVECTOR frontMultFar = -proj.zfar * cam.getForward();

    const XMVECTOR hz = cam.getHorizontalDir();
    const XMVECTOR fw = cam.getForward();
    const XMVECTOR up = cam.getUp();
    const XMVECTOR campos = cam.getPosition();

    frustum.nearFace = { campos + -proj.znear * fw, -fw };
    frustum.farFace = { campos + frontMultFar, fw };

    frustum.rightFace = { campos, XMVector3Cross(up, frontMultFar + hz * halfHSide) };
    frustum.leftFace = { campos, XMVector3Cross(frontMultFar - hz * halfHSide, up) };
    frustum.topFace = { campos, XMVector3Cross(hz, frontMultFar - up * halfVSide) };
    frustum.bottomFace = { campos, XMVector3Cross(frontMultFar + up * halfVSide, hz) };

    return frustum;

}

bool Frustum::isOnFrustum(const AABB& boundingBox) const
{
    return (
        isOnOrForwardPlan(leftFace,     boundingBox) &&
        isOnOrForwardPlan(rightFace,    boundingBox) &&
        isOnOrForwardPlan(topFace,      boundingBox) &&
        isOnOrForwardPlan(bottomFace,   boundingBox) &&
        isOnOrForwardPlan(nearFace,     boundingBox) &&
        isOnOrForwardPlan(farFace,      boundingBox)
        );
}

bool Frustum::isOnOrForwardPlan(const Plan& plan, const AABB& boundingBox)
{
    // Compute the projection interval radius of b onto L(t) = b.c + t * p.n

    const XMVECTOR center = XMVectorAdd(boundingBox.origin, 
        XMVectorDivide(boundingBox.size, XMVECTOR{2,2,2,2 }
    ));
    const XMVECTOR e = XMVectorSubtract(XMVectorAdd(boundingBox.origin, boundingBox.size), center);

    const float r =
        XMVectorGetX(e) * std::abs(XMVectorGetX(plan.normal))
        + XMVectorGetY(e) * std::abs(XMVectorGetY(plan.normal))
        + XMVectorGetZ(e) * std::abs(XMVectorGetZ(plan.normal));

    return -r <= plan.getSDToPlan(center);
}
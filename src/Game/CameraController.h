#pragma once

#include "Cloporte.h"
#include "abstraction/Camera.h"
#include "World/WorldRendering/Terrain.h"



class CameraController
{

private:

    static const Terrain* terrain; // HORRIBLE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

public:

    static void setTerrain(const Terrain* t){  terrain = t; }


    static void     computeFirstPersonPosition(const Cloporte& player, Camera& cam);
    static void     computeThirdPersonPosition(const Cloporte& player, Camera& cam);

    static float    computeCameraRoll(  const Cloporte& player, const Camera& cam);
	static float    computeCameraHeight(const Cloporte& player);

};


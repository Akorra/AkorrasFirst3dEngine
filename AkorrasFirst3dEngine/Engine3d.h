#pragma once

#include "ConsoleGameEngine.h"
#include "engine_utils.h"

class Engine3D : public ConsoleGameEngine
{

private:
	mesh	meshCube;
	mat4x4	matProj;
	vec3d	vCamera;

	float	fTheta;

	// Taken From Command Line Webcam Video
	CHAR_INFO GetColour(float lum);

public:
	Engine3D();

	bool OnUserCreate() override;
	bool OnUserUpdate(float fElapsedTime) override;
};


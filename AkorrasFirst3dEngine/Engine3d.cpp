#include "Engine3d.h"

// Define our static variables
std::atomic<bool> ConsoleGameEngine::m_bAtomActive(false);
std::condition_variable ConsoleGameEngine::m_cvGameFinished;
std::mutex ConsoleGameEngine::m_muxGame;

Engine3D::Engine3D()
{
	m_sAppName = L"3D Demo";
}

CHAR_INFO Engine3D::GetColour(float lum)
{
	short bg_col, fg_col;
	wchar_t sym;
	int pixel_bw = (int)(13.0f * lum);
	switch (pixel_bw)
	{
	case 0: bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID; break;

	case 1: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_QUARTER; break;
	case 2: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_HALF; break;
	case 3: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_THREEQUARTERS; break;
	case 4: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_SOLID; break;

	case 5: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_QUARTER; break;
	case 6: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_HALF; break;
	case 7: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_THREEQUARTERS; break;
	case 8: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_SOLID; break;

	case 9:  bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_QUARTER; break;
	case 10: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_HALF; break;
	case 11: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_THREEQUARTERS; break;
	case 12: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_SOLID; break;
	default:
		bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID;
	}

	CHAR_INFO c;
	c.Attributes = bg_col | fg_col;
	c.Char.UnicodeChar = sym;
	return c;
}

bool Engine3D::OnUserCreate() 
{
	meshCube.LoadFromObjectFile("Assets/spaceship.obj");

	//Projection Matrix
	float fNear = 0.1f;
	float fFar = 1000.0f;
	float fFoV = 90.0f;
	float fAspectRatio = (float)ScreenHeight() / (float)ScreenWidth();
	float fFoVRad = 1.0f / tanf(fFoV * 0.5f / 180.0f * 3.14159f);

	matProj.m[0][0] = fAspectRatio * fFoVRad;
	matProj.m[1][1] = fFoVRad;
	matProj.m[2][2] = fFar / (fFar - fNear);
	matProj.m[2][3] = 1.0f;
	matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
	matProj.m[3][3] = 0.0f;

	return true;
}

bool Engine3D::OnUserUpdate(float fElapsedTime) 
{
	Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

	// Set up rotation matrices
	mat4x4 matRotZ, matRotX;
	fTheta += 1.0f * fElapsedTime;

	// Rotation Z
	matRotZ.m[0][0] = cosf(fTheta);
	matRotZ.m[0][1] = sinf(fTheta);
	matRotZ.m[1][0] = -sinf(fTheta);
	matRotZ.m[1][1] = cosf(fTheta);
	matRotZ.m[2][2] = 1;
	matRotZ.m[3][3] = 1;

	// Rotation X
	matRotX.m[0][0] = 1;
	matRotX.m[1][1] = cosf(fTheta * 0.5f);
	matRotX.m[1][2] = sinf(fTheta * 0.5f);
	matRotX.m[2][1] = -sinf(fTheta * 0.5f);
	matRotX.m[2][2] = cosf(fTheta * 0.5f);
	matRotX.m[3][3] = 1;

	std::vector<triangle> vecTrianglesToRaster;

	// Draw Triangles
	for (const auto& tri : meshCube.tris)
	{
		triangle triProjected, triTranslated;

		// Rotate in Z-Axis
		triangle triRotatedZ = matRotZ * tri;

		// Rotate in X-Axis
		triangle triRotatedZX = matRotX * triRotatedZ;

		// Offset into the screen
		triTranslated = triRotatedZX + vec3d(0.0f, 0.0f ,8.0f);

		vec3d normal, line1, line2;
		line1 = triTranslated.p[1] - triTranslated.p[0];
		line2 = triTranslated.p[2] - triTranslated.p[0];

		normal = line1.cross(line2).normal();

		//if (normal.z < 0)
		if ((normal.x * triTranslated.p[0].x - vCamera.x) +
			(normal.y * triTranslated.p[0].y - vCamera.y) +
			(normal.z * triTranslated.p[0].z - vCamera.z) < 0.0f)
		{
			//Ilumination
			vec3d light_direction = { 0.0f, 0.0f, -1.0f };
			float l = sqrt(light_direction.x * light_direction.x + light_direction.y * light_direction.y + light_direction.z * light_direction.z);
			light_direction.x /= l; light_direction.y /= l; light_direction.z /= l;

			//dot product between normal of tri urface and light source
			float dp = normal.x * light_direction.x + normal.y * light_direction.y + normal.z * light_direction.z;

			CHAR_INFO c = GetColour(dp);
			triTranslated.col = c.Attributes;
			triTranslated.sym = c.Char.UnicodeChar;

			triProjected     = matProj*triTranslated;
			triProjected.col = triTranslated.col;
			triProjected.sym = triTranslated.sym;

			//Scale into View
			triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1;
			triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1;
			triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1;

			triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
			triProjected.p[0].y *= 0.5f * (float)ScreenHeight();
			triProjected.p[1].x *= 0.5f * (float)ScreenWidth();
			triProjected.p[1].y *= 0.5f * (float)ScreenHeight();
			triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
			triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

			// Store triangles for sorting 
			vecTrianglesToRaster.push_back(triProjected);
		}
	}

	std::sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2)
		{
			float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
			float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;

			return z1 > z2;
		});

	for (auto& triProjected : vecTrianglesToRaster)
	{
		//Rasterize triangle
		FillTriangle(triProjected.p[0].x, triProjected.p[0].y,
			triProjected.p[1].x, triProjected.p[1].y,
			triProjected.p[2].x, triProjected.p[2].y,
			triProjected.sym, triProjected.col);

		DrawTriangle(triProjected.p[0].x, triProjected.p[0].y,
			triProjected.p[1].x, triProjected.p[1].y,
			triProjected.p[2].x, triProjected.p[2].y,
			PIXEL_SOLID, FG_CYAN);
	}
	return true;
}
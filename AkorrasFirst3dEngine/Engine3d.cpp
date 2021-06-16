#include "Engine3d.h"

// Define our static variables
std::atomic<bool> ConsoleGameEngine::m_bAtomActive(false);
std::condition_variable ConsoleGameEngine::m_cvGameFinished;
std::mutex ConsoleGameEngine::m_muxGame;

Engine3D::Engine3D()
{
	m_sAppName = L"3D Demo";
	vCamera = vec3d();

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
	meshCube.LoadFromObjectFile("Assets/teapot.obj");

	//Projection Matrix
	matProj = mat4x4::Projection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1f, 1000.0f);
	return true;
}

bool Engine3D::OnUserUpdate(float fElapsedTime) 
{
	vec3d vForward = vLookDir * (8.0f * fElapsedTime);
	vec3d vRight = vLookDir.normalise().cross({0,1,0}) * (8.0f * fElapsedTime);

	if (GetKey(VK_UP).bHeld)
		vCamera.y += 8.0f * fElapsedTime;
	if (GetKey(VK_DOWN).bHeld)
		vCamera.y -= 8.0f * fElapsedTime;
	if (GetKey(VK_LEFT).bHeld)
		vCamera -= vRight; 
	if (GetKey(VK_RIGHT).bHeld)
		vCamera += vRight;
	
	if (GetKey(L'W').bHeld)
		vCamera += vForward;
	if (GetKey(L'S').bHeld)
		vCamera -= vForward;

	if (GetKey(L'A').bHeld)
		fYaw -= 2.0f * fElapsedTime;
	if (GetKey(L'D').bHeld)
		fYaw += 2.0f * fElapsedTime;

	fTheta = 0.0f;

	// Rotation Z
	mat4x4 matRotZ = mat4x4::RotationZ(fTheta * 0.5f);

	// Rotation X
	mat4x4 matRotX = mat4x4::RotationX(fTheta);

	// Translation matrix
	mat4x4 matTrans = mat4x4::Translation(0.0f, 0.0f, 5.0f);

	// World Matrix
	mat4x4 matWorld(1.0f);
	matWorld = matRotZ * matRotX;		//rotate around origin
	matWorld = matWorld * matTrans;		//translate to another location

	vec3d vUp     = { 0,1,0 };
	vec3d vTarget = { 0,0,1 };
	
	mat4x4 matCameraRot = mat4x4::RotationY(fYaw);
	vLookDir = matCameraRot * vTarget;
	vTarget  = vCamera + vLookDir;


	mat4x4 matCamera = mat4x4::PointAt(vCamera, vTarget, vUp);

	// Make view Matrix from camera
	mat4x4 matView = matCamera.Inverse();

	std::vector<triangle> vecTrianglesToRaster;

	// Draw Triangles
	for (const auto& tri : meshCube.tris)
	{
		triangle triProjected, triTransformed, triViewed;

		triTransformed.p[0] = matWorld * tri.p[0];
		triTransformed.p[1] = matWorld * tri.p[1];
		triTransformed.p[2] = matWorld * tri.p[2];

		vec3d normal, line1, line2;
		line1 = triTransformed.p[1] - triTransformed.p[0];
		line2 = triTransformed.p[2] - triTransformed.p[0];

		normal = line1.cross(line2).normalise();

		//cast ray from triangle to camera to see if it is visible
		vec3d vCameraRay = triTransformed.p[0] - vCamera;
		
		// if ray is aligned with normal, then triangle is visible
		if ( normal.dot(vCameraRay) < 0.0f)
		{
			//Ilumination
			vec3d light_direction = { 0.0f, 1.0f, -1.0f };
			light_direction = light_direction.normalise();

			//how aligned are light direction and triangle surface normal
			float dp = max(0.1f, light_direction.dot(normal));

			//Choose console colours as required 
			CHAR_INFO c = GetColour(dp);
			triTransformed.col = c.Attributes;
			triTransformed.sym = c.Char.UnicodeChar;

			//Convert worls space to viewed space
			triViewed.p[0] = matView * triTransformed.p[0];
			triViewed.p[1] = matView * triTransformed.p[1];
			triViewed.p[2] = matView * triTransformed.p[2];
			triViewed.col = triTransformed.col;
			triViewed.sym = triTransformed.sym;

			// Clip Viewd Triangle against near plane
			int nClippedTriangles = 0;
			triangle clipped[2];
			nClippedTriangles = ClipTriangleAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);
		
			for (int n = 0; n < nClippedTriangles; n++)
			{
				// Project Triangles from 3D --> 2D
				triProjected.p[0] = matProj * clipped[n].p[0];
				triProjected.p[1] = matProj * clipped[n].p[1];
				triProjected.p[2] = matProj * clipped[n].p[2];
				triProjected.col = clipped[n].col;
				triProjected.sym = clipped[n].sym;

				//normalize result due to 4th element of vector
				triProjected.p[0] = triProjected.p[0] / triProjected.p[0].w;
				triProjected.p[1] = triProjected.p[1] / triProjected.p[1].w;
				triProjected.p[2] = triProjected.p[2] / triProjected.p[2].w;

				// X/Y are inverted so put them back
				triProjected.p[0].x *= -1.0f;
				triProjected.p[1].x *= -1.0f;
				triProjected.p[2].x *= -1.0f;
				triProjected.p[0].y *= -1.0f;
				triProjected.p[1].y *= -1.0f;
				triProjected.p[2].y *= -1.0f;

				//Scale into View
				vec3d vOffsetView = { 1, 1, 0 };
				triProjected.p[0] += vOffsetView;
				triProjected.p[1] += vOffsetView;
				triProjected.p[2] += vOffsetView;

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
	}

	std::sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2)
	{
		float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
		float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
		return z1 > z2;
	});

	//Clear Screen
	Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

	for (auto& triToRaster : vecTrianglesToRaster)
	{
		// Clip Triangles against all four screen edges, this could yield many triangles
		triangle clipped[2];
		std::list<triangle> listTriangles;

		listTriangles.push_back(triToRaster);
		int nNewTriangles = 1;

		for (int p = 0; p < 4; p++)
		{
			int nTrisToAdd = 0;
			while (nNewTriangles > 0)
			{
				triangle test = listTriangles.front();
				listTriangles.pop_front();
				nNewTriangles--;

				switch(p)
				{
				case 0: nTrisToAdd = ClipTriangleAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break; //bottom view plane
				case 1: nTrisToAdd = ClipTriangleAgainstPlane({ 0.0f, (float)ScreenHeight()-1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break; //top view plane
				case 2: nTrisToAdd = ClipTriangleAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				case 3: nTrisToAdd = ClipTriangleAgainstPlane({ (float)ScreenWidth()-1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				}

				for (int w = 0; w < nTrisToAdd; w++)
					listTriangles.push_back(clipped[w]);
			}

			nNewTriangles = listTriangles.size();

			for (auto& t : listTriangles)
			{
				//Rasterize triangle
				FillTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, t.sym, t.col);
			}
		}		
	}
	return true;
}
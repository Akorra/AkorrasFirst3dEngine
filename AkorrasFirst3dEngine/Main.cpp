#include "Engine3d.h"

int main()
{
	Engine3D demo;
	if (demo.ConstructConsole(256, 240, 4, 4))
		demo.Start();

	return 0;
}
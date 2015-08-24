#include "Application.h"
#include <Windows.h>

// The MAIN function, from here we start the application and run the game loop
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
//	LPSTR lpCmdLine, int nCmdShow)
//{
//	Application app("LightEngine Demo", 800, 600);
//	return app.Run();
//}

int main()
{
	Application app("LightEngine Demo", 800, 600);
	return app.Run();
}
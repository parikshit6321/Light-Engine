#pragma once

/*
	Application is the main class whose instance defines
	how and what the game/demo is going to do.
*/
class Application
{

// Functions

public:
	Application(const char* title, int width, int height);
	int Run();
	~Application();

private:
	bool InitEngine();
	bool InitGLFW();
	bool InitGLEW();

	void Update(float deltaTime);
	void Render();
	void Shutdown();
	void CalculateFPS(int noOfFrames, float interval);

// Variables

private:
	const char*		appTitle;
	int				appWidth;
	int				appHeight;
	float			fps;
};
#pragma once

class Application
{
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

private:
	const char*		appTitle;
	int				appWidth;
	int				appHeight;
	float			fps;
};


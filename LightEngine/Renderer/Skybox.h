#pragma once

// Includes.
#include "..\Contrib\Include\gl\glew.h"
#include "..\Contrib\Include\SOIL.h"
#include "..\Util\Utility.h"
#include "..\Util\Shader.h"
#include <vector>

/*
	Skybox class holds the render data for
	the skybox cube and also, has the texture
	that is to be cube mapped onto it.
*/
class Skybox
{
public:

// Functions

	Skybox();
	void SetupSkybox(void);
	void LoadCubemap(std::vector<const GLchar*> faces);
	void Render(Shader shader);
	~Skybox();

// Variables

	GLuint skyboxVAO, skyboxVBO;
	GLuint cubemapTexture;
};
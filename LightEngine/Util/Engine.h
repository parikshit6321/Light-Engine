#pragma once

// Includes
#include "..\Contrib\Include\gl\glew.h"
#include "..\Contrib\Include\GLFW\glfw3.h"
#include "Utility.h"
#include "Shader.h"
#include "Camera.h"
#include "..\Contrib\Include\SOIL.h"
#include "..\Contrib\Include\assimp\Importer.hpp"
#include "..\Renderer\Mesh.h"
#include "..\Renderer\Model.h"
#include "TextRenderer.h"
#include "..\Renderer\ParticleSystem.h"
#include "..\Renderer\Skybox.h"
#include "..\Renderer\RenderObject.h"

// Linking libraries
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "SOIL.lib")
#pragma comment(lib, "assimp.lib")
#pragma comment(lib, "freetype26d.lib")

// #defines for conditional compiling
#define DEBUG
//#define RENDER_MODELS
#define RENDER_PARTICLES
#define RENDER_ENVIRONMENT_CUBE
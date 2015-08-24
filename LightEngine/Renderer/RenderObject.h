#pragma once

// Includes
#include "..\Contrib\Include\gl\glew.h"
#include "..\Contrib\Include\SOIL.h"
#include "..\Util\Shader.h"

/*
	Class to enable rendering of custom
	objects that are manually created.
	Contains the Vertex Array and Buffer
	required for rendering the custom
	object. Stores texture IDs for the
	diffuse, normal and specular textures
	and also, holds all the vertex Data
	and the number of triangles that are
	to be drawn.
*/
class RenderObject
{
public:

// Functions

	RenderObject(const char* diffusePath, const char* normalPath, const char* specularPath, GLfloat objectVertexData[],
		GLuint objectVAO, GLuint objectVBO, GLuint numOfTriangles = 2);
	void SetupBuffers(GLuint objectVAO, GLuint objectVBO);
	void SetupTextures(const char* diffusePath, const char* normalPath, const char* specularPath);
	void SetupVertexData(GLfloat objectVertexData[]);
	void RenderObject::Render(Shader shader, bool renderTextures = true);
	~RenderObject();

// Variables

	GLuint		VAO, VBO;
	GLuint		diffuse, normal, specular;
	GLfloat *	vertexData;
	GLuint		triangles;
};
#pragma once

// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;

// GL and GLM Includes
#include "..\Contrib\Include\gl\glew.h"
#include "..\Contrib\Include\glm\glm.hpp"
#include "..\Contrib\Include\glm\gtc\matrix_transform.hpp"
#include "..\Contrib\Include\assimp\Importer.hpp"
#include "..\Util\Shader.h"

/*
	Structure to hold vertex data
	for all the vertices in the mesh.
*/
struct Vertex {
	// Position
	glm::vec3 Position;
	// Normal
	glm::vec3 Normal;
	// TexCoords
	glm::vec2 TexCoords;
};

/*
	Structure to hold current texture state.
	id		- current texture id for the mapped texture.
	type	- type of the texture : diffuse, reflection or specular.
	path	- to determine if the texture has already been loaded.
*/
struct Texture {
	GLuint id;
	string type;
	aiString path;
};

/*
	Mesh class that holds vertex data including
	vertices, indices and textures. Each mesh
	of the entire model is rendered one by one
	using the Mesh class' Draw Function.
*/
class Mesh {
public:

// Variables

	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;

// Functions

	Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures);
	void Draw(Shader shader);

private:

// Variables

	// Buffers containing the vertex data.
	GLuint VAO, VBO, EBO;

// Functions

	void setupMesh();
};
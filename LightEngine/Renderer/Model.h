#pragma once

// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

// Contrib Includes
#include "..\Contrib\Include\gl\glew.h"
#include "..\Contrib\Include\glm\glm.hpp"
#include "..\Contrib\Include\glm\gtc\matrix_transform.hpp"
#include "..\Contrib\Include\SOIL.h"
#include "..\Contrib\Include\assimp\Importer.hpp"
#include "..\Contrib\Include\assimp\scene.h"
#include "..\Contrib\Include\assimp\postprocess.h"

#include "Mesh.h"

// Function prototypes.
GLint TextureFromFile(const char* path, string directory);

/*
	Model class that holds all the model data
	loaded using Assimp's Scene Importer. This
	class holds a vector of all the meshes that
	compose the model, the directory where the
	model is stored and a vector of all the
	textures that have been loaded so far so as
	to optimize the model loading function by not
	loading the same texture more than once.
*/
class Model
{
public:

// Functions

	Model(GLchar* path);
	void Draw(Shader shader);

private:

// Variables

	vector<Mesh> meshes;
	string directory;
	vector<Texture> textures_loaded;

// Functions

	void loadModel(string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);

};
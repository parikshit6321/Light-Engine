#include "RenderObject.h"

/*
	Constructor to create a custom object using manually generated data.

	diffusePath		-	Path to the diffuse texture in memory.
	normalPath		-	Path to the normal texture in memory.
	specularPath	-	Path to the specular texture in memory.
	objectVertexData-	Data containing the vertices, normals, texture
	co-ordinates, bitangents, tangents, etc.
	objectVAO		-	Vertex Array for the render data.
	objectVBO		-	Vertex Buffer populated with the relevant data.
	numOfTriangles	-	Number of triangles contained in the polygon
	to calculate the number of vertices that are to
	be passed in the glDrawArrays() call.
*/
RenderObject::RenderObject(const char* diffusePath, const char* normalPath, const char* specularPath, GLfloat objectVertexData[],
	GLuint objectVAO, GLuint objectVBO, GLuint numOfTriangles)
{
	SetupBuffers(objectVAO, objectVBO);
	SetupTextures(diffusePath, normalPath, specularPath);
	SetupVertexData(objectVertexData);

	triangles = numOfTriangles;

	log("RenderObject created successfully.");
}

/*
	Assign the Vertex Array and Vertex Buffers.

	objectVAO	-	Vertex Array bound with the Buffer Setup.
	objectVBO	-	Buffer populated with the vertex data.
*/
void RenderObject::SetupBuffers(GLuint objectVAO, GLuint objectVBO)
{
	VAO = objectVAO;
	VBO = objectVBO;
}

/*
	Generates and loads all the textures
	associated with the RenderObject.

	diffusePath		-	Path to the diffuse texture in memory.
	normalPath		-	Path to the normal texture in memory.
	specularPath	-	Path to the specular texture in memory.
*/
void RenderObject::SetupTextures(const char* diffusePath, const char* normalPath, const char* specularPath)
{
	int width, height;

	// Generating diffuse map for wall.
	glGenTextures(1, &diffuse);
	glBindTexture(GL_TEXTURE_2D, diffuse);

	// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load, create texture and generate mipmaps
	unsigned char * image = SOIL_load_image(diffusePath, &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Generating specular map for wall.
	glGenTextures(1, &specular);
	glBindTexture(GL_TEXTURE_2D, specular);

	// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load, create texture and generate mipmaps
	image = SOIL_load_image(specularPath, &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Generating normal map for wall.
	glGenTextures(1, &normal);
	glBindTexture(GL_TEXTURE_2D, normal);

	// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load, create texture and generate mipmaps
	image = SOIL_load_image(normalPath, &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	log("Textures loaded and set successfully.");
}

/*
	Load all the vertex data into our vertex array.

	objectVertexData	-	array of vertices, indices, normals, bitangents, tangents, etc.
*/
void RenderObject::SetupVertexData(GLfloat objectVertexData[])
{
	vertexData = new GLfloat[sizeof(objectVertexData)];

	for (int i = 0; i < (sizeof(objectVertexData) / sizeof(GLfloat)); ++i)
	{
		vertexData[i] = objectVertexData[i];
	}

	log("Vertex data loaded.");
}

/*
	First, bind all the relevant textures and then,
	use glDrawArrays to render the RenderObject.

	shader			-	Shader used in rendering the object.
	renderTextures	-	Flag to set whether to render textures or not.
*/
void RenderObject::Render(Shader shader, bool renderTextures)
{
	if (renderTextures)
	{
		// Bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuse);

		// Bind specular map
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specular);

		// Bind normal map
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, normal);
	}

	glDrawArrays(GL_TRIANGLES, 0, 3 * triangles);
}

/*
	Destructor to free memory held for vertex data.
*/
RenderObject::~RenderObject()
{

}
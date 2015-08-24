#include "Mesh.h"

/*
	Constructor that takes indexed vertex data and all the associated textures.

	vertices	-	Collection of all the vertex data.
	indices		-	Collection of all the index data.
	textures	-	Collection of the Texture structure variables.
*/
Mesh::Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	// Now that we have all the required data, set the vertex buffers and its attribute pointers.
	this->setupMesh();
}

/*
	Renders the mesh using Indexed Drawing.
	First loads and maps all the textures :
	diffuse, specular and reflection.
	Then, it binds the VAO containing the
	vertex data and uses glDrawElements().

	shader	-	Shader program that we use to render the mesh.
*/
void Mesh::Draw(Shader shader)
{
	// Bind appropriate textures
	GLuint diffuseNr = 1;
	GLuint specularNr = 1;
	GLuint reflectionNr = 1;

	for (GLuint i = 0; i < this->textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // Active proper texture unit before binding
		// Retrieve texture number (the N in diffuse_textureN)
		stringstream ss;
		string number;
		string name = this->textures[i].type;
		if (name == "texture_diffuse")
			ss << diffuseNr++;				// Transfer diffuseNr to stream
		else if (name == "texture_specular")
			ss << specularNr++;				// Transfer specularNr to stream
		else if (name == "texture_reflection")
			ss << reflectionNr++;			// Transfer reflectionNr to stream
		number = ss.str();
		// Now set the sampler to the correct texture unit
		glUniform1i(glGetUniformLocation(shader.program, (name + number).c_str()), i);

		// And finally bind the texture
		glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
	}
	glActiveTexture(GL_TEXTURE0);

	// Draw mesh
	glBindVertexArray(this->VAO);
	glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

/*
	Loads all the relevant vertex data into
	the buffers. Populates up the Vertex Array,
	Vertex Buffer and the Element Buffer for
	Indexed Drawing.
*/
void Mesh::setupMesh(void)
{
	// Create buffers/arrays
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);

	glBindVertexArray(this->VAO);

	// Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

	// Set the vertex attribute pointers

	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

	// Vertex Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));

	// Vertex Texture Coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));

	glBindVertexArray(0);

	log("Mesh Generated.");
}
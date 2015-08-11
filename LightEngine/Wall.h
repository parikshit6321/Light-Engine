#pragma once

#define BREADTH	5
#define LENGTH	5

#include "Wall.h"
#include "Shader.h"
#include "Contrib\Include\SOIL.h"
#include "Contrib\Include\gl\glew.h"

GLuint vertexData[] = {
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
	0.0f, 0.5f, 0.0f, 0.0f, 1.0f
};

class Wall
{
private:
	GLuint diffuse;
	GLuint specular;
	GLuint normal;
	GLuint VAO, VBO;
	GLuint program;

public:
	Wall(const char* diffuseTexture, const char* normalTexture)
	{
		Shader shader("Shaders/wall.vert", "Shaders/wall.frag");
		program = shader.program;

		int width, height;
		unsigned char* diffuseImage = SOIL_load_image(diffuseTexture, &width, &height, 0, SOIL_LOAD_RGB);

		glGenTextures(1, &diffuse);
		glGenTextures(1, &specular);
		glGenTextures(1, &normal);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			GL_LINEAR);
		// Set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, diffuse);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, diffuseImage);
		glGenerateMipmap(GL_TEXTURE_2D);

		SOIL_free_image_data(diffuseImage);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}
	void Draw()
	{
		glUseProgram(program);

		// Bind Texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuse);

		glUniform1i(glGetUniformLocation(program, "texture_diffuse"), 0);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		std::cout << "glDrawArrays() called.\n";
	}
	~Wall()
	{
	}
};
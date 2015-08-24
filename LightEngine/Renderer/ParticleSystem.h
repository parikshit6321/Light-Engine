#pragma once

// Includes
#include <iostream>
#include "..\Contrib\Include\gl\glew.h"
#include "..\Contrib\Include\glm\glm.hpp"
#include "..\Contrib\Include\glm\gtc\matrix_transform.hpp"
#include "..\Contrib\Include\glm\gtc\type_ptr.hpp"
#include "..\Contrib\Include\SOIL.h"
#include "..\Util\Shader.h"
#include "Particle.h"

/*
	A simplistic Particle System for the Light Engine.
	Renders all the particles as textured quads with an
	option of enabling billboarding for a 3D effect.
	
	Contains :
	
		- An array of Particle Structure variables.
		- Vertex Buffer and Vertex Array Data for the particles.
		- Texture that is to be mapped to the quads.
		- Number of particles to be rendered.
		- A billboarding flag which tells whether it is enabled or not.
		- Velocity and Gravity vectors for the particles taken from the user.
		- A customLife variable to vary the amount of time particles remain active.
		- Visibility to control the aplha blending of the particles with surroundings.
*/
class ParticleSystem
{
public:

// Functions

	ParticleSystem::ParticleSystem(const char* texturePath, GLuint count, bool fixedPosition, bool billboardingFlag = true,
		glm::vec3 particleVelocity = glm::vec3(0.0f, -100.0f, 0.0f), glm::vec3 particleGravity = glm::vec3(0.0f, -1.0f, 0.0f),
		GLfloat particleLife = 1.0f, glm::vec3 position = glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3 offsets = glm::vec3(30.0f, 1.0f, 30.0f));
	void InitParticleSystem(bool fixedPosition, glm::vec3 position, glm::vec3 offsets);
	void LoadParticleTexture(const char* texturePath);
	void SetupParticles(void);
	void Render(Shader particleShader, glm::vec3 viewDir, glm::vec3 particleScale = glm::vec3(1.0f));
	void Update(void);
	~ParticleSystem();

// Variables

	Particle *	particles;
	GLuint		particleQuadVAO, particleQuadVBO;
	GLuint		particleTexture;
	GLuint		particleCount;
	bool		billboarding;
	bool		fixedPoint;
	glm::vec3	velocity, gravity;
	glm::vec3	origin, offset;
	GLfloat		customLife;
	GLfloat		visibility;
};
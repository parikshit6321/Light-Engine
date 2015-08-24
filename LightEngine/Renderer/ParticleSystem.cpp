#include "ParticleSystem.h"

const float slowdown = 1.0f;			// Slow Down Particles

// Vertex data for all the particles (Rendered as textured quads).
GLfloat particleVertices[] = {
	// Positions   // TexCoords
	-1.0f, 1.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
	1.0f, -1.0f, 1.0f, 0.0f,

	-1.0f, 1.0f, 0.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 1.0f
};

/*
	Constructor that creates a Particle System suitable for rain, smoke, fire, etc.
	(Default settings are best suited for rain).

		texturePath		-	Path to the texture file in memory.
		count			-	Number of particles in the system.
		fixedPosition	-	If the particles are to originate
							from a fixed point or spread over
							an area.
		billboardingFlag-	To enable/disable billboarding effect.
		particleVelocity-	To set the initial particle velocity.
		particleGravity	-	To set the initial particle gravity.
		particleLife	-	To set the duration for which the 
							particle will remain active.
		position		-	To set the origin of the particles.
		offsets			-	To set the rectangular area from where
							particles will originate.
*/
ParticleSystem::ParticleSystem(const char* texturePath, GLuint count, bool fixedPosition, bool billboardingFlag,
	glm::vec3 particleVelocity, glm::vec3 particleGravity, GLfloat particleLife, glm::vec3 position, glm::vec3 offsets)
{
	fixedPoint = fixedPosition;

	origin = position;
	offset = offsets;

	visibility = 0.1f;

	particleCount = ((count < 0) ? 1000 : count);

	particles = new Particle[particleCount];

	billboarding = billboardingFlag;

	velocity = particleVelocity;
	gravity = particleGravity;
	customLife = particleLife;

	LoadParticleTexture(texturePath);
	InitParticleSystem(fixedPosition, position, offsets);
	SetupParticles();
}

/*
	Function that initiates the particle system
	by assigning the appropriate values passed
	during the object construction.

		fixedPosition	-	Whether the system has a fixed point
							origin or it is spread over an area.
		position		-	Fixed Point Origin for the particles.
		offsets			-	Defines the rectangular origin area.
*/
void ParticleSystem::InitParticleSystem(bool fixedPosition, glm::vec3 position, glm::vec3 offsets)
{
	for (GLint loop = 0; loop < particleCount; loop++)
	{
		// In case, particle system has a fixed point origin.
		if (fixedPosition)
		{
			particles[loop].x = position.x;
			particles[loop].y = position.y;
			particles[loop].z = position.z;
		}
		// In case, particle system is spread over a rectangular area.
		else
		{
			particles[loop].x = position.x + float(rand() % int(offsets.x));
			particles[loop].y = position.y + float(rand() % int(offsets.y));
			particles[loop].z = position.z + float(rand() % int(offsets.z));
		}

		particles[loop].active = true;
		particles[loop].life = customLife;
		particles[loop].fade = float(rand() % 100) / 1000.0f + 0.003f;
		particles[loop].xi = velocity.x;
		particles[loop].yi = velocity.y;
		particles[loop].zi = velocity.z;
		particles[loop].xg = gravity.x;
		particles[loop].yg = gravity.y;
		particles[loop].zg = gravity.z;
	}

	log("Particle System initiailized successfully.");
}

/*
	Function to generate the particle's texture
	and load it into the binded texture. It also
	sets the appropriate filtering parameters.

	texturePath		-	Path to the image that is to
						be mapped to the particle quads.
*/
void ParticleSystem::LoadParticleTexture(const char* texturePath)
{
	// Load texture for particle system.
	glGenTextures(1, &particleTexture);
	glBindTexture(GL_TEXTURE_2D, particleTexture);

	// Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load, create texture and generate mipmaps
	int width, height;
	unsigned char * image = SOIL_load_image(texturePath, &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	log("Particle Texture loaded successfully.");
}

/*
	Generates the render data for the quads.
	Populates the Vertex Buffer and sets the
	Vertex Array.
*/
void ParticleSystem::SetupParticles(void)
{
	glGenVertexArrays(1, &particleQuadVAO);
	glGenBuffers(1, &particleQuadVBO);
	glBindVertexArray(particleQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, particleQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particleVertices), &particleVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glBindVertexArray(0);
}

/*
	Function to finally render the particle system.
	This sets all the relevant uniforms and then,
	uses the vertex and position data for the quads.

		particleShader	-	Shader that is used to render the particles.
		viewDir			-	Forward direction of the camera.
		(Required for billboarding)
		particleScale	-	To scale the particles as per User's wish.
*/
void ParticleSystem::Render(Shader particleShader, glm::vec3 viewDir, glm::vec3 particleScale)
{
	// Set the visibility for alpha blending.
	glUniform1f(glGetUniformLocation(particleShader.program, "visibility"), visibility);

	// Bind the particle texture.
	glUniform1i(glGetUniformLocation(particleShader.program, "particleTexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, particleTexture);

	// Find out the model matrix location.
	GLint modelLocation = glGetUniformLocation(particleShader.program, "model");
	float angle = 0.0f;

	// Iterate through all the particles.
	for (GLint loop = 0; loop < particleCount; loop++)
	{
		// If particle is still active.
		if (particles[loop].active)
		{
			float x = particles[loop].x;
			float y = particles[loop].y;
			float z = particles[loop].z;

			// Determine the angle with which the particle has to be rotated for billboarding.
			angle = glm::acos(glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), glm::normalize(viewDir)));

			// Generate the model matrix for the current particle.
			glm::mat4 model;
			model = glm::translate(model, glm::vec3(-2.5f, 0.0f, -20.0f) + glm::vec3(x, y, z)); // Translate it down a bit so it's at the center of the scene
			model = glm::scale(model, particleScale);

			// If billboarding is enabled, rotate the particle.
			if (billboarding)
			{
				model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
			}

			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

			// Render the particle using glDrawArrays().
			glBindVertexArray(particleQuadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}
	}
}

/*
	Updates the world-space position of all the particles.
	Also instantiates new particles to replace dead ones.
*/
void ParticleSystem::Update(void)
{
	// Iterate through all the particles.
	for (GLint loop = 0; loop < particleCount; loop++)
	{
		// If The Particle Is Active
		if (particles[loop].active)
		{
			// Update the particle's position based on it's speed.
			particles[loop].x += particles[loop].xi / (slowdown * 1000);
			particles[loop].y += particles[loop].yi / (slowdown * 1000);
			particles[loop].z += particles[loop].zi / (slowdown * 1000);

			// Update the particle's speed based on the gravity.
			particles[loop].xi += particles[loop].xg;
			particles[loop].yi += particles[loop].yg;
			particles[loop].zi += particles[loop].zg;

			// Use the fade factor to reduce particle's life.
			particles[loop].life -= particles[loop].fade;

			// If particle has died.
			if (particles[loop].life<0.0f)
			{
				particles[loop].life = customLife;
				particles[loop].fade = float(rand() % 100) / 1000.0f + 0.003f;

				// In case, particle system has a fixed point origin.
				if (fixedPoint)
				{
					particles[loop].x = origin.x;
					particles[loop].y = origin.y;
					particles[loop].z = origin.z;
				}
				// In case, particle system is spread over a rectangular area.
				else
				{
					particles[loop].x = origin.x + float(rand() % int(offset.x));
					particles[loop].y = origin.y + float(rand() % int(offset.y));
					particles[loop].z = origin.z + float(rand() % int(offset.z));
				}

				particles[loop].xi = velocity.x;
				particles[loop].yi = velocity.y;
				particles[loop].zi = velocity.z;
			}
		}
	}
}

/*
	Destructor to clear memory occupied by the particles array.
*/
ParticleSystem::~ParticleSystem()
{
	delete[] particles;

	log("Particle System destructed.");
}
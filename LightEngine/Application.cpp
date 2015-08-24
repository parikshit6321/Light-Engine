#include "Application.h"
#include "Util\Engine.h"

// Variable to toggle the Point Light.
int pointLightOn = 0;

float	xspeed;			
float	yspeed;			
float	zoom = -40.0f;

// Global namespace to hold the Window struct object.
namespace
{
	GLFWwindow* appWindow;
	GLuint VBO, VAO;

	Shader* currentShader;
}

// Variable to toggle the Flash Light.
GLuint flashLight = 0;

// Camera
Camera camera(glm::vec3(1.0f, 0.0f, -1.0f));

// Variables relevant for user input handling.
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

/*
	Callback function that is called whenever GLFW
	handles any keyboard event : key press/key release.
*/
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	//cout << key << endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

/*
	Callback function that is called by GLFW's
	event handler whenever any mouse event occurs.
*/
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

/*
	Callback function which is called by GLFW's
	event handler whenever mouse scroll is used.
*/
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

/*
	Generates a multi-sampled texture for MSAA Anti-Aliasing.

	samples		-	Number of samples to use.
*/
GLuint generateMultiSampleTexture(GLuint samples)
{
	GLuint texture;
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA, 800, 600, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	return texture;
}

/*
	Generates a texture that is suited for attachments to a framebuffer.

	depth	-	Should the texture has a depth buffer too?
	stencil	-	Should the texture has a stencil buffer too?
*/
GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencil)
{
	// What enum to use?
	GLenum attachment_type;
	if (!depth && !stencil)
		attachment_type = GL_RGBA;
	else if (depth && !stencil)
		attachment_type = GL_DEPTH_COMPONENT;
	else if (!depth && stencil)
		attachment_type = GL_STENCIL_INDEX;

	//Generate texture ID and load texture data 
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	if (!depth && !stencil)
		glTexImage2D(GL_TEXTURE_2D, 0, attachment_type, 800, 600, 0, attachment_type, GL_UNSIGNED_BYTE, NULL);
	else // Using both a stencil and depth test, needs special format arguments
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

/*
	Constructor to initialize the application.

	title	-	Title of the application.
	width	-	Width of the viewport.
	height	-	Height of the viewport.
*/
Application::Application(const char* title, int width, int height)
{
	appTitle = title;
	appWidth = width;
	appHeight = height;
}

/*
	Initializes a GLFW window, enables multi-sampling
	and sets the callback functions for event handling.
*/
bool Application::InitGLFW()
{
#ifdef DEBUG
	log("Starting GLFW context, OpenGL 3.3.");
#endif
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	glEnable(GL_MULTISAMPLE); // Enabled by default on some drivers, but not all so always enable to make sure

	// Create a GLFWwindow object that we can use for GLFW's functions
	appWindow = glfwCreateWindow(appWidth, appHeight, appTitle, nullptr, nullptr);
	//appWindow = glfwCreateWindow(appWidth, appHeight, appTitle, glfwGetPrimaryMonitor(), NULL);
	if (appWindow == nullptr)
	{
	#ifdef DEBUG
		log("Failed to create GLFW window.");
	#endif
		
		glfwTerminate();
		return false;
	}
	else
	{
	#ifdef DEBUG
		log("GLFW Window created successfully.");
	#endif
		
	}
	glfwMakeContextCurrent(appWindow);
	// Set the required callback functions
	glfwSetKeyCallback(appWindow, key_callback);

	return true;
}

/*
	Initializes the GLEW library for OpenGL extensions.
*/
bool Application::InitGLEW()
{
	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (glewInit() != GLEW_OK)
	{
	#ifdef DEBUG
		log("Failed to initialize GLEW.");
	#endif
		return false;
	}
	else
	{
	#ifdef DEBUG
		log("GLEW initialized successfully.");
	#endif
	}

	return true;
}

/*
	Initializes the entire engine by first calling
	InitUtility() then, InitGLFW(), InitGLEW(),
	TextRenderer::Init() and enables blending.
*/
bool Application::InitEngine()
{
	InitUtility();

	log("===Initializing Engine===");

#ifdef DEBUG
	log("Utilities initialized successfully.");
#endif

	if (!InitGLFW())
		return false;
	if (!InitGLEW())
		return false;
	if (!TextRenderer::Init(appWidth, appHeight))
		return false;

	// Define the viewport dimensions
	glViewport(0, 0, appWidth, appHeight);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}

/*
	Function which is finally called to execute the application.
*/
int Application::Run()
{
	// Initialize the engine.
	if (!InitEngine())
		return 1;

#ifdef DEBUG
	log("Engine intialization complete.");
#endif

	// Create a rain particle system.
	ParticleSystem rain("Textures/Particle.bmp", 1000, false);

	float prevTime = 0.0f;
	float currTime = 0.0f;
	float deltaTime = 0.0f;

	prevTime = getTimeElapsed();

	// Set the required callback functions
	glfwSetKeyCallback(appWindow, key_callback);
	glfwSetCursorPosCallback(appWindow, mouse_callback);
	glfwSetScrollCallback(appWindow, scroll_callback);

#ifdef DEBUG
	log("Callback functions successfully set.");
#endif
	
	// Options
	glfwSetInputMode(appWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	log("");
	log("===Basic Shader===");
	Shader ourShader("Shaders/basic.vert", "Shaders/basic.frag");

	// World space positions of our walls
	glm::vec3 wallTranslations[] = {
		glm::vec3(0.0f, 0.0f, 0.0f),	// Front wall.
		glm::vec3(5.0f, 0.0f, 0.0f),
		glm::vec3(10.0f, 0.0f, 0.0f),
		glm::vec3(15.0f, 0.0f, 0.0f),
		glm::vec3(20.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -25.0f),	// Back wall.
		glm::vec3(5.0f, 0.0f, -25.0f),
		glm::vec3(10.0f, 0.0f, -25.0f),
		glm::vec3(15.0f, 0.0f, -25.0f),
		glm::vec3(20.0f, 0.0f, -25.0f),
		glm::vec3(-2.5f, -2.5f, 0.0f),	// Left wall.
		glm::vec3(-2.5f, -2.5f, -5.0f),
		glm::vec3(-2.5f, -2.5f, -10.0f),
		glm::vec3(-2.5f, -2.5f, -15.0f),
		glm::vec3(-2.5f, -2.5f, -20.0f),
		glm::vec3(22.5f, -2.5, 0.0f),	// Right wall.
		glm::vec3(22.5f, -2.5f, -5.0f),	
		glm::vec3(22.5f, -2.5f, -10.0f),
		glm::vec3(22.5f, -2.5f, -15.0f),
		glm::vec3(22.5f, -2.5f, -20.0f)
	};

	// World space rotations of our walls
	GLfloat wallRotations[] = {
		180.0f,	// Front wall.
		180.0f,
		180.0f,
		180.0f,
		180.0f,
		0.0f,	// Back wall.
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		0.0f,	// Left wall.
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		180.0f,	// Right wall.
		180.0f,
		180.0f,
		180.0f,
		180.0f
	};

	// For back wall
	// positions
	glm::vec3 wall_pos1(-0.5, 0.5, 0.0);
	glm::vec3 wall_pos2(-0.5, -0.5, 0.0);
	glm::vec3 wall_pos3(0.5, -0.5, 0.0);
	glm::vec3 wall_pos4(0.5, 0.5, 0.0);
	// texture coordinates
	glm::vec2 wall_uv1(0.0, 1.0);
	glm::vec2 wall_uv2(0.0, 0.0);
	glm::vec2 wall_uv3(1.0, 0.0);
	glm::vec2 wall_uv4(1.0, 1.0);
	// normal vector
	glm::vec3 wall_nm(0.0, 0.0, 1.0);

	// calculate tangent/bitangent vectors of both triangles
	glm::vec3 wall_tangent1, wall_bitangent1;
	glm::vec3 wall_tangent2, wall_bitangent2;
	// - triangle 1
	glm::vec3 wall_edge1 = wall_pos2 - wall_pos1;
	glm::vec3 wall_edge2 = wall_pos3 - wall_pos1;
	glm::vec2 wall_deltaUV1 = wall_uv2 - wall_uv1;
	glm::vec2 wall_deltaUV2 = wall_uv3 - wall_uv1;

	GLfloat wall_f = 1.0f / (wall_deltaUV1.x * wall_deltaUV2.y - wall_deltaUV2.x * wall_deltaUV1.y);

	wall_tangent1.x = wall_f * (wall_deltaUV2.y * wall_edge1.x - wall_deltaUV1.y * wall_edge2.x);
	wall_tangent1.y = wall_f * (wall_deltaUV2.y * wall_edge1.y - wall_deltaUV1.y * wall_edge2.y);
	wall_tangent1.z = wall_f * (wall_deltaUV2.y * wall_edge1.z - wall_deltaUV1.y * wall_edge2.z);
	wall_tangent1 = glm::normalize(wall_tangent1);

	wall_bitangent1.x = wall_f * (-wall_deltaUV2.x * wall_edge1.x + wall_deltaUV1.x * wall_edge2.x);
	wall_bitangent1.y = wall_f * (-wall_deltaUV2.x * wall_edge1.y + wall_deltaUV1.x * wall_edge2.y);
	wall_bitangent1.z = wall_f * (-wall_deltaUV2.x * wall_edge1.z + wall_deltaUV1.x * wall_edge2.z);
	wall_bitangent1 = glm::normalize(wall_bitangent1);

	// - triangle 2
	wall_edge1 = wall_pos3 - wall_pos1;
	wall_edge2 = wall_pos4 - wall_pos1;
	wall_deltaUV1 = wall_uv3 - wall_uv1;
	wall_deltaUV2 = wall_uv4 - wall_uv1;

	wall_f = 1.0f / (wall_deltaUV1.x * wall_deltaUV2.y - wall_deltaUV2.x * wall_deltaUV1.y);

	wall_tangent2.x = wall_f * (wall_deltaUV2.y * wall_edge1.x - wall_deltaUV1.y * wall_edge2.x);
	wall_tangent2.y = wall_f * (wall_deltaUV2.y * wall_edge1.y - wall_deltaUV1.y * wall_edge2.y);
	wall_tangent2.z = wall_f * (wall_deltaUV2.y * wall_edge1.z - wall_deltaUV1.y * wall_edge2.z);
	wall_tangent2 = glm::normalize(wall_tangent2);


	wall_bitangent2.x = wall_f * (-wall_deltaUV2.x * wall_edge1.x + wall_deltaUV1.x * wall_edge2.x);
	wall_bitangent2.y = wall_f * (-wall_deltaUV2.x * wall_edge1.y + wall_deltaUV1.x * wall_edge2.y);
	wall_bitangent2.z = wall_f * (-wall_deltaUV2.x * wall_edge1.z + wall_deltaUV1.x * wall_edge2.z);
	wall_bitangent2 = glm::normalize(wall_bitangent2);


	GLfloat wall_vertices[] = {
		// Positions            // normal         // TexCoords  // Tangent                          // Bitangent
		wall_pos1.x, wall_pos1.y, wall_pos1.z, wall_nm.x, wall_nm.y, wall_nm.z, wall_uv1.x, wall_uv1.y, wall_tangent1.x, wall_tangent1.y, wall_tangent1.z, wall_bitangent1.x, wall_bitangent1.y, wall_bitangent1.z,
		wall_pos2.x, wall_pos2.y, wall_pos2.z, wall_nm.x, wall_nm.y, wall_nm.z, wall_uv2.x, wall_uv2.y, wall_tangent1.x, wall_tangent1.y, wall_tangent1.z, wall_bitangent1.x, wall_bitangent1.y, wall_bitangent1.z,
		wall_pos3.x, wall_pos3.y, wall_pos3.z, wall_nm.x, wall_nm.y, wall_nm.z, wall_uv3.x, wall_uv3.y, wall_tangent1.x, wall_tangent1.y, wall_tangent1.z, wall_bitangent1.x, wall_bitangent1.y, wall_bitangent1.z,

		wall_pos1.x, wall_pos1.y, wall_pos1.z, wall_nm.x, wall_nm.y, wall_nm.z, wall_uv1.x, wall_uv1.y, wall_tangent2.x, wall_tangent2.y, wall_tangent2.z, wall_bitangent2.x, wall_bitangent2.y, wall_bitangent2.z,
		wall_pos3.x, wall_pos3.y, wall_pos3.z, wall_nm.x, wall_nm.y, wall_nm.z, wall_uv3.x, wall_uv3.y, wall_tangent2.x, wall_tangent2.y, wall_tangent2.z, wall_bitangent2.x, wall_bitangent2.y, wall_bitangent2.z,
		wall_pos4.x, wall_pos4.y, wall_pos4.z, wall_nm.x, wall_nm.y, wall_nm.z, wall_uv4.x, wall_uv4.y, wall_tangent2.x, wall_tangent2.y, wall_tangent2.z, wall_bitangent2.x, wall_bitangent2.y, wall_bitangent2.z
	};

	// Setup plane VAO
	GLuint VAO_WALL, VBO_WALL;
	glGenVertexArrays(1, &VAO_WALL);
	glGenBuffers(1, &VBO_WALL);
	glBindVertexArray(VAO_WALL);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_WALL);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall_vertices), &wall_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(11 * sizeof(GLfloat)));

	glBindVertexArray(0);

	log("");
	log("===Back Wall RenderObject===");
	RenderObject back_wall("Textures/wall_diffuse.bmp", "Textures/wall_normal.bmp", "Textures/wall_specular.bmp", wall_vertices, VAO_WALL, VBO_WALL);
	
	// For front wall
	// positions
	glm::vec3 front_wall_pos1(0.5, 0.5, 0.0);
	glm::vec3 front_wall_pos2(0.5, -0.5, 0.0);
	glm::vec3 front_wall_pos3(-0.5, -0.5, 0.0);
	glm::vec3 front_wall_pos4(-0.5, 0.5, 0.0);
	// texture coordinates
	glm::vec2 front_wall_uv1(0.0, 1.0);
	glm::vec2 front_wall_uv2(0.0, 0.0);
	glm::vec2 front_wall_uv3(1.0, 0.0);
	glm::vec2 front_wall_uv4(1.0, 1.0);
	// normal vector
	glm::vec3 front_wall_nm(0.0, 0.0, -1.0);

	// calculate tangent/bitangent vectors of both triangles
	glm::vec3 front_wall_tangent1, front_wall_bitangent1;
	glm::vec3 front_wall_tangent2, front_wall_bitangent2;
	// - triangle 1
	glm::vec3 front_wall_edge1 = front_wall_pos2 - front_wall_pos1;
	glm::vec3 front_wall_edge2 = front_wall_pos3 - front_wall_pos1;
	glm::vec2 front_wall_deltaUV1 = front_wall_uv2 - front_wall_uv1;
	glm::vec2 front_wall_deltaUV2 = front_wall_uv3 - front_wall_uv1;

	GLfloat front_wall_f = 1.0f / (front_wall_deltaUV1.x * front_wall_deltaUV2.y - front_wall_deltaUV2.x * front_wall_deltaUV1.y);

	front_wall_tangent1.x = front_wall_f * (front_wall_deltaUV2.y * front_wall_edge1.x - front_wall_deltaUV1.y * front_wall_edge2.x);
	front_wall_tangent1.y = front_wall_f * (front_wall_deltaUV2.y * front_wall_edge1.y - front_wall_deltaUV1.y * front_wall_edge2.y);
	front_wall_tangent1.z = front_wall_f * (front_wall_deltaUV2.y * front_wall_edge1.z - front_wall_deltaUV1.y * front_wall_edge2.z);
	front_wall_tangent1 = glm::normalize(front_wall_tangent1);

	front_wall_bitangent1.x = front_wall_f * (-front_wall_deltaUV2.x * front_wall_edge1.x + front_wall_deltaUV1.x * front_wall_edge2.x);
	front_wall_bitangent1.y = front_wall_f * (-front_wall_deltaUV2.x * front_wall_edge1.y + front_wall_deltaUV1.x * front_wall_edge2.y);
	front_wall_bitangent1.z = front_wall_f * (-front_wall_deltaUV2.x * front_wall_edge1.z + front_wall_deltaUV1.x * front_wall_edge2.z);
	front_wall_bitangent1 = glm::normalize(front_wall_bitangent1);

	// - triangle 2
	front_wall_edge1 = front_wall_pos3 - front_wall_pos1;
	front_wall_edge2 = front_wall_pos4 - front_wall_pos1;
	front_wall_deltaUV1 = front_wall_uv3 - front_wall_uv1;
	front_wall_deltaUV2 = front_wall_uv4 - front_wall_uv1;

	front_wall_f = 1.0f / (front_wall_deltaUV1.x * front_wall_deltaUV2.y - front_wall_deltaUV2.x * front_wall_deltaUV1.y);

	front_wall_tangent2.x = front_wall_f * (front_wall_deltaUV2.y * front_wall_edge1.x - front_wall_deltaUV1.y * front_wall_edge2.x);
	front_wall_tangent2.y = front_wall_f * (front_wall_deltaUV2.y * front_wall_edge1.y - front_wall_deltaUV1.y * front_wall_edge2.y);
	front_wall_tangent2.z = front_wall_f * (front_wall_deltaUV2.y * front_wall_edge1.z - front_wall_deltaUV1.y * front_wall_edge2.z);
	front_wall_tangent2 = glm::normalize(front_wall_tangent2);


	front_wall_bitangent2.x = front_wall_f * (-front_wall_deltaUV2.x * front_wall_edge1.x + front_wall_deltaUV1.x * front_wall_edge2.x);
	front_wall_bitangent2.y = front_wall_f * (-front_wall_deltaUV2.x * front_wall_edge1.y + front_wall_deltaUV1.x * front_wall_edge2.y);
	front_wall_bitangent2.z = front_wall_f * (-front_wall_deltaUV2.x * front_wall_edge1.z + front_wall_deltaUV1.x * front_wall_edge2.z);
	front_wall_bitangent2 = glm::normalize(front_wall_bitangent2);


	GLfloat front_wall_vertices[] = {
		// Positions            // normal         // TexCoords  // Tangent                          // Bitangent
		front_wall_pos1.x, front_wall_pos1.y, front_wall_pos1.z, front_wall_nm.x, front_wall_nm.y, front_wall_nm.z, front_wall_uv1.x, front_wall_uv1.y, front_wall_tangent1.x, front_wall_tangent1.y, front_wall_tangent1.z, front_wall_bitangent1.x, front_wall_bitangent1.y, front_wall_bitangent1.z,
		front_wall_pos2.x, front_wall_pos2.y, front_wall_pos2.z, front_wall_nm.x, front_wall_nm.y, front_wall_nm.z, front_wall_uv2.x, front_wall_uv2.y, front_wall_tangent1.x, front_wall_tangent1.y, front_wall_tangent1.z, front_wall_bitangent1.x, front_wall_bitangent1.y, front_wall_bitangent1.z,
		front_wall_pos3.x, front_wall_pos3.y, front_wall_pos3.z, front_wall_nm.x, front_wall_nm.y, front_wall_nm.z, front_wall_uv3.x, front_wall_uv3.y, front_wall_tangent1.x, front_wall_tangent1.y, front_wall_tangent1.z, front_wall_bitangent1.x, front_wall_bitangent1.y, front_wall_bitangent1.z,

		front_wall_pos1.x, front_wall_pos1.y, front_wall_pos1.z, front_wall_nm.x, front_wall_nm.y, front_wall_nm.z, front_wall_uv1.x, front_wall_uv1.y, front_wall_tangent2.x, front_wall_tangent2.y, front_wall_tangent2.z, front_wall_bitangent2.x, front_wall_bitangent2.y, front_wall_bitangent2.z,
		front_wall_pos3.x, front_wall_pos3.y, front_wall_pos3.z, front_wall_nm.x, front_wall_nm.y, front_wall_nm.z, front_wall_uv3.x, front_wall_uv3.y, front_wall_tangent2.x, front_wall_tangent2.y, front_wall_tangent2.z, front_wall_bitangent2.x, front_wall_bitangent2.y, front_wall_bitangent2.z,
		front_wall_pos4.x, front_wall_pos4.y, front_wall_pos4.z, front_wall_nm.x, front_wall_nm.y, front_wall_nm.z, front_wall_uv4.x, front_wall_uv4.y, front_wall_tangent2.x, front_wall_tangent2.y, front_wall_tangent2.z, front_wall_bitangent2.x, front_wall_bitangent2.y, front_wall_bitangent2.z
	};
	// Setup plane VAO
	GLuint VAO_WALL_FRONT, VBO_WALL_FRONT;
	glGenVertexArrays(1, &VAO_WALL_FRONT);
	glGenBuffers(1, &VBO_WALL_FRONT);
	glBindVertexArray(VAO_WALL_FRONT);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_WALL_FRONT);
	glBufferData(GL_ARRAY_BUFFER, sizeof(front_wall_vertices), &front_wall_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(11 * sizeof(GLfloat)));

	glBindVertexArray(0);

	log("");
	log("===Front Wall RenderObject===");
	RenderObject front_wall("Textures/wall_diffuse.bmp", "Textures/wall_normal.bmp", "Textures/wall_specular.bmp", front_wall_vertices, VAO_WALL_FRONT, VBO_WALL_FRONT);

	// Now for the left wall.
	// positions
	glm::vec3 pos1(0.0, 1.0, 0.0);
	glm::vec3 pos2(0.0, 0.0, 0.0);
	glm::vec3 pos3(0.0, 0.0, -1.0);
	glm::vec3 pos4(0.0, 1.0, -1.0);
	// texture coordinates
	glm::vec2 uv1(0.0, 1.0);
	glm::vec2 uv2(0.0, 0.0);
	glm::vec2 uv3(1.0, 0.0);
	glm::vec2 uv4(1.0, 1.0);
	// normal vector
	glm::vec3 nm(1.0, 0.0, 0.0);

	// calculate tangent/bitangent vectors of both triangles
	glm::vec3 tangent1, bitangent1;
	glm::vec3 tangent2, bitangent2;
	// - triangle 1
	glm::vec3 edge1 = pos2 - pos1;
	glm::vec3 edge2 = pos3 - pos1;
	glm::vec2 deltaUV1 = uv2 - uv1;
	glm::vec2 deltaUV2 = uv3 - uv1;

	GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent1 = glm::normalize(tangent1);

	bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	bitangent1 = glm::normalize(bitangent1);

	// - triangle 2
	edge1 = pos3 - pos1;
	edge2 = pos4 - pos1;
	deltaUV1 = uv3 - uv1;
	deltaUV2 = uv4 - uv1;

	f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent2 = glm::normalize(tangent2);


	bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	bitangent2 = glm::normalize(bitangent2);


	GLfloat wall_left_vertices[] = {
		// Positions            // normal         // TexCoords  // Tangent                          // Bitangent
		pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
		pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
		pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

		pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
		pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
		pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
	};
	// Setup plane VAO
	GLuint VAO_WALL_LEFT, VBO_WALL_LEFT;
	glGenVertexArrays(1, &VAO_WALL_LEFT);
	glGenBuffers(1, &VBO_WALL_LEFT);
	glBindVertexArray(VAO_WALL_LEFT);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_WALL_LEFT);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wall_left_vertices), &wall_left_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(11 * sizeof(GLfloat)));

	glBindVertexArray(0);

	log("");
	log("===Left Wall RenderObject===");
	RenderObject left_wall("Textures/wall_diffuse.bmp", "Textures/wall_normal.bmp", "Textures/wall_specular.bmp", wall_left_vertices, VAO_WALL_LEFT, VBO_WALL_LEFT);

	// For the right wall
	// positions
	glm::vec3 right_wall_pos1(0.0, 1.0, -1.0);
	glm::vec3 right_wall_pos2(0.0, 0.0, -1.0);
	glm::vec3 right_wall_pos3(0.0, 0.0, 0.0);
	glm::vec3 right_wall_pos4(0.0, 1.0, 0.0);
	// texture coordinates
	glm::vec2 right_wall_uv1(0.0, 1.0);
	glm::vec2 right_wall_uv2(0.0, 0.0);
	glm::vec2 right_wall_uv3(1.0, 0.0);
	glm::vec2 right_wall_uv4(1.0, 1.0);
	// normal vector
	glm::vec3 right_wall_nm(-1.0, 0.0, 0.0);

	// calculate tangent/bitangent vectors of both triangles
	glm::vec3 right_wall_tangent1, right_wall_bitangent1;
	glm::vec3 right_wall_tangent2, right_wall_bitangent2;
	// - triangle 1
	glm::vec3 right_wall_edge1 = right_wall_pos2 - right_wall_pos1;
	glm::vec3 right_wall_edge2 = right_wall_pos3 - right_wall_pos1;
	glm::vec2 right_wall_deltaUV1 = right_wall_uv2 - right_wall_uv1;
	glm::vec2 right_wall_deltaUV2 = right_wall_uv3 - right_wall_uv1;

	GLfloat right_wall_f = 1.0f / (right_wall_deltaUV1.x * right_wall_deltaUV2.y - right_wall_deltaUV2.x * right_wall_deltaUV1.y);

	right_wall_tangent1.x = right_wall_f * (right_wall_deltaUV2.y * right_wall_edge1.x - right_wall_deltaUV1.y * right_wall_edge2.x);
	right_wall_tangent1.y = right_wall_f * (right_wall_deltaUV2.y * right_wall_edge1.y - right_wall_deltaUV1.y * right_wall_edge2.y);
	right_wall_tangent1.z = right_wall_f * (right_wall_deltaUV2.y * right_wall_edge1.z - right_wall_deltaUV1.y * right_wall_edge2.z);
	right_wall_tangent1 = glm::normalize(right_wall_tangent1);

	right_wall_bitangent1.x = right_wall_f * (-right_wall_deltaUV2.x * right_wall_edge1.x + right_wall_deltaUV1.x * right_wall_edge2.x);
	right_wall_bitangent1.y = right_wall_f * (-right_wall_deltaUV2.x * right_wall_edge1.y + right_wall_deltaUV1.x * right_wall_edge2.y);
	right_wall_bitangent1.z = right_wall_f * (-right_wall_deltaUV2.x * right_wall_edge1.z + right_wall_deltaUV1.x * right_wall_edge2.z);
	right_wall_bitangent1 = glm::normalize(right_wall_bitangent1);

	// - triangle 2
	right_wall_edge1 = right_wall_pos3 - right_wall_pos1;
	right_wall_edge2 = right_wall_pos4 - right_wall_pos1;
	right_wall_deltaUV1 = right_wall_uv3 - right_wall_uv1;
	right_wall_deltaUV2 = right_wall_uv4 - right_wall_uv1;

	right_wall_f = 1.0f / (right_wall_deltaUV1.x * right_wall_deltaUV2.y - right_wall_deltaUV2.x * right_wall_deltaUV1.y);

	right_wall_tangent2.x = right_wall_f * (right_wall_deltaUV2.y * right_wall_edge1.x - right_wall_deltaUV1.y * right_wall_edge2.x);
	right_wall_tangent2.y = right_wall_f * (right_wall_deltaUV2.y * right_wall_edge1.y - right_wall_deltaUV1.y * right_wall_edge2.y);
	right_wall_tangent2.z = right_wall_f * (right_wall_deltaUV2.y * right_wall_edge1.z - right_wall_deltaUV1.y * right_wall_edge2.z);
	right_wall_tangent2 = glm::normalize(right_wall_tangent2);


	right_wall_bitangent2.x = right_wall_f * (-right_wall_deltaUV2.x * right_wall_edge1.x + right_wall_deltaUV1.x * right_wall_edge2.x);
	right_wall_bitangent2.y = right_wall_f * (-right_wall_deltaUV2.x * right_wall_edge1.y + right_wall_deltaUV1.x * right_wall_edge2.y);
	right_wall_bitangent2.z = right_wall_f * (-right_wall_deltaUV2.x * right_wall_edge1.z + right_wall_deltaUV1.x * right_wall_edge2.z);
	right_wall_bitangent2 = glm::normalize(right_wall_bitangent2);


	GLfloat right_wall_vertices[] = {
		// Positions            // normal         // TexCoords  // Tangent                          // Bitangent
		right_wall_pos1.x, right_wall_pos1.y, right_wall_pos1.z, right_wall_nm.x, right_wall_nm.y, right_wall_nm.z, right_wall_uv1.x, right_wall_uv1.y, right_wall_tangent1.x, right_wall_tangent1.y, right_wall_tangent1.z, right_wall_bitangent1.x, right_wall_bitangent1.y, right_wall_bitangent1.z,
		right_wall_pos2.x, right_wall_pos2.y, right_wall_pos2.z, right_wall_nm.x, right_wall_nm.y, right_wall_nm.z, right_wall_uv2.x, right_wall_uv2.y, right_wall_tangent1.x, right_wall_tangent1.y, right_wall_tangent1.z, right_wall_bitangent1.x, right_wall_bitangent1.y, right_wall_bitangent1.z,
		right_wall_pos3.x, right_wall_pos3.y, right_wall_pos3.z, right_wall_nm.x, right_wall_nm.y, right_wall_nm.z, right_wall_uv3.x, right_wall_uv3.y, right_wall_tangent1.x, right_wall_tangent1.y, right_wall_tangent1.z, right_wall_bitangent1.x, right_wall_bitangent1.y, right_wall_bitangent1.z,

		right_wall_pos1.x, right_wall_pos1.y, right_wall_pos1.z, right_wall_nm.x, right_wall_nm.y, right_wall_nm.z, right_wall_uv1.x, right_wall_uv1.y, right_wall_tangent2.x, right_wall_tangent2.y, right_wall_tangent2.z, right_wall_bitangent2.x, right_wall_bitangent2.y, right_wall_bitangent2.z,
		right_wall_pos3.x, right_wall_pos3.y, right_wall_pos3.z, right_wall_nm.x, right_wall_nm.y, right_wall_nm.z, right_wall_uv3.x, right_wall_uv3.y, right_wall_tangent2.x, right_wall_tangent2.y, right_wall_tangent2.z, right_wall_bitangent2.x, right_wall_bitangent2.y, right_wall_bitangent2.z,
		right_wall_pos4.x, right_wall_pos4.y, right_wall_pos4.z, right_wall_nm.x, right_wall_nm.y, right_wall_nm.z, right_wall_uv4.x, right_wall_uv4.y, right_wall_tangent2.x, right_wall_tangent2.y, right_wall_tangent2.z, right_wall_bitangent2.x, right_wall_bitangent2.y, right_wall_bitangent2.z
	};
	// Setup plane VAO
	GLuint VAO_WALL_RIGHT, VBO_WALL_RIGHT;
	glGenVertexArrays(1, &VAO_WALL_RIGHT);
	glGenBuffers(1, &VBO_WALL_RIGHT);
	glBindVertexArray(VAO_WALL_RIGHT);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_WALL_RIGHT);
	glBufferData(GL_ARRAY_BUFFER, sizeof(right_wall_vertices), &right_wall_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(11 * sizeof(GLfloat)));

	glBindVertexArray(0);

	log("");
	log("===Right Wall RenderObject===");
	RenderObject right_wall("Textures/wall_diffuse.bmp", "Textures/wall_normal.bmp", "Textures/wall_specular.bmp", right_wall_vertices, VAO_WALL_RIGHT, VBO_WALL_RIGHT);

	// Floor Buffer Setup.
	
	// World space positions of our floors
	glm::vec3 floorTranslations[] = {
		glm::vec3(-2.5f, -2.5f, 0.0f),
		glm::vec3(10.0f, -2.5f, 0.0f),
		glm::vec3(-2.5f, -2.5f, -12.5f),
		glm::vec3(10.0f, -2.5f, -12.5f),
	};

	// positions
	glm::vec3 floor_pos1(0.0, 0.0, -12.5);
	glm::vec3 floor_pos2(0.0, 0.0, 0.0);
	glm::vec3 floor_pos3(12.5, 0.0, 0.0);
	glm::vec3 floor_pos4(12.5, 0.0, -12.5);
	// texture coordinates
	glm::vec2 floor_uv1(0.0, 1.0);
	glm::vec2 floor_uv2(0.0, 0.0);
	glm::vec2 floor_uv3(1.0, 0.0);
	glm::vec2 floor_uv4(1.0, 1.0);
	// normal vector
	glm::vec3 floor_nm(0.0, 1.0, 0.0);

	// calculate tangent/bitangent vectors of both triangles
	glm::vec3 floor_tangent1, floor_bitangent1;
	glm::vec3 floor_tangent2, floor_bitangent2;
	// - triangle 1
	glm::vec3 floor_edge1 = floor_pos2 - floor_pos1;
	glm::vec3 floor_edge2 = floor_pos3 - floor_pos1;
	glm::vec2 floor_deltaUV1 = floor_uv2 - floor_uv1;
	glm::vec2 floor_deltaUV2 = floor_uv3 - floor_uv1;

	GLfloat floor_f = 1.0f / (floor_deltaUV1.x * floor_deltaUV2.y - floor_deltaUV2.x * floor_deltaUV1.y);

	floor_tangent1.x = floor_f * (floor_deltaUV2.y * floor_edge1.x - floor_deltaUV1.y * floor_edge2.x);
	floor_tangent1.y = floor_f * (floor_deltaUV2.y * floor_edge1.y - floor_deltaUV1.y * floor_edge2.y);
	floor_tangent1.z = floor_f * (floor_deltaUV2.y * floor_edge1.z - floor_deltaUV1.y * floor_edge2.z);
	floor_tangent1 = glm::normalize(floor_tangent1);

	floor_bitangent1.x = floor_f * (-floor_deltaUV2.x * floor_edge1.x + floor_deltaUV1.x * floor_edge2.x);
	floor_bitangent1.y = floor_f * (-floor_deltaUV2.x * floor_edge1.y + floor_deltaUV1.x * floor_edge2.y);
	floor_bitangent1.z = floor_f * (-floor_deltaUV2.x * floor_edge1.z + floor_deltaUV1.x * floor_edge2.z);
	floor_bitangent1 = glm::normalize(floor_bitangent1);

	// - triangle 2
	floor_edge1 = floor_pos3 - floor_pos1;
	floor_edge2 = floor_pos4 - floor_pos1;
	floor_deltaUV1 = floor_uv3 - floor_uv1;
	floor_deltaUV2 = floor_uv4 - floor_uv1;

	floor_f = 1.0f / (floor_deltaUV1.x * floor_deltaUV2.y - floor_deltaUV2.x * floor_deltaUV1.y);

	floor_tangent2.x = floor_f * (floor_deltaUV2.y * floor_edge1.x - floor_deltaUV1.y * floor_edge2.x);
	floor_tangent2.y = floor_f * (floor_deltaUV2.y * floor_edge1.y - floor_deltaUV1.y * floor_edge2.y);
	floor_tangent2.z = floor_f * (floor_deltaUV2.y * floor_edge1.z - floor_deltaUV1.y * floor_edge2.z);
	floor_tangent2 = glm::normalize(floor_tangent2);


	floor_bitangent2.x = floor_f * (-floor_deltaUV2.x * floor_edge1.x + floor_deltaUV1.x * floor_edge2.x);
	floor_bitangent2.y = floor_f * (-floor_deltaUV2.x * floor_edge1.y + floor_deltaUV1.x * floor_edge2.y);
	floor_bitangent2.z = floor_f * (-floor_deltaUV2.x * floor_edge1.z + floor_deltaUV1.x * floor_edge2.z);
	floor_bitangent2 = glm::normalize(floor_bitangent2);


	GLfloat floor_vertices[] = {
		// Positions            // normal         // TexCoords  // Tangent                          // Bitangent
		floor_pos1.x, floor_pos1.y, floor_pos1.z, floor_nm.x, floor_nm.y, floor_nm.z, floor_uv1.x, floor_uv1.y, floor_tangent1.x, floor_tangent1.y, floor_tangent1.z, floor_bitangent1.x, floor_bitangent1.y, floor_bitangent1.z,
		floor_pos2.x, floor_pos2.y, floor_pos2.z, floor_nm.x, floor_nm.y, floor_nm.z, floor_uv2.x, floor_uv2.y, floor_tangent1.x, floor_tangent1.y, floor_tangent1.z, floor_bitangent1.x, floor_bitangent1.y, floor_bitangent1.z,
		floor_pos3.x, floor_pos3.y, floor_pos3.z, floor_nm.x, floor_nm.y, floor_nm.z, floor_uv3.x, floor_uv3.y, floor_tangent1.x, floor_tangent1.y, floor_tangent1.z, floor_bitangent1.x, floor_bitangent1.y, floor_bitangent1.z,

		floor_pos1.x, floor_pos1.y, floor_pos1.z, floor_nm.x, floor_nm.y, floor_nm.z, floor_uv1.x, floor_uv1.y, floor_tangent2.x, floor_tangent2.y, floor_tangent2.z, floor_bitangent2.x, floor_bitangent2.y, floor_bitangent2.z,
		floor_pos3.x, floor_pos3.y, floor_pos3.z, floor_nm.x, floor_nm.y, floor_nm.z, floor_uv3.x, floor_uv3.y, floor_tangent2.x, floor_tangent2.y, floor_tangent2.z, floor_bitangent2.x, floor_bitangent2.y, floor_bitangent2.z,
		floor_pos4.x, floor_pos4.y, floor_pos4.z, floor_nm.x, floor_nm.y, floor_nm.z, floor_uv4.x, floor_uv4.y, floor_tangent2.x, floor_tangent2.y, floor_tangent2.z, floor_bitangent2.x, floor_bitangent2.y, floor_bitangent2.z
	};
	// Setup plane VAO
	GLuint VAO_FLOOR, VBO_FLOOR;
	glGenVertexArrays(1, &VAO_FLOOR);
	glGenBuffers(1, &VBO_FLOOR);
	glBindVertexArray(VAO_FLOOR);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_FLOOR);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_vertices), &floor_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(11 * sizeof(GLfloat)));

	glBindVertexArray(0);

	log("");
	log("===Floor RenderObject===");
	RenderObject floor("Textures/floor_diffuse.bmp", "Textures/floor_normal.bmp", "Textures/floor_specular.bmp", floor_vertices, VAO_FLOOR, VBO_FLOOR);

	// Point Lights.
	log("");
	log("===Point Light Shader===");
	Shader pointLightShader("Shaders/PointLight.vert", "Shaders/PointLight.frag");
	// Positions of the point lights
	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.5f, 1.0f, -12.5f),
		glm::vec3(24.5f, 1.0f, -12.5f)
	};

	// Setup skybox.
	log("");
	log("===Skybox Shader===");
	Shader skyboxShader("Shaders/skybox.vert", "Shaders/skybox.frag");

	Skybox skybox;

	skybox.SetupSkybox();

	// Cubemap (Skybox)
	vector<const GLchar*> faces;
	faces.push_back("Textures/skybox_left.jpg");
	faces.push_back("Textures/skybox_right.jpg");
	faces.push_back("Textures/skybox_top.jpg");
	faces.push_back("Textures/skybox_bottom.jpg");
	faces.push_back("Textures/skybox_back.jpg");
	faces.push_back("Textures/skybox_front.jpg");
	skybox.LoadCubemap(faces);

	// For post-processing
	GLfloat quadVertices[] = {   // Vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// Positions   // TexCoords
		-1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,

		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};

	GLuint quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glBindVertexArray(0);

	// Framebuffers
	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// Create a multisampled color attachment texture
	GLuint textureColorBufferMultiSampled = generateMultiSampleTexture(4);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
	// Create a renderbuffer object for depth and stencil attachments
	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, 800, 600);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// second framebuffer
	GLuint intermediateFBO;
	GLuint screenTexture = generateAttachmentTexture(false, false);
	glGenFramebuffers(1, &intermediateFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);	// We only need a color buffer

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	log("");
	log("===Post Processing Shader===");
	Shader screenShader("Shaders/post_processing.vert", "Shaders/post_processing.frag");

	log("");
	log("===Liberty Statue Model===");
	Model pedestal("Models/LibertyStatue/LibertStatue.obj");
	
	log("");
	log("===Nanosuit Model===");
	Model nanosuit("Models/Nanosuit/nanosuit.obj");
	
	log("");
	log("===Model Loading Shader===");
	Shader model_loading("Shaders/crysis.vert", "Shaders/crysis.frag");

	log("");
	log("===Particle Shader===");
	Shader particleShader("Shaders/particle.vert", "Shaders/particle.frag");

	log("");
	log("===Shadow Mapping Shader===");
	Shader simpleDepthShader("Shaders/shadow_mapping.vert", "Shaders/shadow_mapping.frag");

	log("");
	log("===Environment Shader===");
	Shader environmentShader("Shaders/environment.vert", "Shaders/environment.frag");

	GLfloat cubeVertices[] = {
		// Positions        // Normals  
		-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f
	};

	// Setup environment mappped cube
	GLuint cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);

	float interval = 0.0f;
	int noOfFrames = 0;

	// For shadow-mapping of directional light
	glEnable(GL_DEPTH_TEST);
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	// Generating the shadow map
	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Attaching the depth map to the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

#ifdef DEBUG
	log("Buffers initialized.");
#endif

	// Render to generate the depth map
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Configure the shader and matrices
	GLfloat near_plane = 1.0f, far_plane = 70.0f;
	glm::mat4 lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);

	glm::mat4 lightView = glm::lookAt(glm::vec3(-10.0f, 40.0f, -25.0f), glm::vec3(25.0f, 1.0f, 2.5f), glm::vec3(1.0));

	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	glm::mat4 direcLightSpaceMatrix = lightSpaceMatrix;

	simpleDepthShader.Use();

	GLint lightSpaceMatrixLocation = glGetUniformLocation(simpleDepthShader.program, "lightSpaceMatrix");
	glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

	// Render scene onto the depth-map
	{
		// Rendering the wall.
		glBindVertexArray(front_wall.VAO);
		for (GLuint i = 0; i < 5; i++)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model;
			model = glm::translate(model, wallTranslations[i]);
			model = glm::scale(model, glm::vec3(5.0f));
			glUniformMatrix4fv(glGetUniformLocation(simpleDepthShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			front_wall.Render(simpleDepthShader, false);
		}
		glBindVertexArray(0);

		glBindVertexArray(back_wall.VAO);
		for (GLuint i = 5; i < 10; i++)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model;
			model = glm::translate(model, wallTranslations[i]);
			model = glm::scale(model, glm::vec3(5.0f));
			glUniformMatrix4fv(glGetUniformLocation(simpleDepthShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			back_wall.Render(simpleDepthShader, false);
		}
		glBindVertexArray(0);

		glBindVertexArray(left_wall.VAO);
		for (GLuint i = 10; i < 15; i++)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model;
			model = glm::translate(model, wallTranslations[i]);
			model = glm::scale(model, glm::vec3(5.0f));
			glUniformMatrix4fv(glGetUniformLocation(simpleDepthShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			left_wall.Render(simpleDepthShader, false);
		}
		glBindVertexArray(0);

		glBindVertexArray(right_wall.VAO);
		for (GLuint i = 15; i < 20; i++)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model;
			model = glm::translate(model, wallTranslations[i]);
			model = glm::scale(model, glm::vec3(5.0f));
			glUniformMatrix4fv(glGetUniformLocation(simpleDepthShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			right_wall.Render(simpleDepthShader, false);
		}
		glBindVertexArray(0);

		glBindVertexArray(VAO_FLOOR);
		for (GLuint i = 0; i < 4; i++)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model;
			model = glm::translate(model, floorTranslations[i]);
			glUniformMatrix4fv(glGetUniformLocation(simpleDepthShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			floor.Render(simpleDepthShader, false);
		}
		glBindVertexArray(0);

#ifdef RENDER_ENVIRONMENT_CUBE
		glm::mat4 model_cube;
		model_cube = glm::translate(model_cube, glm::vec3(10.0f, -2.0f, -10.0f)); // Translate it down a bit so it's at the center of the scene
		model_cube = glm::scale(model_cube, glm::vec3(1.0f));
		glUniformMatrix4fv(glGetUniformLocation(simpleDepthShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model_cube));

		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
#endif

#ifdef RENDER_MODELS

		glm::mat4 model_2;
		model_2 = glm::translate(model_2, glm::vec3(20.0f, -2.5f, -2.0f)); // Translate it down a bit so it's at the center of the scene
		model_2 = glm::rotate(model_2, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model_2 = glm::scale(model_2, glm::vec3(3.0f));
		glUniformMatrix4fv(glGetUniformLocation(simpleDepthShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model_2));
		pedestal.Draw(simpleDepthShader);

		// Now draw the nanosuit
		glm::mat4 model_3;
		model_3 = glm::translate(model_3, glm::vec3(10.0f, -2.5f, -12.5f)); // Translate it down a bit so it's at the center of the scene
		model_3 = glm::scale(model_3, glm::vec3(0.2f));
		glUniformMatrix4fv(glGetUniformLocation(simpleDepthShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model_3));
		nanosuit.Draw(simpleDepthShader);
#endif
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// For shadow-mapping of directional light
	glEnable(GL_DEPTH_TEST);
	GLuint pointDepthMapFBO;
	glGenFramebuffers(1, &pointDepthMapFBO);

	// Generating the shadow map
	GLuint pointDepthMap;
	glGenTextures(1, &pointDepthMap);
	glBindTexture(GL_TEXTURE_2D, pointDepthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Attaching the depth map to the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, pointDepthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, pointDepthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Generate depth map for the point light

	log("");
	log("===Point Depth Shader===");
	Shader pointDepthShader("Shaders/omniDepth.vert", "Shaders/omniDepth.frag");

	// Render to generate the depth map
	glBindFramebuffer(GL_FRAMEBUFFER, pointDepthMapFBO);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Configure the shader and matrices
	near_plane = 1.0f;
	far_plane = 70.0f;
	lightProjection = glm::ortho(-6.0f, 10.0f, -30.0f, 30.0f, near_plane, far_plane);

	lightView = glm::lookAt(glm::vec3(0.0f, 1.0f, -15.0f), glm::vec3(10.0f, 0.5f, -12.5f), glm::vec3(1.0));

	lightSpaceMatrix = lightProjection * lightView;

	glm::mat4 pointLightSpaceMatrix = lightSpaceMatrix;

	pointDepthShader.Use();

	lightSpaceMatrixLocation = glGetUniformLocation(pointDepthShader.program, "lightSpaceMatrix");
	glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

	// Render scene onto the depth-map
	{
		// Rendering the wall.
		glBindVertexArray(front_wall.VAO);
		for (GLuint i = 0; i < 5; i++)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model;
			model = glm::translate(model, wallTranslations[i]);
			model = glm::scale(model, glm::vec3(5.0f));
			glUniformMatrix4fv(glGetUniformLocation(pointDepthShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			front_wall.Render(pointDepthShader, false);
		}
		glBindVertexArray(0);

		glBindVertexArray(back_wall.VAO);
		for (GLuint i = 5; i < 10; i++)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model;
			model = glm::translate(model, wallTranslations[i]);
			model = glm::scale(model, glm::vec3(5.0f));
			glUniformMatrix4fv(glGetUniformLocation(pointDepthShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			back_wall.Render(pointDepthShader, false);
		}
		glBindVertexArray(0);

		glBindVertexArray(left_wall.VAO);
		for (GLuint i = 10; i < 15; i++)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model;
			model = glm::translate(model, wallTranslations[i]);
			model = glm::scale(model, glm::vec3(5.0f));
			glUniformMatrix4fv(glGetUniformLocation(pointDepthShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			left_wall.Render(pointDepthShader, false);
		}
		glBindVertexArray(0);

		glBindVertexArray(right_wall.VAO);
		for (GLuint i = 15; i < 20; i++)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model;
			model = glm::translate(model, wallTranslations[i]);
			model = glm::scale(model, glm::vec3(5.0f));
			glUniformMatrix4fv(glGetUniformLocation(pointDepthShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			right_wall.Render(pointDepthShader, false);
		}
		glBindVertexArray(0);

		glBindVertexArray(floor.VAO);
		for (GLuint i = 0; i < 4; i++)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model;
			model = glm::translate(model, floorTranslations[i]);
			glUniformMatrix4fv(glGetUniformLocation(pointDepthShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			floor.Render(pointDepthShader, false);
		}
		glBindVertexArray(0);

#ifdef RENDER_ENVIRONMENT_CUBE
		glm::mat4 model_cube;
		model_cube = glm::translate(model_cube, glm::vec3(10.0f, -2.0f, -10.0f)); // Translate it down a bit so it's at the center of the scene
		model_cube = glm::scale(model_cube, glm::vec3(1.0f));
		glUniformMatrix4fv(glGetUniformLocation(pointDepthShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model_cube));

		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
#endif

#ifdef RENDER_MODELS
		glm::mat4 model_2;
		model_2 = glm::translate(model_2, glm::vec3(20.0f, -2.5f, -2.0f)); // Translate it down a bit so it's at the center of the scene
		model_2 = glm::rotate(model_2, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model_2 = glm::scale(model_2, glm::vec3(3.0f));
		glUniformMatrix4fv(glGetUniformLocation(pointDepthShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model_2));
		pedestal.Draw(pointDepthShader);

		// Now draw the nanosuit
		glm::mat4 model_3;
		model_3 = glm::translate(model_3, glm::vec3(10.0f, -2.5f, -12.5f)); // Translate it down a bit so it's at the center of the scene
		model_3 = glm::scale(model_3, glm::vec3(0.2f));
		glUniformMatrix4fv(glGetUniformLocation(pointDepthShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model_3));
		nanosuit.Draw(pointDepthShader);
#endif
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

#ifdef DEBUG
	log("Depth Maps Generated.");
#endif

	// Main application loop
	while (!glfwWindowShouldClose(appWindow))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		currTime = getTimeElapsed();
		deltaTime = currTime - prevTime;

		// Update
		Update(deltaTime);

		// 1. Draw scene as normal in multisampled buffers
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		// Clear the colorbuffer
		glViewport(0, 0, appWidth, appHeight);
		glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Draw skybox first
		skyboxShader.Use();
		glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
		glm::mat4 projection = glm::perspective(camera.Zoom, (float)appWidth / (float)appHeight, 0.1f, 100.0f);
		
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		skybox.Render(skyboxShader);
		
		// Activate shader
		ourShader.Use();

		glUniform1i(glGetUniformLocation(ourShader.program, "pointLightOn"), pointLightOn);

		glUniform1i(glGetUniformLocation(ourShader.program, "diffuseMap"), 0);
		glUniform1i(glGetUniformLocation(ourShader.program, "specularMap"), 1);
		glUniform1i(glGetUniformLocation(ourShader.program, "normalMap"), 2);

		glActiveTexture(GL_TEXTURE3); // We already have 3 texture units active (in this shader) so set the skybox as the 4th texture unit (texture units are 0 based so index number 3)
		glUniform1i(glGetUniformLocation(ourShader.program, "shadowMap"), 3);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		glActiveTexture(GL_TEXTURE4); // We already have 3 texture units active (in this shader) so set the skybox as the 4th texture unit (texture units are 0 based so index number 3)
		glUniform1i(glGetUniformLocation(ourShader.program, "pointShadowMap"), 4);
		glBindTexture(GL_TEXTURE_2D, pointDepthMap);

		lightSpaceMatrixLocation = glGetUniformLocation(ourShader.program, "direcLightSpaceMatrix");
		glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(direcLightSpaceMatrix));

		lightSpaceMatrixLocation = glGetUniformLocation(ourShader.program, "pointLightSpaceMatrix");
		glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(pointLightSpaceMatrix));

		GLint viewPosLoc = glGetUniformLocation(ourShader.program, "viewPos");
		glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);

		GLint lightPosLoc = glGetUniformLocation(ourShader.program, "lightPos");
		glUniform3f(lightPosLoc, -2.4f, 1.0f, -15.0f);

		GLint cameraDirLoc = glGetUniformLocation(ourShader.program, "cameraDir");
		glUniform3f(cameraDirLoc, camera.Front.x, camera.Front.y, camera.Front.z);

		GLint flashLightLoc = glGetUniformLocation(ourShader.program, "flashLight");
		glUniform1i(flashLightLoc, flashLight);

		// Create camera transformation
		view = camera.GetViewMatrix();
		projection = glm::perspective(camera.Zoom, (float)appWidth / (float)appHeight, 0.1f, 1000.0f);
		// Get the uniform locations
		GLint modelLoc = glGetUniformLocation(ourShader.program, "model");
		GLint viewLoc = glGetUniformLocation(ourShader.program, "view");
		GLint projLoc = glGetUniformLocation(ourShader.program, "projection");
		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// Rendering the wall.
		glBindVertexArray(front_wall.VAO);
		for (GLuint i = 0; i < 5; i++)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model;
			model = glm::translate(model, wallTranslations[i]);
			model = glm::scale(model, glm::vec3(5.0f));
			glUniformMatrix4fv(glGetUniformLocation(ourShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			front_wall.Render(ourShader);
		}
		glBindVertexArray(0);

		glBindVertexArray(back_wall.VAO);
		for (GLuint i = 5; i < 10; i++)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model;
			model = glm::translate(model, wallTranslations[i]);
			model = glm::scale(model, glm::vec3(5.0f));
			glUniformMatrix4fv(glGetUniformLocation(ourShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			back_wall.Render(ourShader);
		}
		glBindVertexArray(0);

		glBindVertexArray(left_wall.VAO);
		for (GLuint i = 10; i < 15; i++)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model;
			model = glm::translate(model, wallTranslations[i]);
			model = glm::scale(model, glm::vec3(5.0f));
			glUniformMatrix4fv(glGetUniformLocation(ourShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			left_wall.Render(ourShader);
		}
		glBindVertexArray(0);

		glBindVertexArray(right_wall.VAO);
		for (GLuint i = 15; i < 20; i++)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model;
			model = glm::translate(model, wallTranslations[i]);
			model = glm::scale(model, glm::vec3(5.0f));
			glUniformMatrix4fv(glGetUniformLocation(ourShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			right_wall.Render(ourShader);
		}
		glBindVertexArray(0);

		// Rendering the floor.
		
		glBindVertexArray(floor.VAO);
		for (GLuint i = 0; i < 4; i++)
		{
			// Calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model;
			model = glm::translate(model, floorTranslations[i]);
			glUniformMatrix4fv(glGetUniformLocation(ourShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			floor.Render(ourShader);
		}
		glBindVertexArray(0);

		// Render the light cube
		pointLightShader.Use();

		glUniform1i(glGetUniformLocation(pointLightShader.program, "pointLightOn"), pointLightOn);

		// Get the uniform locations
		modelLoc = glGetUniformLocation(pointLightShader.program, "model");
		viewLoc = glGetUniformLocation(pointLightShader.program, "view");
		projLoc = glGetUniformLocation(pointLightShader.program, "projection");
		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(skybox.skyboxVAO);
		glm::mat4 model;
		model = glm::translate(model, glm::vec3(-2.4f, 1.0f, -15.0f));
		model = glm::scale(model, glm::vec3(0.05, 0.1, 0.2));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

#ifdef RENDER_MODELS
		// Render the models
		model_loading.Use();   // <-- Don't forget this one!

		glUniform1i(glGetUniformLocation(model_loading.program, "pointLightOn"), pointLightOn);

		glActiveTexture(GL_TEXTURE4); // We already have 3 texture units active (in this shader) so set the skybox as the 4th texture unit (texture units are 0 based so index number 3)
		glUniform1i(glGetUniformLocation(model_loading.program, "shadowMap"), 4);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		glActiveTexture(GL_TEXTURE5); // We already have 3 texture units active (in this shader) so set the skybox as the 4th texture unit (texture units are 0 based so index number 3)
		glUniform1i(glGetUniformLocation(model_loading.program, "pointShadowMap"), 5);
		glBindTexture(GL_TEXTURE_2D, pointDepthMap);

		lightSpaceMatrixLocation = glGetUniformLocation(model_loading.program, "direcLightSpaceMatrix");
		glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(direcLightSpaceMatrix));

		lightSpaceMatrixLocation = glGetUniformLocation(model_loading.program, "pointLightSpaceMatrix");
		glUniformMatrix4fv(lightSpaceMatrixLocation, 1, GL_FALSE, glm::value_ptr(pointLightSpaceMatrix));

		viewPosLoc = glGetUniformLocation(model_loading.program, "viewPos");
		glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);

		flashLightLoc = glGetUniformLocation(model_loading.program, "flashLight");
		glUniform1i(flashLightLoc, flashLight);

		cameraDirLoc = glGetUniformLocation(ourShader.program, "cameraDir");
		glUniform3f(cameraDirLoc, camera.Front.x, camera.Front.y, camera.Front.z);

		// Transformation matrices
		projection = glm::perspective(camera.Zoom, (float)appWidth / (float)appHeight, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(model_loading.program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(model_loading.program, "view"), 1, GL_FALSE, glm::value_ptr(view));

		// Draw the Statue of Liberty
		glUniform1i(glGetUniformLocation(model_loading.program, "reflectionMap"), 0);

		glm::mat4 model_2;
		model_2 = glm::translate(model_2, glm::vec3(20.0f, -2.5f, -2.0f)); // Translate it down a bit so it's at the center of the scene
		model_2 = glm::rotate(model_2, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model_2 = glm::scale(model_2, glm::vec3(3.0f));
		glUniformMatrix4fv(glGetUniformLocation(model_loading.program, "model"), 1, GL_FALSE, glm::value_ptr(model_2));
		pedestal.Draw(model_loading);

		// Draw the Nanosuit
		glUniform1i(glGetUniformLocation(model_loading.program, "reflectionMap"), 1);
		
		glActiveTexture(GL_TEXTURE3); // We already have 3 texture units active (in this shader) so set the skybox as the 4th texture unit (texture units are 0 based so index number 3)
		glUniform1i(glGetUniformLocation(model_loading.program, "skybox"), 3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.cubemapTexture);

		// Now draw the nanosuit
		glm::mat4 model_3;
		model_3 = glm::translate(model_3, glm::vec3(10.0f, -2.5f, -12.5f)); // Translate it down a bit so it's at the center of the scene
		model_3 = glm::scale(model_3, glm::vec3(0.2f));
		glUniformMatrix4fv(glGetUniformLocation(model_loading.program, "model"), 1, GL_FALSE, glm::value_ptr(model_3));
		nanosuit.Draw(model_loading);
#endif

#ifdef RENDER_ENVIRONMENT_CUBE
		environmentShader.Use();

		// Set uniforms

		projection = glm::perspective(camera.Zoom, (float)appWidth / (float)appHeight, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(environmentShader.program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(environmentShader.program, "view"), 1, GL_FALSE, glm::value_ptr(view));

		glm::mat4 model_cube;
		model_cube = glm::translate(model_cube, glm::vec3(10.0f, -2.0f, -10.0f)); // Translate it down a bit so it's at the center of the scene
		model_cube = glm::scale(model_cube, glm::vec3(1.0f));
		glUniformMatrix4fv(glGetUniformLocation(environmentShader.program, "model"), 1, GL_FALSE, glm::value_ptr(model_cube));

		glUniform3f(glGetUniformLocation(environmentShader.program, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(environmentShader.program, "skybox"), 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.cubemapTexture);

		// Perform the render call

		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
#endif

#ifdef RENDER_PARTICLES
		// Render the particle system
		particleShader.Use();

		projection = glm::perspective(camera.Zoom, (float)appWidth / (float)appHeight, 0.1f, 100.0f);
		view = camera.GetViewMatrix();

		glUniformMatrix4fv(glGetUniformLocation(particleShader.program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(particleShader.program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		
		rain.Render(particleShader, camera.Front, glm::vec3(0.02f, 0.1f, 0.02f));
		rain.Update();

#endif

		// 2. Now blit multisampled buffer(s) to normal colorbuffer of intermediate FBO. Image is stored in screenTexture
		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
		glBlitFramebuffer(0, 0, 800, 600, 0, 0, 800, 600, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		// 3. Now render quad with scene's visuals as its texture image
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		// Draw Screen quad
		screenShader.Use();

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(screenShader.program, "screenTexture"), 0);
		glBindTexture(GL_TEXTURE_2D, screenTexture);

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		// Calculate FPS for debugging.
		interval += deltaTime;
		++noOfFrames;

		if (interval >= 1.0f)
		{
			CalculateFPS(noOfFrames, interval);
			interval = 0.0f;
			noOfFrames = 0;
		}

		// Swap the screen buffers
		glfwSwapBuffers(appWindow);
	}

	Shutdown();
	return 0;
}

void Application::Update(float deltaTime)
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (keys[GLFW_KEY_F])
		flashLight = 1;
	if (keys[GLFW_KEY_G])
		flashLight = 0;
	if (keys[GLFW_KEY_O])
		pointLightOn = 1;
	if (keys[GLFW_KEY_P])
		pointLightOn = 0;
}

void Application::Render()
{	
}

void Application::CalculateFPS(int noOfFrames, float interval)
{
	fps = 60;

	std::stringstream ss;
	ss << "LightEngine Demo  ||  FPS : " << fps;

	glfwSetWindowTitle(appWindow, ss.str().c_str());
}

void Application::Shutdown()
{
	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
#ifdef DEBUG
	log("Engine shutdown complete.");
#endif
}

Application::~Application()
{
}
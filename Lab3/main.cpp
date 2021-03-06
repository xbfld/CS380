// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/affine.hpp>
#include <common/geometry.hpp>
#include <common/arcball.hpp>

float g_groundSize = 100.0f;
float g_groundY = -2.5f;

GLuint lightLocGround, lightLocObject;

// View properties
glm::mat4 Projection;
float windowWidth = 1024.0f;
float windowHeight = 768.0f;
int frameBufferWidth = 0;
int frameBufferHeight = 0;
float fov = 45.0f;
float fovy = fov;

// Model properties
Model ground, object;
glm::mat4 skyRBT;
glm::mat4 eyeRBT;
const glm::mat4 worldRBT = glm::mat4(1.0f);
glm::mat4 objectOffsetFrame = glm::scale(11.0f, 11.0f, 11.0f) * glm::rotate(glm::mat4(1.0f), 10.0f, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(0.03f, -0.08f, 0.0f));
glm::mat4 objectCenterRBT = glm::mat4(1.0f);
struct SUB_OBJECT_INFO
{
	glm::mat4 RBT;
	Material material;
	SHADER_TYPE type;
};
std::vector <SUB_OBJECT_INFO> subObjects = std::vector<SUB_OBJECT_INFO>();
glm::mat4 arcballRBT = glm::mat4(1.0f);
glm::mat4 aFrame;

// Mouse interaction
bool MOUSE_LEFT_PRESS = false; bool MOUSE_MIDDLE_PRESS = false; bool MOUSE_RIGHT_PRESS = false;

// Transformation
glm::mat4 m = glm::mat4(1.0f);

// Manipulation index
int object_index = 1; int view_index = 0; int sky_type = 0;

// Arcball manipulation
Model arcBall;
float arcBallScreenRadius = 0.25f * min(windowWidth, windowHeight);
float arcBallScale = 0.01f; float ScreenToEyeScale = 0.01f;
float prev_x = 0.0f; float prev_y = 0.0f;

double cTime;
double dTime;
double lTime;

struct DirectionalLight
{
	vec3 direction;

	// contains color and intensity
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
struct PointLight
{
	vec3 position;

	// // coefficient for falloff function
	// float c0;	// constant coefficient
	// float c1;	// linear coefficient
	// float c2;	// quadratic coefficient
	// float c3;	// cubic coefficient
	vec3 coefficient;

	// contains color and intensity
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
struct SpotLight
{
	vec3 position;
	vec3 direction;

	// cosine of corn angle
	float radius_inner;
	float radius_outer;

	// contains color and intensity
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

const vec3 RED(1.0f, 0.0f, 0.0f);
const vec3 GREEN(0.0f, 1.0f, 0.0f);
const vec3 BLUE(0.0f, 0.0f, 1.0f);
const vec3 CYAN(0.0f, 1.0f, 1.0f);
const vec3 MAGENTA(1.0f, 0.0f, 1.0f);
const vec3 YELLOW(1.0f, 1.0f, 0.0f);
const vec3 BLACK(0.0f, 0.0f, 0.0f);
const vec3 WHITE(1.0f, 1.0f, 1.0f);
const vec3 COLORS[8]{ WHITE, RED, GREEN, BLUE, CYAN, MAGENTA, YELLOW };

const Material MATERIAL_PLASTIC_GREEN{ glm::vec3(0.0f), glm::vec3(0.1f,0.35f,0.1f), glm::vec3(0.45f, 0.55f, 0.45f), 32.0 };
const Material MATERIAL_BRONZE{ glm::vec3(0.2125f,0.1275f,0.054f), glm::vec3(0.714f,0.4284f,0.18144f), glm::vec3(0.393548f, 0.271906f, 0.166721f), 25.6 };
const Material MATERIAL_COPPER{ glm::vec3(0.19125,0.0735,0.0225), glm::vec3(0.7038,0.27048,0.0828), glm::vec3(0.256777, 0.137622, 0.086014), 12.8 };
const Material MATERIAL_RUBBER_BLACK{ glm::vec3(0.02), glm::vec3(0.01), glm::vec3(0.4), 10 };
const Material MATERIAL_JADE{ glm::vec3(0.135, 0.2225, 0.1575), glm::vec3(0.54,0.89,0.63), glm::vec3(0.316228,0.316228,0.316228), 12.8 };
// Reflection property for common materials (http://www.real3dtutorials.com/tut00008.php)

DirectionalLight dLight{ vec3(0.0f,-1.0f,0.0f), WHITE*0.005f,WHITE, WHITE };
PointLight pLight{ vec3(2.0f), vec3(1.0f, 0.0f, 0.02f), WHITE*0.005f,WHITE, WHITE };
SpotLight sLight{ vec3(0.0f,3.0f,0.0f), vec3(0.0f,-1.0f,0.0f), 0.8f, 0.7f, WHITE*0.005f,WHITE, WHITE };

int d_color_index(0), p_color_index(0), s_color_index(0);
static void cycle_color(int &c) { c = (7 <= c) ? 0 : c + 1; }

static bool non_ego_cube_manipulation()
{
	return object_index != 0 && view_index != object_index;
}

static bool use_arcball()
{
	return (object_index == 0 && sky_type == 0) || non_ego_cube_manipulation();
}

static void window_size_callback(GLFWwindow* window, int width, int height)
{
	// Get resized size and set to current window
	windowWidth = (float)width;
	windowHeight = (float)height;

	// glViewport accept pixel size, please use glfwGetFramebufferSize rather than window size.
	// window size != framebuffer size
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
	glViewport(0, 0, frameBufferWidth, frameBufferHeight);

	arcBallScreenRadius = 0.25f * min(frameBufferWidth, frameBufferHeight);

	if (frameBufferWidth >= frameBufferHeight)
	{
		fovy = fov;
	}
	else {
		const float RAD_PER_DEG = 0.5f * glm::pi<float>() / 180.0f;
		fovy = atan2(sin(fov * RAD_PER_DEG) * (float)frameBufferHeight / (float)frameBufferWidth, cos(fov * RAD_PER_DEG)) / RAD_PER_DEG;
	}

	// Update projection matrix
	Projection = glm::perspective(fov, windowWidth / windowHeight, 0.1f, 100.0f);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	MOUSE_LEFT_PRESS |= (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS);
	MOUSE_RIGHT_PRESS |= (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS);
	MOUSE_MIDDLE_PRESS |= (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS);

	MOUSE_LEFT_PRESS &= !(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE);
	MOUSE_RIGHT_PRESS &= !(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE);
	MOUSE_MIDDLE_PRESS &= !(button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE);

	if (action == GLFW_RELEASE) {
		prev_x = 0.0f; prev_y = 0.0f;
	}
}

void setWrtFrame()
{
	switch (object_index)
	{
	case 0:
		// world-sky: transFact(worldRBT) * linearFact(skyRBT), sky-sky: transFact(skyRBT) * linearFact(skyRBT)
		aFrame = (sky_type == 0) ? linearFact(skyRBT) : skyRBT;
		break;
	case 1:
		aFrame = transFact(objectCenterRBT) * linearFact(eyeRBT);
		break;
	}
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (view_index != 0 && object_index == 0) return;
	// Convert mouse pointer into screen space. (http://gamedev.stackexchange.com/questions/83570/why-is-the-origin-in-computer-graphics-coordinates-at-the-top-left)
	xpos = xpos * ((float)frameBufferWidth / windowWidth);
	ypos = (float)frameBufferHeight - ypos * ((float)frameBufferHeight / windowHeight) - 1.0f;

	double dx_t = xpos - prev_x;
	double dy_t = ypos - prev_y;
	double dx_r = xpos - prev_x;
	double dy_r = ypos - prev_y;

	if (view_index == 0 && object_index == 0)
	{
		if (sky_type == 0) { dx_t = -dx_t; dy_t = -dy_t; dx_r = -dx_r; dy_r = -dy_r; }
		else { dx_r = -dx_r; dy_r = -dy_r; }
	}

	if (MOUSE_LEFT_PRESS)
	{
		if (prev_x - 1e-16 < 1e-8 && prev_y - 1e-16 < 1e-8) {
			prev_x = (float)xpos; prev_y = (float)ypos;
			return;
		}

		if (use_arcball())
		{
			// 1. Get eye coordinate of arcball and compute its screen coordinate
			glm::vec4 arcball_eyecoord = glm::inverse(eyeRBT) * arcballRBT * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			glm::vec2 arcballCenter = eye_to_screen(
				glm::vec3(arcball_eyecoord),
				Projection,
				frameBufferWidth,
				frameBufferHeight
				);

			// compute z index
			glm::vec2 p1 = glm::vec2(prev_x, prev_y) - arcballCenter;
			glm::vec2 p2 = glm::vec2(xpos, ypos) - arcballCenter;

			glm::vec3 v1 = glm::normalize(glm::vec3(p1.x, p1.y, sqrt(max(0.0f, pow(arcBallScreenRadius, 2) - pow(p1.x, 2) - pow(p1.y, 2)))));
			glm::vec3 v2 = glm::normalize(glm::vec3(p2.x, p2.y, sqrt(max(0.0f, pow(arcBallScreenRadius, 2) - pow(p2.x, 2) - pow(p2.y, 2)))));

			glm::quat w1, w2;
			// 2. Compute arcball rotation (Chatper 8)
			if (object_index == 0 && view_index == 0 && sky_type == 0) { w1 = glm::quat(0.0f, -v1); w2 = glm::quat(0.0f, v2); }
			else { w1 = glm::quat(0.0f, v2); w2 = glm::quat(0.0f, -v1); }

			// Arcball: axis k and 2*theta (Chatper 8)
			glm::quat w = w1 * w2;
			m = glm::toMat4(w);
		}
		else // ego motion
		{
			glm::quat xRotation = glm::angleAxis((float)-dy_r * 0.1f, glm::vec3(1.0f, 0.0f, 0.0f));
			glm::quat yRotation = glm::angleAxis((float)dx_r * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));

			glm::quat w = yRotation * xRotation;
			m = glm::toMat4(w);
		}

		// Apply transformation with auxiliary frame
		setWrtFrame();
		if (object_index == 0) { skyRBT = aFrame * m * glm::inverse(aFrame) * skyRBT; }
		else
		{
			objectCenterRBT = aFrame * m * glm::inverse(aFrame) * objectCenterRBT;
			for (size_t i = 0; i < subObjects.size(); i++)
			{
				subObjects[i].RBT = aFrame * m * glm::inverse(aFrame) * subObjects[i].RBT;
			}
		}

		prev_x = (float)xpos; prev_y = (float)ypos;
	}
}


void toggleEyeMode()
{
	view_index = (view_index + 1) % 2;
	if (view_index == 0) {
		std::cout << "Using sky view" << std::endl;
	}
	else {
		std::cout << "Using object " << view_index << " view" << std::endl;
	}
}

void cycleManipulation()
{
	object_index = (object_index + 1) % 2;
	if (object_index == 0) {
		std::cout << "Manipulating sky frame" << std::endl;
	}
	else {
		std::cout << "Manipulating object " << object_index << std::endl;
	}
}

void cycleSkyAMatrix()
{
	if (object_index == 0 && view_index == 0) {
		sky_type = (sky_type + 1) % 2;
		if (sky_type == 0) {
			std::cout << "world-sky" << std::endl;
		}
		else {
			std::cout << "sky-sky" << std::endl;
		}
	}
	else {
		std::cout << "Unable to change sky mode" << std::endl;
	}
}

static void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	glm::mat4 m;
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_H:
			std::cout << "CS380 Homework Assignment 4" << std::endl;
			std::cout << "keymaps:" << std::endl;
			std::cout << "h\t\t Help command" << std::endl;
			std::cout << "v\t\t Change eye matrix" << std::endl;
			std::cout << "o\t\t Change current manipulating object" << std::endl;
			std::cout << "m\t\t Change auxiliary frame between world-sky and sky-sky" << std::endl;
			break;
		case GLFW_KEY_V:
			toggleEyeMode();
			break;
		case GLFW_KEY_O:
			cycleManipulation();
			break;
		case GLFW_KEY_M:
			cycleSkyAMatrix();
			break;
		case GLFW_KEY_1:
			cycle_color(d_color_index);
			break;
		case GLFW_KEY_2:
			cycle_color(p_color_index);
			break;
		case GLFW_KEY_3:
			cycle_color(s_color_index);
			break;
		default:
			break;
		}
	}
}

void passing_dLight(Model *m, DirectionalLight dL)
{
	vec3 d = vec3(inverse(eyeRBT) * vec4(dL.direction, 0.0f));
	glUseProgram(m->GLSLProgramID);
	glUniform3f(glGetUniformLocation(m->GLSLProgramID, "dLight.direction"), d.x, d.y, d.z);
	glUniform3f(glGetUniformLocation(m->GLSLProgramID, "dLight.illumination.ambient"), dL.ambient.r, dL.ambient.g, dL.ambient.b);
	glUniform3f(glGetUniformLocation(m->GLSLProgramID, "dLight.illumination.diffuse"), dL.diffuse.r, dL.diffuse.g, dL.diffuse.b);
	glUniform3f(glGetUniformLocation(m->GLSLProgramID, "dLight.illumination.specular"), dL.specular.r, dL.specular.g, dL.specular.b);
}
void passing_pLight(Model *m, PointLight pL)
{
	vec3 p = vec3(inverse(eyeRBT) * vec4(pL.position, 1.0f));
	glUseProgram(m->GLSLProgramID);
	glUniform3f(glGetUniformLocation(m->GLSLProgramID, "pLight.position"), p.x, p.y, p.z);
	glUniform3f(glGetUniformLocation(m->GLSLProgramID, "pLight.coefficient"), pL.coefficient.x, pL.coefficient.y, pL.coefficient.z);
	glUniform3f(glGetUniformLocation(m->GLSLProgramID, "pLight.illumination.ambient"), pL.ambient.r, pL.ambient.g, pL.ambient.b);
	glUniform3f(glGetUniformLocation(m->GLSLProgramID, "pLight.illumination.diffuse"), pL.diffuse.r, pL.diffuse.g, pL.diffuse.b);
	glUniform3f(glGetUniformLocation(m->GLSLProgramID, "pLight.illumination.specular"), pL.specular.r, pL.specular.g, pL.specular.b);
}
void passing_sLight(Model *m, SpotLight sL)
{
	vec3 p = vec3(inverse(eyeRBT) * vec4(sL.position, 1.0f));
	vec3 d = vec3(inverse(eyeRBT) * vec4(sL.direction, 0.0f));
	glUseProgram(m->GLSLProgramID);
	glUniform3f(glGetUniformLocation(m->GLSLProgramID, "sLight.position"), p.x, p.y, p.z);
	glUniform3f(glGetUniformLocation(m->GLSLProgramID, "sLight.direction"), d.x, d.y, d.z);
	glUniform1f(glGetUniformLocation(m->GLSLProgramID, "sLight.radius_inner"), sL.radius_inner);
	glUniform1f(glGetUniformLocation(m->GLSLProgramID, "sLight.radius_outer"), sL.radius_outer);
	glUniform3f(glGetUniformLocation(m->GLSLProgramID, "sLight.illumination.ambient"), sL.ambient.r, sL.ambient.g, sL.ambient.b);
	glUniform3f(glGetUniformLocation(m->GLSLProgramID, "sLight.illumination.diffuse"), sL.diffuse.r, sL.diffuse.g, sL.diffuse.b);
	glUniform3f(glGetUniformLocation(m->GLSLProgramID, "sLight.illumination.specular"), sL.specular.r, sL.specular.g, sL.specular.b);
}

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow((int)windowWidth, (int)windowHeight, "Lab 4: 20130156 - 김준", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetKeyCallback(window, keyboard_callback);

	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

	// Clear with sky color
	glClearColor((GLclampf)(128. / 255.), (GLclampf)(200. / 255.), (GLclampf)(255. / 255.), (GLclampf) 0.);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Enable culling
	// glEnable(GL_CULL_FACE);
	// Backface culling
	glCullFace(GL_BACK);

	Projection = glm::perspective(fov, windowWidth / windowHeight, 0.1f, 100.0f);
	skyRBT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.25, 8.0));

	aFrame = linearFact(skyRBT);

	// initial eye frame = sky frame;
	eyeRBT = skyRBT;

	// Initialize Ground Model
	ground = Model();
	init_ground(ground);
	ground.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");
	ground.set_projection(&Projection);
	ground.set_eye(&eyeRBT);
	glm::mat4 groundRBT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, g_groundY, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(g_groundSize, 1.0f, g_groundSize));
	ground.set_model(&groundRBT);
	ground.set_material(MATERIAL_PLASTIC_GREEN);
	object.set_shader_type(SHADER_TYPE::PHONG);

	//TODO: Initialize model by loading .obj file
	object = Model();
	//init_cube(object, vec3(1.0f));
	//init_sphere(object);
	init_obj(object, "bunny.obj", glm::vec3(1.0, 1.0, 1.0));
	object.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");
	object.set_projection(&Projection);
	object.set_eye(&eyeRBT);
	object.set_model(&objectCenterRBT);
	object.set_offset(&objectOffsetFrame);
	object.set_material(MATERIAL_COPPER);
	//object.set_material(MATERIAL_BRONZE);
	object.set_shader_type(SHADER_TYPE::FLAT);

	SUB_OBJECT_INFO soi;
	for (size_t i = 0; i < 3; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			soi.RBT = glm::translate(objectCenterRBT, 1.5f * (vec3(-1.0f, -1.0f, 0.0f) + vec3(1.0f)*vec3(i, j, 0)));
			switch (i)
			{
			case 0:
				soi.material = MATERIAL_COPPER;
				break;
			case 1:
				soi.material = MATERIAL_RUBBER_BLACK;
				break;
			case 2:
				soi.material = MATERIAL_JADE;
				break;
			default:
				soi.material = MATERIAL_PLASTIC_GREEN;
				break;
			}
			switch ((i + j) % 3)
			{
			case 0:
				soi.type = SHADER_TYPE::PHONG;
				break;
			case 1:
				soi.type = SHADER_TYPE::FLAT;
				break;
			case 2:
				soi.type = SHADER_TYPE::TOON;
				break;
			default:
				soi.type = SHADER_TYPE::LAST;
				break;
			}
			subObjects.push_back(soi);
		}
	}

	arcBall = Model();
	init_sphere(arcBall);
	arcBall.initialize(DRAW_TYPE::INDEX, "VertexShader.glsl", "FragmentShader.glsl");

	arcBall.set_projection(&Projection);
	arcBall.set_eye(&eyeRBT);
	arcBall.set_model(&arcballRBT);

	float anguler_v = 1.0f;
	float r;
	do {
		lTime = cTime;
		cTime = glfwGetTime();
		dTime = cTime - lTime;
		r = anguler_v * cTime;

		dLight.ambient = 0.005f * COLORS[d_color_index];
		dLight.diffuse = 0.4f * COLORS[d_color_index];
		dLight.specular = 0.4f * COLORS[d_color_index];

		pLight.ambient = 0.005f * COLORS[p_color_index];
		pLight.diffuse = 0.4f * COLORS[p_color_index];
		pLight.specular = 0.4f * COLORS[p_color_index];

		sLight.ambient = 0.005f * COLORS[s_color_index];
		sLight.diffuse = 0.4f * COLORS[s_color_index];
		sLight.specular = 0.4f * COLORS[s_color_index];

		dLight.direction = vec3(cos(r*0.95), -3.0, sin(r*1.05));
		sLight.direction = vec3(-sin(r*1.2), -3.0, cos(r*0.85));

		pLight.position = vec3(3.0f,1.0f,3.0f) * vec3(cos(r*0.7), cos(r*1.1), cos(r*1.3));
		sLight.position = vec3(sin(r*1.15), sin(r*1.35), sin(r*0.8))+vec3(0.0f,3.0f,0.0f);
		//std::cout << sLight.position.x << sLight.position.y << sLight.position.z << std::endl;

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		eyeRBT = (view_index == 0) ? skyRBT : objectCenterRBT;

		//TODO: pass the light value to the shader

		passing_dLight(&ground, dLight);
		passing_pLight(&ground, pLight);
		passing_sLight(&ground, sLight);

		passing_dLight(&object, dLight);
		passing_pLight(&object, pLight);
		passing_sLight(&object, sLight);

		// TODO: draw OBJ model


		// Draw wireframe of arcBall with dynamic radius
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		switch (object_index)
		{
		case 0:
			arcballRBT = (sky_type == 0) ? worldRBT : skyRBT;
			break;
		case 1:
			arcballRBT = objectCenterRBT;
			break;
		default:
			break;
		}

		ScreenToEyeScale = compute_screen_eye_scale(
			(glm::inverse(eyeRBT) * arcballRBT * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)).z,
			fovy,
			frameBufferHeight
			);
		arcBallScale = ScreenToEyeScale * arcBallScreenRadius;
		arcballRBT = arcballRBT * glm::scale(worldRBT, glm::vec3(arcBallScale, arcBallScale, arcBallScale));
		glEnable(GL_CULL_FACE);
		arcBall.draw();
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		ground.draw();
		for (size_t i = 0; i < subObjects.size(); i++)
		{
			object.set_model(&subObjects[i].RBT);
			object.set_material(subObjects[i].material);
			object.set_shader_type(subObjects[i].type);
			object.draw();
		}
		// Swap buffers (Double buffering)
		glfwSwapBuffers(window);
		glfwPollEvents();
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Clean up data structures and glsl objects
	ground.cleanup();
	object.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

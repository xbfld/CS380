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

GLint lightLocGround, lightLocRed, lightLocGreen, lightLocArc;

// View properties
glm::mat4 Projection;
float windowWidth = 1024.0f;
float windowHeight = 768.0f;
int frameBufferWidth = 0;
int frameBufferHeight = 0;
float fov = 45.0f;
float fovy = fov;

// Model properties
Model ground, redCube, greenCube;
glm::mat4 skyRBT;
glm::mat4 g_objectRbt[2] = { glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.5f, 0.0f)) * glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f)), // RBT for redCube
							glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.5f, 0.0f)) * glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f)) }; // RBT for greenCube
glm::mat4 eyeRBT;
glm::mat4 worldRBT = glm::mat4(1.0f);
glm::mat4 aFrame;

// Arcball manipulation
Model arcBall;
glm::mat4 arcballRBT;
glm::mat4 *arcballCenterRBT = &worldRBT;
float arcBallScreenRadius = 0.25f * min(windowWidth, windowHeight); // for the initial assignment
float arcBallScale = 0.01f;

// Function definition
static void window_size_callback(GLFWwindow*, int, int);
static void mouse_button_callback(GLFWwindow*, int, int, int);
static void cursor_pos_callback(GLFWwindow*, double, double);
static void keyboard_callback(GLFWwindow*, int, int, int, int);
void update_fovy(void);

// Mouse & Keyboard input related states
int press_mouse = -1;
int object_mode = 0;
bool world_sky_mode = true;
int viewpoint_mode = 0;
double last_xpos = 0.0f;
double last_ypos = 0.0f;
glm::mat4 *target_objectRBT = &skyRBT;


// Helper function: Update the vertical field-of-view(float fovy in global)
void update_fovy()
{
	if (frameBufferWidth >= frameBufferHeight)
	{
		fovy = fov;
	}
	else {
		const float RAD_PER_DEG = 0.5f * glm::pi<float>() / 180.0f;
		fovy = (float)atan2(sin(fov * RAD_PER_DEG) * ((float)frameBufferHeight / (float)frameBufferWidth), cos(fov * RAD_PER_DEG)) / RAD_PER_DEG;
	}
}

void update_aFrame()
{
	/*if ((0 == viewpoint_mode) && (0 == object_mode) && (0 == world_sky_mode))
	{
		aFrame = transFact(worldRBT)*linearFact(eyeRBT);
	}*/
	aFrame = transFact(*arcballCenterRBT)*linearFact(eyeRBT);
}

void update_eye()
{
	switch (viewpoint_mode)
	{
	case 0:
		eyeRBT = skyRBT;
		break;
	case 1:
		eyeRBT = g_objectRbt[0];
		break;
	case 2:
		eyeRBT = g_objectRbt[1];
		break;
	default:
		break;
	}
}

void update_arcBallScale()
{
	double _z;
	if ((0 == viewpoint_mode) && (0 == object_mode) && (world_sky_mode))
	{
		_z = transFact(glm::inverse(eyeRBT) * worldRBT)[3].z;
	}
	_z = transFact(glm::inverse(eyeRBT)* *arcballCenterRBT)[3].z;
	arcBallScale = compute_screen_eye_scale(_z, fovy, frameBufferHeight);
}

void update_arcBallRBT()
{
	arcballRBT = *arcballCenterRBT * glm::scale(vec3(arcBallScale * arcBallScreenRadius));
}

vec3 get_target_center()
{
	return vec3(transFact(glm::inverse(eyeRBT)* *target_objectRBT)[3]);
}

quat get_arcball_quat(double x, double y)
{
	vec2 arc_screen_center = eye_to_screen(get_target_center(), Projection, frameBufferWidth, frameBufferHeight);
	vec2 d_pos = vec2(x, y) - arc_screen_center;
	double _z = pow(arcBallScreenRadius, 2.0f) - glm::dot(d_pos, d_pos);
	_z = (_z < 0) ? 0 : sqrt(_z);
	return quat(0, glm::normalize(vec3(d_pos, _z)));
}

void print_help()
{
	std::cout << "CS380 Homework Assignment 2" << std::endl;
	std::cout << "keymaps:" << std::endl;
	std::cout << "h\t\t Help command" << std::endl;
	std::cout << "v\t\t Change eye frame (your viewpoint)" << std::endl;
	std::cout << "o\t\t Change current manipulating object" << std::endl;
	std::cout << "m\t\t Change auxiliary frame between world-sky and sky-sky" << std::endl;
	std::cout << "c\t\t Change manipulation method" << std::endl;
}

// TODO: Modify GLFW window resized callback function
static void window_size_callback(GLFWwindow* window, int width, int height)
{
	// Get resized size and set to current window
	windowWidth = (float)width;
	windowHeight = (float)height;

	// glViewport accept pixel size, please use glfwGetFramebufferSize rather than window size.
	// window size != framebuffer size
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
	glViewport(0, 0, (GLsizei)frameBufferWidth, (GLsizei)frameBufferHeight);

	// Update projection matrix
	Projection = glm::perspective(fov, ((float)frameBufferWidth / (float)frameBufferHeight), 0.1f, 100.0f);

	// Update arcball radius
	arcBallScreenRadius = 0.25f * min((float)frameBufferWidth, (float)frameBufferHeight);
}

// TODO: Fill up GLFW mouse button callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	printf("mouse button callback: %d, %d, %d\n", button, action, mods);
	// button: 0,1,2 -> L,R,M
	// action: 0,1 -> release, press
	// mods: comb(1,2,4) -> Shift, Ctrl, Alt
	if (press_mouse == -1)
	{
		if (action == GLFW_PRESS)
		{
			press_mouse = button;
			glfwGetCursorPos(window, &last_xpos, &last_ypos);
			update_aFrame();
		}
	}
	else
	{
		if (action == GLFW_RELEASE && press_mouse == button)
		{
			press_mouse = -1;
			update_arcBallScale();
		}
	}
}

// TODO: Fill up GLFW cursor position callback function
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	double d_x = xpos - last_xpos;
	double d_y = ypos - last_ypos;
	quat last_quat;
	quat cur_quat;
	quat d_quat;
	mat4 manipulate = mat4(1.0f);
	switch (press_mouse)
	{
	case -1:
		break;
	case GLFW_MOUSE_BUTTON_1:
		// Left Mouse Button Pressed
		printf("cursor pos callbakc: Left, %f, %f\n", xpos, ypos);
		if (viewpoint_mode != object_mode)
		{
			last_quat = get_arcball_quat(last_xpos, frameBufferHeight - 1 - last_ypos);
			cur_quat = get_arcball_quat(xpos, frameBufferHeight - 1 - ypos);
			d_quat = cur_quat*inverse(last_quat);
			manipulate = glm::toMat4(d_quat);
		}
		else
		{
			double x_rad = d_x / arcBallScreenRadius;
			double y_rad = d_y / arcBallScreenRadius;
			manipulate = glm::toMat4(quat(cos(-y_rad), vec3(sin(-y_rad), 0.0f, 0.0f))*quat(cos(x_rad), vec3(0.0f, x_rad, 0.0f)));

			manipulate = inverse(manipulate);
		}
		*target_objectRBT = aFrame * manipulate * glm::inverse(aFrame) * *target_objectRBT;
		last_xpos = xpos;
		last_ypos = ypos;
		break;
	case GLFW_MOUSE_BUTTON_2:
		// Right Mouse Button Pressed
		printf("cursor pos callbakc: Right, %f, %f\n", xpos, ypos);
		if (viewpoint_mode != object_mode)
		{
			manipulate = glm::translate(vec3(d_x, -d_y, 0.0f) * arcBallScale);
		}
		else
		{
			manipulate = glm::translate(vec3(d_x, -d_y, 0.0f) / arcBallScreenRadius);
		}
		*target_objectRBT = aFrame * manipulate * glm::inverse(aFrame) * *target_objectRBT;
		last_xpos = xpos;
		last_ypos = ypos;
		break;
	case GLFW_MOUSE_BUTTON_3:
		// Middle Mouse Button Pressed
		printf("cursor pos callbakc: Middle, %f, %f\n", xpos, ypos);
		if (viewpoint_mode != object_mode)
		{
			manipulate = glm::translate(vec3(0.0f, 0.0f, d_y) * arcBallScale);
		}
		else
		{
			manipulate = glm::translate(vec3(0.0f, 0.0f, d_y) / arcBallScreenRadius);
		}
		*target_objectRBT = aFrame * manipulate * glm::inverse(aFrame) * *target_objectRBT;
		last_xpos = xpos;
		last_ypos = ypos;
		break;

	default:
		break;
	}
	update_aFrame();
	//update_arcBallScale();
	update_arcBallRBT();
}

static void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (press_mouse == -1)
		{
			switch (key)
			{
			case GLFW_KEY_H:
				print_help();
				break;
			case GLFW_KEY_V:
				// DONE: Change viewpoint
				switch (viewpoint_mode)
				{
				case 0:
					viewpoint_mode = 1;
					printf("viewpoint_mode: cube1\n");
					break;
				case 1:
					viewpoint_mode = 2;
					printf("viewpoint_mode: cube2\n");
					break;
				case 2:
					viewpoint_mode = 0;
					printf("viewpoint_mode: sky\n");
					break;
				default:
					printf("viewpoint_mode: %d // Unreachable\n", object_mode);
					break;
				}
				update_eye();

				break;
			case GLFW_KEY_O:
				// DONE: Change manipulating object
				switch (object_mode)
				{
				case 0:
					object_mode = 1;
					target_objectRBT = &g_objectRbt[0];
					arcballCenterRBT = target_objectRBT;
					printf("object_mode: cube1\n");
					break;
				case 1:
					object_mode = 2;
					target_objectRBT = &g_objectRbt[1];
					arcballCenterRBT = target_objectRBT;
					printf("object_mode: cube2\n");
					break;
				case 2:
					object_mode = 0;
					if (viewpoint_mode != 0)
					{
						printf("It is not allowed modifying the sky camera when the current camera view is a cube view.\n");
					}
					else
					{
						target_objectRBT = &skyRBT;
						if (world_sky_mode)
						{
							arcballCenterRBT = &worldRBT;
							printf("object_mode: world-sky\n");
						}
						else
						{
							printf("object_mode: sky-sky\n");
						}
					}
					break;
				default:
					printf("object_mode: %d // Unreachable\n", object_mode);
					break;
				}
				update_arcBallScale();
				update_arcBallRBT();

				break;
			case GLFW_KEY_M:
				// DONE: Change auxiliary frame between world-sky and sky-sky
				if ((0 == viewpoint_mode) && (0 == object_mode))
				{
					if (!world_sky_mode)
					{
						world_sky_mode = !world_sky_mode;
						arcballCenterRBT = &worldRBT;
						update_arcBallScale();
						update_arcBallRBT();
						printf("object_mode: world-sky\n");
					}
					else
					{
						world_sky_mode = !world_sky_mode;
						printf("object_mode: sky-sky\n");
					}
				}
				break;
			case GLFW_KEY_C:
				// TODO: Add an additional manipulation method
				break;
			default:
				break;
			}
			//update_aFrame();
		}
		else
		{
			switch (key)
			{
			case GLFW_KEY_H:
				print_help();
				break;
			default:
				std::cout << "Before changing view or object modes, release the mouse button " << std::endl;
				break;
			}
		}
	}
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
	window = glfwCreateWindow((int)windowWidth, (int)windowHeight, "Homework 2: ", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = (GLboolean)true; // Needed for core profile
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
	// Update arcBallScreenRadius with framebuffer size
	arcBallScreenRadius = 0.25f * min((float)frameBufferWidth, (float)frameBufferHeight); // for the initial assignment

	// Clear with sky color
	glClearColor((GLclampf)(128. / 255.), (GLclampf)(200. / 255.), (GLclampf)(255. / 255.), (GLclampf) 0.);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Enable culling
	glEnable(GL_CULL_FACE);
	// Backface culling
	glCullFace(GL_BACK);

	Projection = glm::perspective(fov, ((float)frameBufferWidth / (float)frameBufferHeight), 0.1f, 100.0f);
	skyRBT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.25, 4.0));

	// initial eye frame = sky frame;
	eyeRBT = skyRBT;

	// Initialize Ground Model
	ground = Model();
	init_ground(ground);
	ground.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl"); //
	ground.set_projection(&Projection);
	ground.set_eye(&eyeRBT);
	glm::mat4 groundRBT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, g_groundY, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(g_groundSize, 1.0f, g_groundSize));
	ground.set_model(&groundRBT);

	redCube = Model();
	init_cube(redCube, glm::vec3(1.0, 0.0, 0.0));
	redCube.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");

	redCube.set_projection(&Projection);
	redCube.set_eye(&eyeRBT);
	redCube.set_model(&g_objectRbt[0]);

	greenCube = Model();
	init_cube(greenCube, glm::vec3(0.0, 1.0, 0.0));
	greenCube.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");

	greenCube.set_projection(&Projection);
	greenCube.set_eye(&eyeRBT);
	greenCube.set_model(&g_objectRbt[1]);

	// TODO: Initialize arcBall
	// Initialize your arcBall with DRAW_TYPE::INDEX (it uses GL_ELEMENT_ARRAY_BUFFER to draw sphere)
	arcBall = Model();
	init_sphere(arcBall);
	arcBall.initialize(DRAW_TYPE::INDEX, "VertexShader.glsl", "FragmentShader.glsl");

	arcBall.set_projection(&Projection);
	arcBall.set_eye(&eyeRBT);
	arcBall.set_model(&arcballRBT);

	update_arcBallScale();
	update_arcBallRBT();

	// Setting Light Vectors
	glm::vec3 lightVec = glm::vec3(0.0f, 1.0f, 0.0f);
	lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "uLight");
	glUniform3f(lightLocGround, lightVec.x, lightVec.y, lightVec.z);

	lightLocRed = glGetUniformLocation(redCube.GLSLProgramID, "uLight");
	glUniform3f(lightLocRed, lightVec.x, lightVec.y, lightVec.z);

	lightLocGreen = glGetUniformLocation(greenCube.GLSLProgramID, "uLight");
	glUniform3f(lightLocGreen, lightVec.x, lightVec.y, lightVec.z);

	lightLocArc = glGetUniformLocation(arcBall.GLSLProgramID, "uLight");
	glUniform3f(lightLocArc, lightVec.x, lightVec.y, lightVec.z);

	do {
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// DONE: Change Viewpoint with respect to your current view index
		update_eye();
		update_aFrame();

		redCube.draw();
		greenCube.draw();
		ground.draw();

		// DONE: Draw wireframe of arcball with dynamic radius
		if ((object_mode != viewpoint_mode)
			|| ((viewpoint_mode == 0) && (world_sky_mode)))
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			arcBall.draw();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// Swap buffers (Double buffering)
		glfwSwapBuffers(window);
		glfwPollEvents();
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Clean up data structures and glsl objects
	ground.cleanup();
	redCube.cleanup();
	greenCube.cleanup();
	arcBall.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

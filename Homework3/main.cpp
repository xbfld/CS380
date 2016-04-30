// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

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
#include <common/picking.hpp>

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
glm::mat4 g_objectRbt[2] = {
	glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.5f, 0.0f)) * glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f)), // RBT for redCube
	glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.5f, 0.0f)) * glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f)) }; // RBT for greenCube
glm::mat4 eyeRBT;
glm::mat4 worldRBT = glm::mat4(1.0f);
glm::mat4 aFrame;

// Rubic's cube pieces
Model rubixModel[9];
glm::mat4 g_rubixRbt[9];
int rubix_w = 3;	// # of columns
int rubix_h = 3;	// # of rows
int rubix_d = 1;	// # of layers
GLint lightLocRubix[9];
int offsetID = 100;

std::vector<int> pickedIDs;

// Arcball manipulation
Model arcBall;
glm::mat4 arcballRBT = glm::mat4(1.0f);
float arcBallScreenRadius = 0.25f * min(windowWidth, windowHeight); // for the initial assignment
float screenToEyeScale = 0.01f;
float arcBallScale = 0.01f;

// Function definition
static void window_size_callback(GLFWwindow*, int, int);
static void mouse_button_callback(GLFWwindow*, int, int, int);
static void cursor_pos_callback(GLFWwindow*, double, double);
static void keyboard_callback(GLFWwindow*, int, int, int, int);
void update_fovy(void);

void selection_checking(void);


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

	// re-allocate textures with respect to new framebuffer width and height
	reallocate_picking_texture(frameBufferWidth, frameBufferHeight);

	// Update projection matrix
	Projection = glm::perspective(fov, ((float)frameBufferWidth / (float)frameBufferHeight), 0.1f, 100.0f);
}

// TODO: Fill up GLFW mouse button callback function
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	//example code for picking
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		int target = pick((int)xpos, (int)ypos, frameBufferWidth, frameBufferHeight);
		//std::cout << "Picked node: " << target << std::endl;
		pickedIDs.push_back(target);

		// Print recent picked ID
		std::cout << "Picked node: ";
		for (size_t i = 0; i < pickedIDs.size(); i++)
		{
			std::cout << pickedIDs[i] << " ";
		}
		std::cout << std::endl;

		selection_checking();
	}
}

// TODO: Fill up GLFW cursor position callback function
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{

}

static void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_H:
			std::cout << "CS380 Homework Assignment 3" << std::endl;
			std::cout << "keymaps:" << std::endl;
			std::cout << "h\t\t Help command" << std::endl;
			std::cout << "p\t\t Enable/Disable picking" << std::endl;
			break;
		case GLFW_KEY_P:
			// TODO: Enable/Disable picking
			break;
		default:
			break;
		}
	}
}

// Helper function: Return index of Rubix piece
int rubix_index(int w, int h, int d)
{
	return w + rubix_w * (h + rubix_h * d);
}
int rubix_index(ivec3 p)
{
	return rubix_index(p.x,p.y,p.z);
}
// Helper function: Decode PickingID to w, h, d
glm::ivec3 rubix_decode(int id)
{
	glm::ivec3 result(-1);
	id -= offsetID;
	result.x = id % rubix_w;
	id /= rubix_w;
	result.y = id % rubix_h;
	id /= rubix_h;
	result.z = id;
	return result;
}

// Initialize Rubix Model
// Setting Light Vector
void rubix_setup()
{
	int r_index;
	Model *r_model;
	vec3 rubic_color[6] = {
		glm::vec3(1.0, 1.0, 0.0), glm::vec3(0.0, 1.0, 1.0), glm::vec3(1.0, 0.0, 1.0),
		glm::vec3(0.0, 0.0, 1.0) , glm::vec3(1.0, 0.0, 0.0) , glm::vec3(0.0, 1.0, 0.0) };

	glm::vec3 pos_offset = glm::vec3(-0.5f*(rubix_w - 1), -0.5f*(rubix_h - 1), -0.5f*(rubix_d - 1));

	for (size_t d = 0; d < rubix_d; d++)
	{
		for (size_t h = 0; h < rubix_h; h++)
		{
			for (size_t w = 0; w < rubix_w; w++)
			{
				r_index = rubix_index(w, h, d);
				r_model = &(rubixModel[r_index]);
				g_rubixRbt[r_index] = glm::translate(glm::mat4(1.0f), pos_offset + glm::vec3(1.0f * w, 1.0f * h, 1.0f * d));

				// Initialize Rubix Piece Model
				*r_model = Model();
				init_rubic(*r_model, &rubic_color[0]);
				r_model->initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");
				r_model->initialize_picking("PickingVertexShader.glsl", "PickingFragmentShader.glsl");
				r_model->set_projection(&Projection);
				r_model->set_eye(&eyeRBT);
				r_model->set_model(&g_rubixRbt[r_index]);

				r_model->objectID = r_index + offsetID;

				// Setting Light Vectors
				glm::vec3 lightVec = glm::vec3(0.0f, 1.0f, 0.0f);
				lightLocGreen = glGetUniformLocation(r_model->GLSLProgramID, "uLight");
				glUniform3f(lightLocRubix[r_index], lightVec.x, lightVec.y, lightVec.z);
			}
		}
	}
}

void rubix_draw_picking()
{
	// TODO: Replace literal 9 to variable
	for (size_t r = 0; r < 9; r++)
	{
		rubixModel[r].drawPicking();
	}
}

void rubix_draw()
{
	// TODO: Replace literal 9 to variable
	for (size_t r = 0; r < 9; r++)
	{
		rubixModel[r].draw();
	}
}

void rubix_cleanup()
{
	// TODO: Replace literal 9 to variable
	for (size_t r = 0; r < 9; r++)
	{
		rubixModel[r].cleanup();
	}
}

// Helper function: Find line or plane contains picked pieces
glm::ivec3 rubix_common()
{
	if (pickedIDs.empty())
	{
		return glm::ivec3(-1);
	}
	
	glm::ivec3 result(-2);
	glm::ivec3 p;
	for (size_t i = 0; i < pickedIDs.size(); i++)
	{
		p = rubix_decode(pickedIDs[i]);
		// Find common part
		for (size_t j = 0; j < 3; j++)
		{
			if (-2 == result[j])
			{
				result[j] = p[j];
			}
			else if (p[j] != result[j])
			{
				result[j] = -1;
			}
		}
	}

	return result;
}

// Check selections
// TODO: Setting aFrame
void selection_checking()
{
	// Check selelction validity
	if (pickedIDs.size() >= 2)
	{
		glm::ivec3 common = rubix_common();
		std::cout << "common: "
			<< common.x << " "
			<< common.y << " "
			<< common.z << std::endl;
		pickedIDs.clear();

		// Setting aFrame
		if (common.x != -1)
		{
			if (-1 == common.y)
			{
				common.y = rubix_h / 2;
			}
			aFrame = transFact(g_rubixRbt[rubix_index(common)])*linearFact(eyeRBT);
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
	window = glfwCreateWindow((int)windowWidth, (int)windowHeight, "Homework 3: 20130156 - 김준", NULL, NULL);
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

	// Initialize framebuffer object and picking textures
	picking_initialize(frameBufferWidth, frameBufferHeight);

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

	/*redCube = Model();
	init_cube(redCube, glm::vec3(1.0, 0.0, 0.0));
	redCube.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");
	redCube.initialize_picking("PickingVertexShader.glsl", "PickingFragmentShader.glsl");

	redCube.set_projection(&Projection);
	redCube.set_eye(&eyeRBT);
	redCube.set_model(&g_objectRbt[0]);

	redCube.objectID = 2;

	greenCube = Model();
	vec3 rubic_color[6] =
	{ glm::vec3(1.0, 1.0, 0.0), glm::vec3(0.0, 1.0, 1.0), glm::vec3(1.0, 0.0, 1.0),
		glm::vec3(0.0, 0.0, 1.0) , glm::vec3(1.0, 0.0, 0.0) , glm::vec3(0.0, 1.0, 0.0) };
	init_rubic(greenCube, &rubic_color[0]);
	greenCube.initialize(DRAW_TYPE::ARRAY, "VertexShader.glsl", "FragmentShader.glsl");
	greenCube.initialize_picking("PickingVertexShader.glsl", "PickingFragmentShader.glsl");
	greenCube.set_projection(&Projection);
	greenCube.set_eye(&eyeRBT);
	greenCube.set_model(&g_objectRbt[1]);

	greenCube.objectID = 3;*/

	// DONE: Initialize arcBall
	// Initialize your arcBall with DRAW_TYPE::INDEX (it uses GL_ELEMENT_ARRAY_BUFFER to draw sphere)
	arcBall = Model();
	init_sphere(arcBall);
	arcBall.initialize(DRAW_TYPE::INDEX, "VertexShader.glsl", "FragmentShader.glsl");
	arcBall.set_projection(&Projection);
	arcBall.set_eye(&eyeRBT);
	arcBall.set_model(&arcballRBT);

	arcBall.objectID = 1;

	// Setting Light Vectors
	glm::vec3 lightVec = glm::vec3(0.0f, 1.0f, 0.0f);
	lightLocGround = glGetUniformLocation(ground.GLSLProgramID, "uLight");
	glUniform3f(lightLocGround, lightVec.x, lightVec.y, lightVec.z);

	/*lightLocRed = glGetUniformLocation(redCube.GLSLProgramID, "uLight");
	glUniform3f(lightLocRed, lightVec.x, lightVec.y, lightVec.z);

	lightLocGreen = glGetUniformLocation(greenCube.GLSLProgramID, "uLight");
	glUniform3f(lightLocGreen, lightVec.x, lightVec.y, lightVec.z);*/

	lightLocArc = glGetUniformLocation(arcBall.GLSLProgramID, "uLight");
	glUniform3f(lightLocArc, lightVec.x, lightVec.y, lightVec.z);

	rubix_setup();

	pickedIDs = std::vector<int>();

	do {
		// first pass: picking shader
		// binding framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, picking_fbo);
		// Background: RGB = 0x000000 => objectID: 0
		glClearColor((GLclampf) 0.0f, (GLclampf) 0.0f, (GLclampf) 0.0f, (GLclampf) 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// drawing objects in framebuffer (picking process)
		/*redCube.drawPicking();
		greenCube.drawPicking();*/
		rubix_draw_picking();

		// second pass: your drawing
		// unbinding framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor((GLclampf)(128. / 255.), (GLclampf)(200. / 255.), (GLclampf)(255. / 255.), (GLclampf)0.);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/*redCube.draw();
		greenCube.draw();*/
		rubix_draw();

		ground.draw();

		// DONE: Draw wireframe of arcball with dynamic radius
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		arcBall.draw();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Swap buffers (Double buffering)
		glfwSwapBuffers(window);
		glfwPollEvents();
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Clean up data structures and glsl objects
	ground.cleanup();
	/*redCube.cleanup();
	greenCube.cleanup();*/
	arcBall.cleanup();
	rubix_cleanup();

	pickedIDs.clear();

	// Cleanup textures
	delete_picking_resources();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

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


#include <common/shader.hpp>
#include <common/affine.hpp>
#include <common/geometry.hpp>
#include <common/arcball.hpp>
#include <common/texture.hpp>

using namespace glm;

float g_groundSize = 100.0f;
float g_groundY = -2.5f;

GLuint lightLocCube;
GLuint isSky, isEye;

GLuint addPrograms[3];
GLuint texture[9];
GLuint textureID[3][9];
GLuint bumpTex;
GLuint bumpTexID;
GLuint cubeTex;
GLuint cubeTexID;

// View properties
glm::mat4 Projection;
float windowWidth = 1024.0f;
float windowHeight = 768.0f;
int frameBufferWidth = 0;
int frameBufferHeight = 0;
float fov = 60.0f;
float fovy = fov;
bool animate = true;

// Model properties
glm::mat4 skyRBT;
glm::mat4 eyeRBT;
const glm::mat4 worldRBT = glm::mat4(1.0f);
glm::mat4 arcballRBT = glm::mat4(1.0f);
glm::mat4 aFrame;
//cubes
glm::mat4 objectRBT[9];
Model cubes[9];
int program_cnt = 1;
int cube_index[9]{ 0,1,2,3,4,5,6,7,8 };
//cube animation
int rot_line = 0;
float ani_delta = 10.0f;

//Sky box
Model skybox;
glm::mat4 skyboxRBT = glm::translate(0.0f, 0.0f, 0.0f);//Will be fixed(cause it is the sky)

vec3 eyePosition = vec3(0.0, 0.0, 6.0);
// Mouse interaction
bool MOUSE_LEFT_PRESS = false; bool MOUSE_MIDDLE_PRESS = false; bool MOUSE_RIGHT_PRESS = false;

// Transformation
glm::mat4 m = glm::mat4(1.0f);

// Manipulation index
int object_index = 0; int view_index = 0; int sky_type = 0;

// Arcball manipulation
Model arcBall;
float arcBallScreenRadius = 0.25f * min(windowWidth, windowHeight);
float arcBallScale = 0.01f; float ScreenToEyeScale = 0.01f;
float prev_x = 0.0f; float prev_y = 0.0f;

GLenum  cube[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X,
GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };

void init_cubeRBT() {
	objectRBT[0] = glm::translate(-1.2f, 1.2f, 0.f) * glm::scale(1.1f, 1.1f, 1.1f);
	objectRBT[1] = glm::translate(0.0f, 1.2f, 0.f) * glm::scale(1.1f, 1.1f, 1.1f);
	objectRBT[2] = glm::translate(1.2f, 1.2f, 0.f) * glm::scale(1.1f, 1.1f, 1.1f);
	objectRBT[3] = glm::translate(-1.2f, 0.0f, 0.f) * glm::scale(1.1f, 1.1f, 1.1f);
	objectRBT[4] = glm::translate(0.0f, 0.0f, 0.f) * glm::scale(1.1f, 1.1f, 1.1f);//Center
	objectRBT[5] = glm::translate(1.2f, 0.0f, 0.f) * glm::scale(1.1f, 1.1f, 1.1f);
	objectRBT[6] = glm::translate(-1.2f, -1.2f, 0.f) * glm::scale(1.1f, 1.1f, 1.1f);
	objectRBT[7] = glm::translate(0.0f, -1.2f, 0.f) * glm::scale(1.1f, 1.1f, 1.1f);
	objectRBT[8] = glm::translate(1.2f, -1.2f, 0.f) * glm::scale(1.1f, 1.1f, 1.1f);
}
void set_program(int p) {
	for (int i = 0; i < 9; i++) {
		cubes[i].GLSLProgramID = addPrograms[p];
	}
}
void init_shader(int idx, const char * vertexShader_path, const char * fragmentShader_path) {
	addPrograms[idx] = LoadShaders(vertexShader_path, fragmentShader_path);
	glUseProgram(addPrograms[idx]);
}
void init_cubemap(const char * baseFileName, int size) {
	glActiveTexture(GL_TEXTURE0 + 3);
	glGenTextures(1, &cubeTexID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexID);
	const char * suffixes[] = { "posx", "negx", "posy", "negy", "posz", "negz" };
	GLuint targets[] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};
	GLint w, h;
	//glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, size, size);
	for (int i = 0; i < 6; i++) {
		std::string texName = std::string(baseFileName) + "_" + suffixes[i] + ".bmp";
		unsigned char* data = loadBMP_cube(texName.c_str(), &w, &h);
		glTexImage2D(targets[i], 0, GL_RGB, w, h, 0,
			GL_BGR, GL_UNSIGNED_BYTE, data);
		delete[] data;
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 3);
}
void init_texture(void) {
	//TODO: Initialize first texture
	const char * suffixes[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8" };
	for (size_t j = 0; j < 9; j++)
	{
		std::string texName = std::string() + "cube_" + suffixes[j] + ".bmp";
		texture[j] = loadBMP_custom(texName.c_str());
		for (int i = 0; i < 3; i++) textureID[i][j] = glGetUniformLocation(addPrograms[i], "myTextureSampler");
	}
	//texture[0] = loadBMP_custom("cube.bmp");
	//for (int i = 0; i < 3; i++) textureID[i][0] = glGetUniformLocation(addPrograms[i], "myTextureSampler");

	////TODO: Initialize second texture
	//texture[1] = loadBMP_custom("brick.bmp");
	//for (int i = 0; i < 3; i++) textureID[i][1] = glGetUniformLocation(addPrograms[i], "myTextureSampler");
	//TODO: Initialize bump texture
	bumpTex = loadBMP_custom("brick_bump.bmp");
	bumpTexID = glGetUniformLocation(addPrograms[1], "myBumpSampler");

	//TODO: Initialize Cubemap texture	
	init_cubemap("beach", 2048);
}
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
		aFrame = transFact(objectRBT[0]) * linearFact(eyeRBT);
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
		else { objectRBT[0] = aFrame * m * glm::inverse(aFrame) * objectRBT[0]; }

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
		std::vector<int> line{ GLFW_KEY_Q , GLFW_KEY_A , GLFW_KEY_W, GLFW_KEY_D, GLFW_KEY_R, GLFW_KEY_F, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_S, GLFW_KEY_E, GLFW_KEY_X, GLFW_KEY_C};
		for (size_t i = 0; i < line.size(); i++)
		{
			if (key == line[i] && rot_line == 0)
				rot_line = i+1;
		}
		switch (key)
		{
		case GLFW_KEY_4:
			if (animate) animate = false;
			else animate = true;
			break;
		case GLFW_KEY_P://Change Programs
			program_cnt++;
			if (program_cnt > 2)
				program_cnt = 0;
			set_program(program_cnt);
			break;
		default:
			break;
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
	window = glfwCreateWindow((int)windowWidth, (int)windowHeight, "Homework5: 20130156 김준", NULL, NULL);
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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);


	Projection = glm::perspective(fov, windowWidth / windowHeight, 0.1f, 100.0f);
	skyRBT = glm::translate(glm::mat4(1.0f), eyePosition);

	aFrame = linearFact(skyRBT);
	// initial eye frame = sky frame;
	eyeRBT = skyRBT;

	//init shader
	init_shader(0, "VertexShader.glsl", "FragmentShader.glsl");
	init_shader(1, "BumpVertexShader.glsl", "BumpFragmentShader.glsl");
	init_shader(2, "EnvVertexShader.glsl", "EnvFragmentShader.glsl");

	//TODO: Initialize cube model by calling textured cube model
	init_cubeRBT();
	cubes[0] = Model();
	init_texture_cube(cubes[0]);
	cubes[0].initialize(DRAW_TYPE::ARRAY, addPrograms[0]);

	cubes[0].set_projection(&Projection);
	cubes[0].set_eye(&eyeRBT);
	cubes[0].set_model(&objectRBT[0]);
	for (int i = 1; i < 9; i++) {
		cubes[i] = Model();
		cubes[i].initialize(DRAW_TYPE::ARRAY, cubes[0]);

		cubes[i].set_projection(&Projection);
		cubes[i].set_eye(&eyeRBT);
		cubes[i].set_model(&objectRBT[i]);
	}

	skybox = Model();
	init_skybox(skybox);
	skybox.initialize(DRAW_TYPE::ARRAY, addPrograms[2]);
	skybox.set_projection(&Projection);
	skybox.set_eye(&eyeRBT);
	skybox.set_model(&skyboxRBT);


	arcBall = Model();
	init_sphere(arcBall);
	arcBall.initialize(DRAW_TYPE::INDEX, cubes[0].GLSLProgramID);

	arcBall.set_projection(&Projection);
	arcBall.set_eye(&eyeRBT);
	arcBall.set_model(&arcballRBT);

	//init textures
	init_texture();

	mat4 oO[9];
	for (int i = 0; i < 9; i++) oO[i] = objectRBT[i];
	float angle = 0.0f;
	double pre_time = glfwGetTime();
	program_cnt = 0;
	set_program(0);

	//first column rotation
	int ani_count = 0;
	float ani_angle = 0.0f;
	mat4 curRBT[9];
	for (int i = 0; i < 9; i++) curRBT[i] = objectRBT[i];

	do {
		double cur_time = glfwGetTime();
		// Clear the screen
		if (cur_time - pre_time > 0.008) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			eyeRBT = (view_index == 0) ? skyRBT : objectRBT[0];

			std::vector<int> line;
			int direction = (rot_line%2)*2-1;
			vec3 rot_axis = (rot_line<=6) ? vec3(-1.f,0.f,0.f) : vec3(0.f, -1.f, 0.f);

			switch (rot_line)
			{
			case 1:
			case 2:
				line = { 0,3,6 };
				break;
			case 3:
			case 4:
				line = { 1,4,7 };
				break;
			case 5:
			case 6:
				line = { 2,5,8 };
				break;
			case 7:
			case 8:
				line = { 0,1,2 };
				break;
			case 9:
			case 10:
				line = { 3,4,5 };
				break;
			case 11:
			case 12:
				line = { 6,7,8 };
				break;
			default:
				break;
			}

			//cube rotation
			if(rot_line>0)
			{
				ani_angle += 0.3f;
				for (size_t i = 0; i < line.size(); i++)
				{
					objectRBT[cube_index[line[i]]] = glm::rotate(glm::mat4(1.0f), ani_angle * ani_delta * direction, rot_axis) * curRBT[cube_index[line[i]]];
				}
				ani_count++;
				if (ani_count > 59) {
					for (size_t i = 0; i < line.size(); i++)
					{
						curRBT[cube_index[line[i]]] = objectRBT[cube_index[line[i]]];
					}
					int _i0 = cube_index[line[2]];
					int _i1 = cube_index[line[1]];
					int _i2 = cube_index[line[0]];
					cube_index[line[0]] = _i0;
					cube_index[line[1]] = _i1;
					cube_index[line[2]] = _i2;
					rot_line = 0;
					ani_angle = 0.0f;
					ani_count = 0;
				}
			}

			glm::vec3 lightVec = glm::vec3(sin(angle), 0.0f, cos(angle));
			glm::vec4 pLightPos = inverse(eyeRBT) * vec4(0.0f, 2.0f * cos(angle), 2.0f * sin(angle), 1.0f);
			glm::vec4 sDest = vec4(2.0f * cos(angle), -2.0f, 2.0f * sin(angle), 1.0f);
			glm::vec4 sLightPoss = vec4(0.0f, 4.0f, 0.0f, 1.0f);
			glm::vec4 sLightPos = inverse(eyeRBT)  * vec4(0.0f, 4.0f, 0.0f, 1.0f);
			glm::vec4 sLightDir = inverse(eyeRBT) * (sDest - sLightPoss);

			if (animate)
				angle += 0.02f;
			if (angle > 360.0f) angle -= 360.0f;

			//TODO: draw OBJ models		
			//TODO: pass the light value (uniform variables) to shader
			//TODO: pass the texture value to shader

			if (program_cnt == 2) {
				isSky = glGetUniformLocation(addPrograms[2], "DrawSkyBox");
				glUniform1i(isSky, 0);
				//TODO: pass the cubemap texture to shader
				glActiveTexture(GL_TEXTURE0 + 3);
				glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexID);
				glUniform1i(cubeTex, 3);

			}
			//TODO: pass the first texture value to shader			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture[0]);
			glUniform1i(textureID[program_cnt][0], 0);

			//draw first cube models
			glUseProgram(cubes[0].GLSLProgramID);
			lightLocCube = glGetUniformLocation(cubes[0].GLSLProgramID, "uLight");
			glUniform3f(lightLocCube, lightVec.x, lightVec.y, lightVec.z);
			lightLocCube = glGetUniformLocation(cubes[0].GLSLProgramID, "vLight");
			glUniform3f(lightLocCube, pLightPos.x, pLightPos.y, pLightPos.z);
			cubes[0].draw();

			//TODO: pass bump(normalmap) texture value to shader
			if (program_cnt == 1) {
				glActiveTexture(GL_TEXTURE0 + 2);
				glBindTexture(GL_TEXTURE_2D, bumpTex);
				glUniform1i(bumpTexID, 2);
			}


			for (int i = 1; i < 9; i++) {
				//TODO: pass second texture value to shader						
				glActiveTexture(GL_TEXTURE0 + 10 + i);
				glBindTexture(GL_TEXTURE_2D, texture[i]);
				glUniform1i(textureID[program_cnt][i], 10 + i);
				//draw second cube models
				glUseProgram(cubes[i].GLSLProgramID);
				lightLocCube = glGetUniformLocation(cubes[i].GLSLProgramID, "uLight");
				glUniform3f(lightLocCube, lightVec.x, lightVec.y, lightVec.z);
				lightLocCube = glGetUniformLocation(cubes[i].GLSLProgramID, "vLight");
				glUniform3f(lightLocCube, pLightPos.x, pLightPos.y, pLightPos.z);
				cubes[i].draw2(cubes[0]);
			}


			if (program_cnt == 2) {
				isSky = glGetUniformLocation(addPrograms[2], "DrawSkyBox");
				glUniform1i(isSky, 1);

				//TODO: Pass the texture(cubemap value to shader) and eye position
				glUseProgram(addPrograms[2]);
				isEye = glGetUniformLocation(addPrograms[2], "WorldCameraPosition");
				glUniform3f(isEye, eyePosition.x, eyePosition.y, eyePosition.z);
				cubeTex = glGetUniformLocation(addPrograms[2], "cubemap");
				glActiveTexture(GL_TEXTURE0 + 3);
				glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexID);
				glUniform1i(cubeTex, 3);

				glDepthMask(GL_FALSE);
				skybox.draw();
				glDepthMask(GL_TRUE);
				glUniform1i(isSky, 0);
			}
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			switch (object_index)
			{
			case 0:
				arcballRBT = (sky_type == 0) ? worldRBT : skyRBT;
				break;
			case 1:
				arcballRBT = objectRBT[0];
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
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glfwSwapBuffers(window);
			glfwPollEvents();
			pre_time = cur_time;
		}
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Clean up data structures and glsl objects	
	for (int i = 0;i < 9;i++) cubes[i].cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
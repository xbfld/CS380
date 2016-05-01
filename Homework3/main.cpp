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
glm::mat4 rubixCubeRbt = glm::mat4(1.0f);
int rubix_pos2id[9]{ 0 };	// Integer position to object ID
int rubix_id2pos[9]{ 0 };	// Object ID to integer position
int rubix_w = 3;	// # of columns
int rubix_h = 3;	// # of rows
int rubix_d = 1;	// # of layers
GLint lightLocRubix[9];
int offsetID = 100;

std::vector<int> pickedIDs;

// Arcball manipulation
Model arcBall;
glm::mat4 arcballRBT = glm::mat4(1.0f);
glm::mat4 *arcballCenterRBT = &worldRBT;
float arcBallScreenRadius = 0.25f * min(windowWidth, windowHeight); // for the initial assignment
float screenToEyeScale = 0.01f;
float arcBallScale = 0.01f;
vec3 rotation_axis{ 1.0f,0.0f,0.0f };

// Function definition
static void window_size_callback(GLFWwindow*, int, int);
static void mouse_button_callback(GLFWwindow*, int, int, int);
static void cursor_pos_callback(GLFWwindow*, double, double);
static void keyboard_callback(GLFWwindow*, int, int, int, int);
void update_fovy(void);

void selection_checking(void);
int rubix_encode(int, int, int);

// Mouse & Keyboard input related states
int press_mouse = -1; // -1 if not pressed else relate button ID
int object_mode = 1;
bool world_sky_mode = true;
int viewpoint_mode = 0;
double last_xpos = 0.0f;
double last_ypos = 0.0f;
quat last_quat;
quat base_quat;
std::vector<quat> bases = std::vector<quat>();
int magnet_state = -1;
std::vector<glm::mat4 *> target_objectRBT = std::vector<glm::mat4 *>();
std::vector<int> target_objectID = std::vector<int>();

void manipulate_targets(glm::mat4);

void update_aFrame()
{
	aFrame = transFact(*arcballCenterRBT)*linearFact(eyeRBT);
}

void update_eye()
{
	/*switch (viewpoint_mode)
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
	}*/
}

void update_arcBallScale()
{
	double _z;
	/*if ((0 == viewpoint_mode) && (0 == object_mode) && (world_sky_mode))
	{
		_z = transFact(glm::inverse(eyeRBT) * worldRBT)[3].z;
	}*/
	_z = transFact(glm::inverse(eyeRBT)* *arcballCenterRBT)[3].z;
	arcBallScale = compute_screen_eye_scale(_z, fovy, frameBufferHeight);
}

void update_arcBallRBT()
{
	arcballRBT = *arcballCenterRBT * glm::scale(vec3(arcBallScale * arcBallScreenRadius));
}

vec3 get_arcball_center()
{
	return vec3(transFact(glm::inverse(eyeRBT)* *arcballCenterRBT)[3]);
}

vec3 get_arcball_pos(double x, double y)
{
	vec2 arc_screen_center = eye_to_screen(get_arcball_center(), Projection, frameBufferWidth, frameBufferHeight);
	vec2 d_pos = vec2(x, y) - arc_screen_center;
	double _z = pow(arcBallScreenRadius, 2.0f) - glm::dot(d_pos, d_pos);
	_z = (_z < 0) ? 0 : sqrt(_z);
	return glm::normalize(vec3(d_pos, _z));
}

// Remained old version
quat get_arcball_quat(double x, double y)
{
	return quat(0, get_arcball_pos(x, y));
}
quat get_arcball_quat(vec3 pos)
{
	return quat(0, pos);
}
// Setting rotate axis
quat get_arcball_quat(vec3 pos, vec3 rot_axis)
{
	return quat(0, glm::normalize(cross(pos, rot_axis)));
}

bool is_close_to_stick(quat q, quat base, float sens)
{
	float _angle = angle(q, base);
	return (_angle < sens);
}

quat magnet(quat q, quat base, float sens)
{
	float min_angle = sens;
	float _angle;
	magnet_state = -1;
	for (size_t i = 0; i < bases.size(); i++)
	{
		_angle = angle(q, bases[i]);
		//std::cout << "_angle: " << _angle << std::endl;
		if (_angle < min_angle)
		{
			magnet_state = i;
			min_angle = _angle;
		}
	}
	if (-1 != magnet_state)
	{
		q = bases[magnet_state];
	}
	//std::cout << "magnet_state: " << magnet_state << std::endl;
	return q;
}
quat magnet(quat q, quat base)
{
	return magnet(q, base, 180.0f);
}

quat screen_to_arcball_quat(double xpos, double ypos)
{
	vec3 arcball_pos;
	arcball_pos = get_arcball_pos(xpos, frameBufferHeight - 1 - ypos);
	return get_arcball_quat(arcball_pos, rotation_axis);
}

// Rotate arcball based on input quaternian
void arcball_rotate(quat cur_quat)
{
	quat d_quat;
	mat4 manipulate = mat4(1.0f);

	// TODO: Replace literal 10.0f to variable
	cur_quat = magnet(cur_quat, base_quat, 10.0f);

	d_quat = cur_quat*inverse(last_quat);
	manipulate = glm::toMat4(d_quat);
	manipulate_targets(aFrame * manipulate * glm::inverse(aFrame));
	last_quat = cur_quat;
}
// Rotate arcball based on screen pos
// Update last cursor pos
void arcball_rotate(double xpos, double ypos)
{
	double d_x = xpos - last_xpos;
	double d_y = ypos - last_ypos;
	arcball_rotate(screen_to_arcball_quat(xpos, ypos));
	last_xpos = xpos;
	last_ypos = ypos;
}

void update_rubix_pos_id()
{
	int id;
	int pos;
	int nid; // new id
	std::vector<int> poses = std::vector<int>();
	switch (magnet_state)
	{
	case 0:
	case 1:
		break;
	case 2:
	case 3:
		for (size_t i = 0; i < target_objectID.size(); i++)
		{
			id = target_objectID[i];
			nid = target_objectID[target_objectID.size() - i - 1];
			pos = rubix_id2pos[id];
			rubix_pos2id[pos] = nid;
			poses.push_back(pos);
		}
		for (size_t i = 0; i < target_objectID.size(); i++)
		{
			pos = poses[i];
			rubix_id2pos[rubix_pos2id[pos]] = pos;
		}
		poses.clear();
		poses.shrink_to_fit();
		break;
	default:
		break;
	}
}

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

	// Update arcball radius
	arcBallScreenRadius = 0.25f * min((float)frameBufferWidth, (float)frameBufferHeight);
	update_arcBallScale();
	update_arcBallRBT();
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
			base_quat = screen_to_arcball_quat(last_xpos, last_ypos);
			last_quat = base_quat;

			// Hard coded magnet points
			quat co_base = base_quat*quat(0.0f, rotation_axis);
			bases.push_back(base_quat);
			bases.push_back(-base_quat);
			bases.push_back(co_base);
			bases.push_back(-co_base);

			update_aFrame();

			if (mods == 2)
			{
				//screen_drag_mode = true;
			}
		}
	}
	else
	{
		if (action == GLFW_RELEASE && press_mouse == button)
		{
			press_mouse = -1;

			if (GLFW_MOUSE_BUTTON_1 == button)
			{
				// Magnet. Find the Right Place
				arcball_rotate(magnet(last_quat, base_quat));
				update_rubix_pos_id();
			}
			bases.clear();
			bases.shrink_to_fit();
			update_arcBallScale();
			update_arcBallRBT();
			//screen_drag_mode = false;
		}
	}
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
	/*double d_x = xpos - last_xpos;
	double d_y = ypos - last_ypos;
	vec3 last_arcball_pos;
	vec3 cur_arcball_pos;
	quat last_quat;
	quat cur_quat;
	quat d_quat;
	mat4 manipulate = mat4(1.0f);*/
	switch (press_mouse)
	{
	case -1:
		break;
	case GLFW_MOUSE_BUTTON_1:
		// Left Mouse Button Pressed
		printf("cursor pos callbakc: Left, %f, %f\n", xpos, ypos);
		printf("bases size: %d\n", bases.size());
		arcball_rotate(xpos, ypos);
		break;
	case GLFW_MOUSE_BUTTON_2:
		// Right Mouse Button Pressed
	/*{
		printf("cursor pos callbakc: Right, %f, %f\n", xpos, ypos);
		if ((viewpoint_mode != object_mode)
			|| ((0 == viewpoint_mode) && (0 == object_mode) && world_sky_mode))
		{
			manipulate = glm::translate(vec3(d_x, -d_y, 0.0f) * arcBallScale);
		}
		else
		{
			manipulate = glm::translate(vec3(d_x, -d_y, 0.0f) / arcBallScreenRadius);
		}
		if ((0 == viewpoint_mode) && (0 == object_mode) && world_sky_mode)
		{
			manipulate = inverse(manipulate);
		}
		manipulate_targets(aFrame * manipulate * glm::inverse(aFrame));

		last_xpos = xpos;
		last_ypos = ypos;
	}*/
		break;
	case GLFW_MOUSE_BUTTON_3:
		// Middle Mouse Button Pressed
	/*{
		printf("cursor pos callbakc: Middle, %f, %f\n", xpos, ypos);
		if ((viewpoint_mode != object_mode)
			|| ((0 == viewpoint_mode) && (0 == object_mode) && world_sky_mode))
		{
			manipulate = glm::translate(vec3(0.0f, 0.0f, d_y) * arcBallScale);
		}
		else
		{
			manipulate = glm::translate(vec3(0.0f, 0.0f, d_y) / arcBallScreenRadius);
		}
		if ((0 == viewpoint_mode) && (0 == object_mode) && world_sky_mode)
		{
			manipulate = inverse(manipulate);
		}
		manipulate_targets(aFrame * manipulate * glm::inverse(aFrame));
		last_xpos = xpos;
		last_ypos = ypos;
	}*/
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

// Helper function: Encode integer position from w, h, d position
int rubix_encode(int w, int h, int d)
{
	return w + rubix_w * (h + rubix_h * d);
}
int rubix_encode(ivec3 p)
{
	return rubix_encode(p.x, p.y, p.z);
}
// Helper function: Decode integer position to w, h, d position
glm::ivec3 rubix_decode(int id)
{
	glm::ivec3 result(-1);
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
	// Alternative value
	rubixCubeRbt = glm::translate(0.0f, -0.5f, -2.0f) *
		glm::eulerAngleYXZ(radians(0.0f), radians(-45.0f), radians(-30.0f));

	for (size_t d = 0; d < rubix_d; d++)
	{
		for (size_t h = 0; h < rubix_h; h++)
		{
			for (size_t w = 0; w < rubix_w; w++)
			{
				r_index = rubix_encode(w, h, d);
				r_model = &(rubixModel[r_index]);
				g_rubixRbt[r_index] = rubixCubeRbt * glm::translate(glm::mat4(1.0f), pos_offset + glm::vec3(1.0f * w, 1.0f * h, 1.0f * d));
				rubix_pos2id[r_index] = r_index;
				rubix_id2pos[r_index] = r_index;
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
		// p contains w, h, d position
		p = rubix_decode(rubix_id2pos[pickedIDs[i] - offsetID]);
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
		int id;
		std::cout << "common: "
			<< common.x << " "
			<< common.y << " "
			<< common.z << std::endl;
		pickedIDs.clear();

		// Setting aFrame
		target_objectID.clear();
		target_objectRBT.clear();
		// same column
		if ((-1 != common.x) && (-1 == common.y))
		{
			for (size_t h = 0; h < rubix_h; h++)
			{
				common.y = h;
				id = rubix_pos2id[rubix_encode(common)];
				target_objectID.push_back(id);
				target_objectRBT.push_back(&g_rubixRbt[id]);
			}
			common.y = rubix_h / 2;
			arcballCenterRBT = target_objectRBT[1];
			rotation_axis = vec3(rubixCubeRbt * vec4(1.0f, 0.0f, 0.0f, 0.0f));
		}
		// same row
		else if ((-1 == common.x) && (-1 != common.y))
		{
			for (size_t w = 0; w < rubix_w; w++)
			{
				common.x = w;
				id = rubix_pos2id[rubix_encode(common)];
				target_objectID.push_back(id);
				target_objectRBT.push_back(&g_rubixRbt[id]);
			}
			common.x = rubix_w / 2;
			arcballCenterRBT = target_objectRBT[1];
			rotation_axis = vec3(rubixCubeRbt * vec4(0.0f, 1.0f, 0.0f, 0.0f));
		}
		update_arcBallScale();
		update_arcBallRBT();
	}
}

// Helper function: Manipulates all targets
void manipulate_targets(glm::mat4 m)
{
	for (size_t i = 0; i < target_objectRBT.size(); i++)
	{
		*target_objectRBT[i] = m* *target_objectRBT[i];
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
		rubix_draw_picking();

		// second pass: your drawing
		// unbinding framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor((GLclampf)(128. / 255.), (GLclampf)(200. / 255.), (GLclampf)(255. / 255.), (GLclampf)0.);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	arcBall.cleanup();
	rubix_cleanup();

	pickedIDs.clear();

	// Cleanup textures
	delete_picking_resources();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

// Shader library
#include <common/shader.hpp>

#define BUFFER_OFFSET( offset ) ((GLvoid*) (offset))

GLFWwindow* window;
int windowWidth, windowHeight;

GLuint programID;
GLuint VAID;
GLuint VBID;
GLuint PillarID;
GLuint WatcherID;
GLuint colorbuffer;

std::vector<glm::vec3> initial_triangle;
std::vector<glm::vec3> g_vertex_buffer_data;
//std::vector<glm::vec3> g_color_buffer_data;
std::vector<glm::vec3> pillar_vertex_buffer_data;
std::vector<glm::vec3> back_snowflakes_position;
std::vector<glm::vec3> watcher_vertex_buffer_data;

std::vector<float> back_snowflakes_phase;

glm::mat4 Projection;
glm::mat4 View = glm::lookAt(
	glm::vec3(0, 0, 2),
	glm::vec3(0, 0, 0),
	glm::vec3(0, 1, 0));
glm::mat4 View_light;

// fovy
const float fovy = 45.0f;

// snowflakes iteration steps
const int snowflake_iter = 5;

// background snowflakes
const int back_snowflake_number = 100;
const float xrange[2]{ -1.0f,1.0f };
const float yrange[2]{ -1.0f,1.0f };
const float snow_lifetime = 2.0f;
const float speed = 1.0f;
glm::mat4 R_frame = glm::rotate(30.0f, glm::vec3(0, 0, 1));

// Using on objects positions
float back_size = 0.1f;
float sub_size = 0.05f;
float main_size = 0.3f;

float degree = 0.0f;
double last_time = 0.0;
glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

// Pillar
float top = 20.0f;
float bottom = -0.01f;
int pillar_number = 20;

void wall(glm::vec2 a, glm::vec2 b, std::vector<vec3> *buffer_data)
{
	(*buffer_data).push_back(glm::vec3(a, bottom));
	(*buffer_data).push_back(glm::vec3(b, bottom));
	(*buffer_data).push_back(glm::vec3(a, top));
	(*buffer_data).push_back(glm::vec3(a, top));
	(*buffer_data).push_back(glm::vec3(b, bottom));
	(*buffer_data).push_back(glm::vec3(b, top));
}

// Random float between min to max
float random_range(float min, float max)
{
	return (max - min) * ((((float)rand()) / (float)RAND_MAX)) + min;
}

// Random float between -1.0f to 1.0f
float random_normal()
{
	return random_range(-1.0f, 1.0f);
}

vec2 random_normal_vec2()
{
	return vec2(random_normal(), random_normal());
}

vec3 random_start_point()
{
	vec2 random_center = random_normal_vec2();
	return vec3(random_center, -0.01f) + vec3(0.0f, 0.0f + speed * snow_lifetime / 2.0f, 0.0f);
}

// DONE: Implement koch snowflake
void koch_line(glm::vec3 a, glm::vec3 b, int iter)
{
	glm::vec3 b2a = b - a;
	glm::vec3 rot = glm::rotateZ(b2a, 30.0f);

	std::vector<glm::vec3> points = std::vector<glm::vec3>();
	points.push_back(a + b2a / 3.0f);
	points.push_back(a + rot*sqrt(3.0f) / 3.0f);
	points.push_back(a + b2a *2.0f / 3.0f);

	for (size_t i = 0; i < points.size(); i++)
	{
		g_vertex_buffer_data.push_back(points[i]);
	}

	if (iter < snowflake_iter)
	{
		koch_line(a, points[0], iter + 1);
		koch_line(points[0], points[1], iter + 1);
		koch_line(points[1], points[2], iter + 1);
		koch_line(points[2], b, iter + 1);
	}

	points.clear();

}

// DONE: Initialize model
void init_model(void)
{
	initial_triangle = std::vector<glm::vec3>();
	initial_triangle.push_back(glm::vec3(-0.5f, 0.0f, 0.0f));
	initial_triangle.push_back(glm::vec3(0.0f, sqrt(0.75), 0.0f));
	initial_triangle.push_back(glm::vec3(0.5f, 0.0f, 0.0f));
	glm::vec3 center = glm::vec3(0.0f, sqrt(0.75) / 3, 0.0f);

	g_vertex_buffer_data = std::vector<glm::vec3>();
	for (size_t i = 0; i < initial_triangle.size(); i++)
	{
		g_vertex_buffer_data.push_back(initial_triangle[i] - center);
	}
	koch_line(g_vertex_buffer_data[0], g_vertex_buffer_data[1], 0);
	koch_line(g_vertex_buffer_data[1], g_vertex_buffer_data[2], 0);
	koch_line(g_vertex_buffer_data[2], g_vertex_buffer_data[0], 0);

	srand(time(NULL)); // seeding a random function
	vec2 random_center;
	float random_radian;
	for (size_t i = 0; i < pillar_number; i++)
	{
		random_center = random_normal_vec2();
		random_radian = random_range(0.0f, 120.0f);
		vec2 a = 0.1f * glm::rotate(vec2(1.0f, 0.0f), random_radian) + random_center;
		vec2 b = 0.1f * glm::rotate(vec2(1.0f, 0.0f), random_radian + 120.0f) + random_center;
		vec2 c = 0.1f * glm::rotate(vec2(1.0f, 0.0f), random_radian + 240.0f) + random_center;
		pillar_vertex_buffer_data.push_back(vec3(a, 0.10f));
		pillar_vertex_buffer_data.push_back(vec3(b, 0.10f));
		pillar_vertex_buffer_data.push_back(vec3(c, 0.10f));
		wall(a, b, &pillar_vertex_buffer_data);
		wall(b, c, &pillar_vertex_buffer_data);
		wall(c, a, &pillar_vertex_buffer_data);
	}

	// Watcher Model
	watcher_vertex_buffer_data.push_back(vec3(1.0f, 0.0f, 0.0f));
	watcher_vertex_buffer_data.push_back(vec3(3.0f, 0.0f, 0.0f));
	watcher_vertex_buffer_data.push_back(vec3(0.0f, sqrt(3.0f), 0.0f));
	watcher_vertex_buffer_data.push_back(vec3(1.0f, 0.0f, 0.0f));
	watcher_vertex_buffer_data.push_back(vec3(0.0f, -sqrt(3.0f), 0.0f));
	watcher_vertex_buffer_data.push_back(vec3(3.0f, 0.0f, 0.0f));

	watcher_vertex_buffer_data.push_back(vec3(-1.0f, 0.0f, 0.0f));
	watcher_vertex_buffer_data.push_back(vec3(0.0f, sqrt(3.0f), 0.0f));
	watcher_vertex_buffer_data.push_back(vec3(-3.0f, 0.0f, 0.0f));
	watcher_vertex_buffer_data.push_back(vec3(-1.0f, 0.0f, 0.0f));
	watcher_vertex_buffer_data.push_back(vec3(-3.0f, 0.0f, 0.0f));
	watcher_vertex_buffer_data.push_back(vec3(0.0f, -sqrt(3.0f), 0.0f));

	watcher_vertex_buffer_data.push_back(vec3(0.0f, sqrt(3.0f), 0.0f));
	watcher_vertex_buffer_data.push_back(vec3(-0.5f, sqrt(3.0f) / 2.0f, 0.0f));
	watcher_vertex_buffer_data.push_back(vec3(0.5f, sqrt(3.0f) / 2.0f, 0.0f));

	watcher_vertex_buffer_data.push_back(vec3(0.0f, -sqrt(3.0f), 0.0f));
	watcher_vertex_buffer_data.push_back(vec3(0.5f, -sqrt(3.0f) / 2.0f, 0.0f));
	watcher_vertex_buffer_data.push_back(vec3(-0.5f, -sqrt(3.0f) / 2.0f, 0.0f));

	// Generates Vertex Array Objects in the GPU¡¯s memory and passes back their identifiers
	// Create a vertex array object that represents vertex attributes stored in a vertex buffer object.
	glGenVertexArrays(1, &VAID);
	glBindVertexArray(VAID);

	// Create and initialize a buffer object, Generates our buffers in the GPU¡¯s memory
	glGenBuffers(1, &VBID);
	glBindBuffer(GL_ARRAY_BUFFER, VBID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*g_vertex_buffer_data.size(), &g_vertex_buffer_data[0], GL_STATIC_DRAW);

	glGenBuffers(1, &PillarID);
	glBindBuffer(GL_ARRAY_BUFFER, PillarID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*pillar_vertex_buffer_data.size(), &pillar_vertex_buffer_data[0], GL_STATIC_DRAW);

	glGenBuffers(1, &WatcherID);
	glBindBuffer(GL_ARRAY_BUFFER, WatcherID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*watcher_vertex_buffer_data.size(), &watcher_vertex_buffer_data[0], GL_STATIC_DRAW);

	// init time
	last_time = glfwGetTime();

	// init background snowflake

	for (size_t i = 0; i < back_snowflake_number; i++)
	{
		back_snowflakes_position.push_back(random_start_point());
		back_snowflakes_phase.push_back((float)i / back_snowflake_number);
	}

}

void draw_object(glm::mat4 MVP, float color[], std::vector<vec3> *buffer_data)
{
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ColorID = glGetUniformLocation(programID, "vcolor");

	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform4fv(ColorID, 1, color);
	glDrawArrays(GL_TRIANGLES, 0, (*buffer_data).size());
}

void draw_snowflake(glm::mat4 MVP, float color[])
{
	draw_object(MVP, color, &g_vertex_buffer_data);
}

void draw_pillar(glm::mat4 MVP, float color[])
{
	draw_object(MVP, color, &pillar_vertex_buffer_data);
}

void draw_watcher(glm::mat4 MVP, float color[])
{
	draw_object(MVP, color, &watcher_vertex_buffer_data);
}

// DONE: Draw model
void draw_model()
{
	double current_time = glfwGetTime();
	double delta_time = current_time - last_time;
	last_time = current_time;

	degree += 60.0f * (float)delta_time;

	glm::mat4 T;
	glm::mat4 R;
	glm::mat4 S;
	glm::mat4 MVP;

	// Draw background snowflake
	glUseProgram(programID);
	glBindVertexArray(VAID);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	float white_color[4]{ 1.0f, 1.0f, 1.0f, 1.0f };

	// update all back snowflakes
	for (size_t i = 0; i < back_snowflake_number; i++)
	{
		float *phase = &back_snowflakes_phase[i];
		vec3 *pos = &back_snowflakes_position[i];
		(*phase) += delta_time / snow_lifetime;
		if (1.0f < (*phase))
		{
			(*phase) -= 1.0f;
			(*pos) = random_start_point();
		}
		T = glm::translate((*pos) + vec3(0.0f, -speed, 0.0f) * (*phase));
		R = glm::rotate(degree*1.3f, glm::vec3(0, 0, 1));
		S = glm::scale(glm::vec3(back_size)
			* (*phase) * (1.0f - (*phase))*4.0f); // back_size is Max size
		MVP = Projection * View * R_frame * T * R * S;
		draw_snowflake(MVP, white_color);
	}

	// Draw Watcher
	glDisableVertexAttribArray(0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, WatcherID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	float sub_color[4]{ 0.3f, 1.0f, 0.7f, 1.0f };
	float radius = 0.8f;
	float radian = radians(degree);
	vec2 xy = vec2(cos(radian), sin(radian*1.2)) * radius;

	T = glm::translate(
		position
		+ vec3(xy, 0.01f)
		);
	S = glm::scale(glm::vec3(sub_size));
	MVP = Projection * View * T * S;

	draw_watcher(MVP, sub_color);

	// Draw revolution snowflake
	glUseProgram(programID);
	glBindVertexArray(VAID);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	T = glm::translate(
		position
		+ vec3(vec2(cos(radian*2.0f), -sin(radian*2.0f)) * radius * 0.5f, 0.01f)
		);
	R = glm::rotate(degree, glm::vec3(0, 0, 1));
	S = glm::scale(glm::vec3(sub_size));
	MVP = Projection * View * T * R * S;
	float revo_color[4]{ 0.0f, 1.0f, 1.0f, 1.0f };
	draw_snowflake(MVP, revo_color);

	// Draw main snowflake
	glUseProgram(programID);
	glBindVertexArray(VAID);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	T = glm::translate(position);
	R = glm::rotate(degree, glm::vec3(0, 0, 1));
	S = glm::scale(glm::vec3(main_size));
	MVP = Projection * View * T * R * S;
	float main_color[4]{ 0.5f, 0.7f, 0.8f, 1.0f };

	draw_snowflake(MVP, main_color);

	// Draw Pillar
	glDisableVertexAttribArray(0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, PillarID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	T = glm::translate(position);
	R = glm::mat4();
	S = glm::scale(glm::vec3(1.0f));
	// View Point over Watcher
	View_light = glm::lookAt(
		glm::vec3(xy, 2),
		glm::vec3(xy, 0),
		glm::vec3(0, 1, 0));

	MVP = Projection * View_light * T * R * S;

	vec2 xycenter = vec2(Projection * View_light * vec4(position, 1.0f));
	// Translate the clip space
	MVP = glm::translate(vec3(-xycenter / 2.0f, 0.0f)) * MVP;

	float black[4]{ 0.0f, 0.0f, 0.0f, 1.0f };

	draw_pillar(MVP, black);
	// End: Draw Pillar

	// Draw back Watcher
	glDisableVertexAttribArray(0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, WatcherID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	T = glm::translate(position);
	S = glm::scale(glm::vec3(main_size));
	MVP = Projection * View * T * S;

	float gray[4]{ 0.5f, 0.5f, 0.5f, 0.5f };

	draw_watcher(MVP, gray);


	glDisableVertexAttribArray(0);
	//glDisableVertexAttribArray(1);
}

// Resize Window
void window_size_callback(GLFWwindow* window, int width, int height)
{

	glViewport(0, 0, width, height);

	float ratio = (float)width / (float)height;
	Projection = glm::perspective(fovy, ratio, 0.1f, 100.0f);
}


int main(int argc, char* argv[])
{
	// Step 1: Initialization
	if (!glfwInit())
	{
		return -1;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// DONE: GLFW create window and context
	window = glfwCreateWindow(1024, 768, "Lab 1: 20130156_김준", NULL, NULL);
	if (window == NULL)
	{
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// END

	// DONE: Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		return -1;
	}
	// END

	Projection = glm::perspective(fovy, 4.0f / 3.0f, 0.1f, 100.0f);
	View = glm::lookAt(
		glm::vec3(0, 0, 2),
		glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0));
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;

	// DONE: Initialize OpenGL and GLSL
	glClearColor(0.10f, 0.10f, 0.10f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	glViewport(0, 0, width, height);
	programID = LoadShaders("VertexShader.glsl", "FragmentShader.glsl");
	//GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	// END

	glfwSetWindowSizeCallback(window, window_size_callback);

	init_model();

	// Step 2: Main event loop
	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		draw_model();

		glfwSwapBuffers(window);
		glfwPollEvents();
	} while (!glfwWindowShouldClose(window));

	// Step 3: Termination
	initial_triangle.clear();
	g_vertex_buffer_data.clear();
	pillar_vertex_buffer_data.clear();
	back_snowflakes_position.clear();
	back_snowflakes_phase.clear();

	glDeleteBuffers(1, &PillarID);
	glDeleteBuffers(1, &VBID);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VAID);

	glfwTerminate();

	return 0;
}

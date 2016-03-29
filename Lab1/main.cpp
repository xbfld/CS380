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
GLuint colorbuffer;

std::vector<glm::vec3> initial_triangle;
std::vector<glm::vec3> g_vertex_buffer_data;
//std::vector<glm::vec3> g_color_buffer_data;
std::vector<glm::vec3> pillar_vertex_buffer_data;
std::vector<glm::vec3> back_snowflakes_position;

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
const float lifespan = 2.0f;
const float speed = 1.0f;
float snowflakes_phase = 0.0f;
glm::mat4 R_frame = glm::rotate(30.0f, glm::vec3(0, 0, 1));

// Using on objects positions
float back_size = 0.1f;
float sub_size = 0.2f;
float main_size = 0.3f;

float degree = 0.0f;
double last_time = 0.0;
glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

// Pillar
float top = 20.0f;
float bottom = -0.01f;

void wall(glm::vec2 a, glm::vec2 b, std::vector<vec3> *buffer_data)
{
	(*buffer_data).push_back(glm::vec3(a, bottom));
	(*buffer_data).push_back(glm::vec3(b, bottom));
	(*buffer_data).push_back(glm::vec3(a, top));
	(*buffer_data).push_back(glm::vec3(a, top));
	(*buffer_data).push_back(glm::vec3(b, bottom));
	(*buffer_data).push_back(glm::vec3(b, top));
}

vec3 random_start_point()
{
	// take random float between (xrange[0], xrange[1]) and (yrange[0], yrange[1])
	float x = (xrange[1] - xrange[0]) * ((((float)rand()) / (float)RAND_MAX)) + xrange[0];
	float y = (yrange[1] - yrange[0]) * ((((float)rand()) / (float)RAND_MAX)) + yrange[0];
	//printf("%f, %f", x, y);
	return vec3(x, y + speed * lifespan / 2.0f, 0.0f);
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
	if (iter == 3)
	{
		wall(glm::vec2(a), glm::vec2(points[1]), &pillar_vertex_buffer_data);
		wall(glm::vec2(points[1]), glm::vec2(b), &pillar_vertex_buffer_data);
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


	/*g_color_buffer_data.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
	g_color_buffer_data.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
	g_color_buffer_data.push_back(glm::vec3(0.0f, 0.0f, 1.0f));

	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*g_color_buffer_data.size(), &g_color_buffer_data[0], GL_STATIC_DRAW);*/

	last_time = glfwGetTime();

	// init background snowflake

	for (size_t i = 0; i < back_snowflake_number; i++)
	{
		back_snowflakes_position.push_back(random_start_point());
	}

}

void draw_snowflake(glm::mat4 MVP, float color[])
{
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ColorID = glGetUniformLocation(programID, "vcolor");

	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform4fv(ColorID, 1, color);
	glDrawArrays(GL_TRIANGLES, 0, g_vertex_buffer_data.size());
}

// DONE: Draw model
void draw_model()
{
	glUseProgram(programID);
	glBindVertexArray(VAID);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	//glEnableVertexAttribArray(1);
	//glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	//glVertexAttribPointer(
	//	1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
	//	3,                                // size
	//	GL_FLOAT,                         // type
	//	GL_FALSE,                         // normalized?
	//	0,                                // stride
	//	(void*)0                          // array buffer offset
	//	);

	double current_time = glfwGetTime();
	double delta_time = current_time - last_time;
	last_time = current_time;

	degree += 60.0f * (float)delta_time;

	glm::mat4 T;
	glm::mat4 R;
	glm::mat4 S;
	glm::mat4 MVP;

	// Draw background snowflake
	float white_color[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
	snowflakes_phase += delta_time;
	if (lifespan < snowflakes_phase)
	{
		snowflakes_phase -= lifespan;
		for (size_t i = 0; i < back_snowflake_number; i++)
		{
			back_snowflakes_position[i] = random_start_point();
		}
	}
	for (size_t i = 0; i < back_snowflake_number; i++)
	{
		T = glm::translate(back_snowflakes_position[i] + vec3(0.0f, -speed, 0.0f)*snowflakes_phase);
		R = glm::rotate(degree*1.3f, glm::vec3(0, 0, 1));
		S = glm::scale(glm::vec3(back_size)
			*(snowflakes_phase)*(lifespan - snowflakes_phase)*4.0f / lifespan / lifespan); // back_size is Max size
		MVP = Projection * View * R_frame * T * R * S;
		draw_snowflake(MVP, white_color);
	}


	// Draw sub snowflake
	float sub_color[4]{ 0.0f, 1.0f, 1.0f, 1.0f };
	float radius = 0.8f;
	float radian = radians(degree);
	vec2 xy = vec2(cos(radian), sin(radian*1.2)) * radius;

	T = glm::translate(
		position
		+ vec3(xy, 0.01f)
		);
	S = glm::scale(glm::vec3(sub_size));
	MVP = Projection * View * T * S;

	draw_snowflake(MVP, sub_color);

	// Draw revolution snowflake
	T = glm::translate(
		position
		+ vec3(vec2(cos(radian*2.0f), -sin(radian*2.0f)) * radius * 0.5f, 0.01f)
		);
	R = glm::rotate(degree, glm::vec3(0, 0, 1));
	S = glm::scale(glm::vec3(sub_size));
	MVP = Projection * View * T * R * S;
	float revo_color[4]{ 1.0f, 1.0f, 0.0f, 1.0f };
	draw_snowflake(MVP, revo_color);

	// Draw main snowflake
	T = glm::translate(position);
	R = glm::rotate(degree, glm::vec3(0, 0, 1));
	S = glm::scale(glm::vec3(main_size));
	MVP = Projection * View * T * R * S;
	//vec4 pp = Projection*View*vec4(1.0f, 1.0f, 1.0f, 1.0f);
	//printf("%f,%f,%f,%f\n", pp.x, pp.y, pp.z, pp.w);
	//printf("%f,%f\n", pp.x / pp.w, pp.y / pp.w);
	float main_color[4]{ 1.0f, 0.0f, 1.0f, 1.0f };

	draw_snowflake(MVP, main_color);

	// Draw Pillar

	glDisableVertexAttribArray(0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, PillarID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	T = glm::translate(position);
	R = glm::rotate(degree, glm::vec3(0, 0, 1));
	S = glm::scale(glm::vec3(main_size));
	// View Point
	View_light = glm::lookAt(
		glm::vec3(xy, 2),
		glm::vec3(xy, 0),
		glm::vec3(0, 1, 0));

	MVP = Projection * View_light * T * R * S;

	vec2 xycenter = vec2(Projection * View_light * vec4(position, 1.0f));
	MVP = glm::translate(vec3(-xycenter / 2.0f, 0.0f)) * MVP;

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ColorID = glGetUniformLocation(programID, "vcolor");
	float color[4]{ 0.0f, 0.0f, 0.0f, 1.0f };
	//float color[4]{ 0.5f, 0.5f, 0.5f, 0.5f };

	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform4fv(ColorID, 1, color);
	glDrawArrays(GL_TRIANGLES, 0, pillar_vertex_buffer_data.size());
	// End: Draw Pillar

	glDisableVertexAttribArray(0);
	//glDisableVertexAttribArray(1);
}

// Resize Window
void window_size_callback(GLFWwindow* window, int width, int height)
{

	glViewport(0, 0, width, height);

	float ratio = (float)width / (float)height;
	// make min(fovx,fovy) be 45.0f
	// and, fovy = fovx * height / width
	Projection = glm::perspective(fovy, ratio, 0.1f, 100.0f);
	// glfwSetWindowSize(window, width, height);
	printf("%f w: %d h: %d\n", glfwGetTime(), width, height);
	printf("%f\n", 45.0f / (min(1.0f, ratio)));
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

	glDeleteBuffers(1, &PillarID);
	glDeleteBuffers(1, &VBID);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VAID);

	glfwTerminate();

	return 0;
}

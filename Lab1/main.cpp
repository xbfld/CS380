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

std::vector<glm::vec3> initial_triangle;
std::vector<glm::vec3> g_vertex_buffer_data;

glm::mat4 Projection;
glm::mat4 View;
float degree = 0.0f;
double last_time = 0.0;
glm::vec3 speed = glm::vec3(1.0f, 0.6f, 0.0f);
glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

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

	if (iter > 0)
	{
		koch_line(a, points[0], iter - 1);
		koch_line(points[0], points[1], iter - 1);
		koch_line(points[1], points[2], iter - 1);
		koch_line(points[2], b, iter - 1);
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
	koch_line(g_vertex_buffer_data[0], g_vertex_buffer_data[1], 5);
	koch_line(g_vertex_buffer_data[1], g_vertex_buffer_data[2], 5);
	koch_line(g_vertex_buffer_data[2], g_vertex_buffer_data[0], 5);

	// Generates Vertex Array Objects in the GPU¡¯s memory and passes back their identifiers
	// Create a vertex array object that represents vertex attributes stored in a vertex buffer object.
	glGenVertexArrays(1, &VAID);
	glBindVertexArray(VAID);

	// Create and initialize a buffer object, Generates our buffers in the GPU¡¯s memory
	glGenBuffers(1, &VBID);
	glBindBuffer(GL_ARRAY_BUFFER, VBID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*g_vertex_buffer_data.size(), &g_vertex_buffer_data[0], GL_STATIC_DRAW);

	last_time = glfwGetTime();

}

// DONE: Draw model
void draw_model()
{
	glUseProgram(programID);
	glBindVertexArray(VAID);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), BUFFER_OFFSET(0));

	double current_time = glfwGetTime();
	double delta_time = current_time - last_time;
	last_time = current_time;

	degree += 60.0f * (float)delta_time;
	position += speed * (float)delta_time;
	if (position.x < -1.0f || position.x > 1.0f)
	{
		speed = glm::reflect(speed, glm::vec3(1.0f, 0.0f, 0.0f));
	}
	if (position.y < -0.8f || position.y > 0.8f)
	{
		speed = glm::reflect(speed, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::mat4 T = glm::translate(position);
	glm::mat4 R = glm::rotate(sin(degree / 180.0f*3.14f)*180.0f, glm::vec3(0, 0, 1));
	glm::mat4 MVP = Projection * View * T * R;
	glm::mat4 MVP2 = Projection * View;

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

	glDrawArrays(GL_TRIANGLES, 0, g_vertex_buffer_data.size());

	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);

	glDrawArrays(GL_TRIANGLES, 0, g_vertex_buffer_data.size());

	glDisableVertexAttribArray(0);
}

// Resize Window
void window_size_callback(GLFWwindow* window, int width, int height)
{

	glViewport(0, 0, width, height);

	float ratio = (float)width / (float)height;
	// make min(fovx,fovy) be 45.0f
	// and, fovy = fovx * height / width
	Projection = glm::perspective(45.0f / (min(1.0f, ratio)), ratio, 0.1f, 100.0f);
	// glfwSetWindowSize(window, width, height);
	printf("%f w: %d h: %d\n", glfwGetTime(), width, height);
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

	Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	View = glm::lookAt(
		glm::vec3(0, 0, 2),
		glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0));
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;

	// DONE: Initialize OpenGL and GLSL
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	glViewport(0, 0, width, height);
	programID = LoadShaders("VertexShader.glsl", "FragmentShader.glsl");
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
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

	glDeleteBuffers(1, &VBID);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VAID);

	glfwTerminate();

	return 0;
}

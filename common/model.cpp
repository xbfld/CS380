#include <iostream>
#include <vector>

#include "model.hpp"
#include "shader.hpp"

using namespace std;

Model::Model()
{
	// Initialize model information
	vertices = std::vector<glm::vec3>();
	indices = std::vector<unsigned int>();
	normals = std::vector<glm::vec3>();
	colors = std::vector<glm::vec3>();
}

void Model::add_vertex(float x, float y, float z)
{
	vertices.push_back(glm::vec3(x, y, z));
}

void Model::add_vertex(glm::vec3 vertex)
{
	vertices.push_back(vertex);
}

void Model::add_normal(float x, float y, float z)
{
	normals.push_back(glm::vec3(x, y, z));
}

void Model::add_normal(glm::vec3 normal)
{
	normals.push_back(normal);
}

void Model::add_color(float r, float g, float b)
{
	colors.push_back(glm::vec3(r, g, b));
}

void Model::add_color(glm::vec3 color)
{
	colors.push_back(color);
}

void Model::add_index(unsigned int idx)
{
	indices.push_back(idx);
}

void Model::set_projection(glm::mat4* projection)
{
	this->Projection = projection;
}

void Model::set_eye(glm::mat4* eye)
{
	this->Eye = eye;
}

void Model::set_model(glm::mat4* model)
{
	this->ModelTransform = model;
}

void Model::initialize(DRAW_TYPE type, const char * vertexShader_path, const char * fragmentShader_path)
{
	this->GLSLProgramID = LoadShaders(vertexShader_path, fragmentShader_path);
	this->type = type;

	glGenVertexArrays(1, &this->VertexArrayID);
	glBindVertexArray(this->VertexArrayID);

	glGenBuffers(1, &this->VertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, this->VertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*this->vertices.size(), &this->vertices[0], GL_STATIC_DRAW);

	if (this->type == DRAW_TYPE::INDEX)
	{
		glGenBuffers(1, &this->IndexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->IndexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*this->indices.size(), &this->indices[0], GL_STATIC_DRAW);
	}

	glGenBuffers(1, &this->NormalBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, this->NormalBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*this->normals.size(), &this->normals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &this->ColorBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, this->ColorBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*this->colors.size(), &this->colors[0], GL_STATIC_DRAW);
}

void Model::draw()
{
	glUseProgram(this->GLSLProgramID);
	GLint ProjectionID = glGetUniformLocation(this->GLSLProgramID, "Projection");
	GLint EyeID = glGetUniformLocation(this->GLSLProgramID, "Eye");
	GLint ModelTransformID = glGetUniformLocation(this->GLSLProgramID, "ModelTransform");

	glUniformMatrix4fv(ProjectionID, 1, GL_FALSE, &(*(this->Projection))[0][0]);
	glUniformMatrix4fv(EyeID, 1, GL_FALSE, &(*(this->Eye))[0][0]);
	glUniformMatrix4fv(ModelTransformID, 1, GL_FALSE, &(*(this->ModelTransform))[0][0]);

	glBindVertexArray(this->VertexArrayID);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->VertexBufferID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), ((GLvoid*)(0)));

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->NormalBufferID);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), ((GLvoid*)(0)));

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, this->ColorBufferID);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), ((GLvoid*)(0)));

	if (this->type == DRAW_TYPE::ARRAY)
	{
		glDrawArrays(GL_TRIANGLES, 0, (GLsizei) this->vertices.size());
	}
	else {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->IndexBufferID);
		glDrawElements(GL_TRIANGLES, (GLsizei) this->indices.size(), GL_UNSIGNED_INT, ((GLvoid *)0));
	}
}

void Model::cleanup()
{
	// Clean up data structures
	this->vertices.clear();
	this->vertices.shrink_to_fit();

	this->indices.clear();
	this->indices.shrink_to_fit();

	this->normals.clear();
	this->normals.shrink_to_fit();

	this->colors.clear();
	this->colors.shrink_to_fit();

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &this->VertexBufferID);
	glDeleteBuffers(1, &this->NormalBufferID);
	glDeleteBuffers(1, &this->ColorBufferID);
	if (this->type == DRAW_TYPE::INDEX) glDeleteBuffers(1, &this->IndexBufferID);
	glDeleteProgram(this->GLSLProgramID);
	glDeleteVertexArrays(1, &this->VertexArrayID);
}

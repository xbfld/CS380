#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
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

glm::mat4* Model::get_model()
{
	return this->ModelTransform;
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

	//TODO: generate/bind buffer for colors and store the color values


	//TODO: generate/bind buffer for normals and store the normal values

}

void Model::initialize_picking(const char* picking_vertex_shader, const char* picking_fragment_shader)
{
	this->PickingProgramID = LoadShaders(picking_vertex_shader, picking_fragment_shader);
}

bool Model::loadOBJ(const char * path,	glm::vec3 color){
	cout << "Loading OBJ file " << path << "..." << endl;
	vector<GLushort> elements;
	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	ifstream in(path, ios::in);
	if (!in)
	{
		cerr << "Cannot open " << path << endl; exit(1);
	}

	string line;
	while (getline(in, line))
	{
		if (line.substr(0, 2) == "v ")
		{
			istringstream s(line.substr(2));
			glm::vec3 v; s >> v.x; s >> v.y; s >> v.z; //v.w = 1.0f;
			vertices.push_back(v);
			
		}
		else if (line.substr(0, 2) == "f ")
		{
			istringstream s(line.substr(2));
			GLushort a, b, c;
			s >> a; s >> b; s >> c;
			a--; b--; c--;
			elements.push_back(a); elements.push_back(b); elements.push_back(c);
		}		
		else if (line[0] == '#')
		{
			/* ignoring this line */
		}
		else
		{
			/* ignoring this line */
		}
	}

	normals.resize(vertices.size(), glm::vec3(0.0, 0.0, 0.0));
		
	for (int i = 0; i < elements.size(); i += 3)
	{
		GLushort ia = elements[i];
		GLushort ib = elements[i + 1];
		GLushort ic = elements[i + 2];
		glm::vec3 normal = glm::normalize(glm::cross(
			glm::vec3(vertices[ib]) - glm::vec3(vertices[ia]),
			glm::vec3(vertices[ic]) - glm::vec3(vertices[ia])));				
		normals[ia] += normal; normals[ib] += normal; normals[ic] += normal;		

		add_vertex(vertices[ia]);
		add_vertex(vertices[ib]);
		add_vertex(vertices[ic]);
		
		add_color(color);
		add_color(color);
		add_color(color);
	}
	for (int i = 0; i < normals.size(); i++){
		normals[i] = glm::normalize(normals[i]);
	}
	for (int i = 0; i < elements.size(); i += 3)
	{
		GLushort ia = elements[i];
		GLushort ib = elements[i + 1];
		GLushort ic = elements[i + 2];

		add_normal(normals[ia]);
		add_normal(normals[ib]);
		add_normal(normals[ic]);
	}

	return true;
}

void Model::draw()
{
	glUseProgram(this->GLSLProgramID);
	GLuint ProjectionID = glGetUniformLocation(this->GLSLProgramID, "Projection");
	GLuint EyeID = glGetUniformLocation(this->GLSLProgramID, "Eye");
	GLuint ModelTransformID = glGetUniformLocation(this->GLSLProgramID, "ModelTransform");

	glUniformMatrix4fv(ProjectionID, 1, GL_FALSE, &(*(this->Projection))[0][0]);
	glUniformMatrix4fv(EyeID, 1, GL_FALSE, &(*(this->Eye))[0][0]);
	glUniformMatrix4fv(ModelTransformID, 1, GL_FALSE, &(*(this->ModelTransform))[0][0]);

	glBindVertexArray(this->VertexArrayID);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->VertexBufferID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), ((GLvoid*)(0)));

	//TODO: pass the color values to vertex shader


	//TODO: pass the normal values to vertex shader

	
	if (this->type == DRAW_TYPE::ARRAY)
	{
		glDrawArrays(GL_TRIANGLES, 0, this->vertices.size());
	}
	else {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->IndexBufferID);
		glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, ((GLvoid *)0));
	}
}

void Model::drawPicking()
{
	if (this->objectID >= 0) 
	{
		glUseProgram(this->PickingProgramID);
		GLuint ProjectionID = glGetUniformLocation(this->PickingProgramID, "Projection");
		GLuint EyeID = glGetUniformLocation(this->PickingProgramID, "Eye");
		GLuint ModelTransformID = glGetUniformLocation(this->PickingProgramID, "ModelTransform");
		GLuint objectIDLoc = glGetUniformLocation(this->PickingProgramID, "objectID");

		glUniformMatrix4fv(ProjectionID, 1, GL_FALSE, &(*(this->Projection))[0][0]);
		glUniformMatrix4fv(EyeID, 1, GL_FALSE, &(*(this->Eye))[0][0]);
		glUniformMatrix4fv(ModelTransformID, 1, GL_FALSE, &(*(this->ModelTransform))[0][0]);
		
		float r = ((objectID >> 16) & 0xFF) / 255.0f;
		float g = ((objectID >> 8) & 0xFF) / 255.0f;
		float b = (objectID & 0xFF) / 255.0f;

		glm::vec3 objectIDVector = glm::vec3(r,g,b);
		glUniform3f(objectIDLoc, objectIDVector.x, objectIDVector.y, objectIDVector.z);

		glBindVertexArray(this->VertexArrayID);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, this->VertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), ((GLvoid*)(0)));

		if (this->type == DRAW_TYPE::ARRAY)
		{
			glDrawArrays(GL_TRIANGLES, 0, this->vertices.size());
		}
		else {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->IndexBufferID);
			glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, ((GLvoid *)0));
		}
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

	// Cleanup VBO and shader
	glDisableVertexAttribArray(0);
	glDeleteBuffers(1, &this->VertexBufferID);
	
	//TODO: delete color buffer
	
	//TODO: delete normal buffer
	
	
	if (this->type == DRAW_TYPE::INDEX) glDeleteBuffers(1, &this->IndexBufferID);
	glDeleteProgram(this->GLSLProgramID);
	glDeleteVertexArrays(1, &this->VertexArrayID);
}

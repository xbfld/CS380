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
void Model::add_texcoord(float s, float t){
	texcoords.push_back(glm::vec2(s,t));
}
void Model::add_texcoord(glm::vec2 norm){
	texcoords.push_back(norm);
}
void Model::add_tangent(glm::vec3 tan){
	tangents.push_back(tan);
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
	glGenBuffers(1, &this->ColorBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, this->ColorBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*this->colors.size(), &this->colors[0], GL_STATIC_DRAW);

	//TODO: generate/bind buffer for normals and store the normal values
	glGenBuffers(1, &this->NormalBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, this->NormalBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*this->normals.size(), &this->normals[0], GL_STATIC_DRAW);

	//texture
	glGenBuffers(1, &this->TexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, this->TexBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*this->texcoords.size(), &this->texcoords[0], GL_STATIC_DRAW);
	//cout << texcoords.size() << endl;

	//tangent bitangent
	glGenBuffers(1, &this->TangentID);
	glBindBuffer(GL_ARRAY_BUFFER, this->TangentID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*this->tangents.size(), &this->tangents[0], GL_STATIC_DRAW);		
}

void Model::initialize(DRAW_TYPE type,  GLuint program)
{
	this->GLSLProgramID = program;
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
	glGenBuffers(1, &this->ColorBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, this->ColorBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*this->colors.size(), &this->colors[0], GL_STATIC_DRAW);

	//TODO: generate/bind buffer for normals and store the normal values
	glGenBuffers(1, &this->NormalBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, this->NormalBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*this->normals.size(), &this->normals[0], GL_STATIC_DRAW);

	//texture
	glGenBuffers(1, &this->TexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, this->TexBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*this->texcoords.size(), &this->texcoords[0], GL_STATIC_DRAW);	

	//tangent bitangent
	glGenBuffers(1, &this->TangentID);
	glBindBuffer(GL_ARRAY_BUFFER, this->TangentID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*this->tangents.size(), &this->tangents[0], GL_STATIC_DRAW);		
}
void Model::initialize(DRAW_TYPE type, Model model){
	this->GLSLProgramID = model.GLSLProgramID;
	this->type = type;
	
	this->VertexArrayID = model.VertexArrayID;
	glBindVertexArray(this->VertexArrayID);
	
	
	this->VertexBufferID = model.VertexBufferID;
	glBindBuffer(GL_ARRAY_BUFFER, this->VertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*model.vertices.size(), &model.vertices[0], GL_STATIC_DRAW);

	//TODO: generate/bind buffer for colors and store the color values	
	this->ColorBufferID = model.ColorBufferID;
	glBindBuffer(GL_ARRAY_BUFFER, this->ColorBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*model.colors.size(), &model.colors[0], GL_STATIC_DRAW);

	//TODO: generate/bind buffer for normals and store the normal values	
	this->NormalBufferID = model.ColorBufferID;
	glBindBuffer(GL_ARRAY_BUFFER, this->NormalBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*model.normals.size(), &model.normals[0], GL_STATIC_DRAW);
	
	//TODO: texture coordinate
	this->TexBufferID = model.TexBufferID;	
	glBindBuffer(GL_ARRAY_BUFFER, this->TexBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*model.texcoords.size(), &model.texcoords[0], GL_STATIC_DRAW);	

	//tangent
	this->TangentID = model.TangentID;
	
	glBindBuffer(GL_ARRAY_BUFFER, this->TangentID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*model.tangents.size(), &model.tangents[0], GL_STATIC_DRAW);		
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

bool Model::loadOBJ2(const char * path){
	cout << "Loading OBJ file with texture coordinates" << path << "..." << endl;
	vector <glm::vec3> points;
	vector <glm::vec3> norms;
	vector <glm::vec2> texCoords;
	vector <glm::vec3> faces;
	
	ifstream in(path, ios::in);
	if (!in)
	{
		cerr << "Cannot open " << path << endl; exit(1);
	}

	string line;
	vector<glm::vec3> face;
	bool isTc = true;
	while (getline(in, line))
	{
		if (line.substr(0, 2) == "v ")
		{			
			istringstream s(line.substr(2));
			glm::vec3 v; s >> v.x; s >> v.y; s >> v.z; //v.w = 1.0f;
			points.push_back(v);
		}
		else if (line.substr(0, 3) == "vt "){			
			float s, t;
			istringstream fs(line.substr(3));
			fs >> s >> t;
			texCoords.push_back(glm::vec2(s, t));
		}
		else if (line.substr(0, 3) == "vn ")
		{			
			float x, y, z;
			istringstream fs(line.substr(3));
			fs >> x >> y >> z;
			norms.push_back(glm::vec3(x, y, z));
		}		
		else if (line.substr(0, 2) == "f "){
			face.clear();
			istringstream fs(line.substr(2));
			size_t slash1, slash2;
			while (fs.good()){
				string vertString;
				fs >> vertString;
				//cout << vertString << endl;
				int pIndex = -1, nIndex = -1, tcIndex = -1;
				slash1 = vertString.find("/");
				if (slash1 == string::npos){
					pIndex = atoi(vertString.c_str()) - 1;
				}
				else{
					slash2 = vertString.find("/", slash1 + 1);
					pIndex = atoi(vertString.substr(0, slash1).c_str())
						- 1;
					if (slash2 > slash1 + 1) {
						tcIndex =
							atoi(vertString.substr(slash1 + 1, slash2).c_str())
							- 1;
					}
					nIndex =
						atoi(vertString.substr(slash2 + 1, vertString.length()).c_str())
						- 1;
				}
				if (pIndex == -1) {
					printf("Missing point index!!!");
				}
				else {
					face.push_back(glm::vec3(pIndex,tcIndex,nIndex));
				}
				//cout << tcIndex << " " << pIndex << endl;
				//cout << nIndex << " " << pIndex << endl;
				if (tcIndex == -1){					
					isTc = false;
				}
				/*if (false && tcIndex != -1 && pIndex != tcIndex) {//texture load good or not
					printf("Texture and point indices are not consistent.\n");
					exit(1);
					}
					if (nIndex != -1 && nIndex != pIndex) {
					printf("Normal and point indices are not consistent.\n");
					exit(1);
					}*/
			}			
			if (face.size() > 3) {
				//cout << "HERER" ;
				glm::vec3 v0 = face[0];
				glm::vec3 v1 = face[1];
				glm::vec3 v2 = face[2];
				// First face
				faces.push_back(v0);
				faces.push_back(v1);
				faces.push_back(v2);
				for (GLuint i = 3; i < face.size(); i++) {
					v1 = v2;
					v2 = face[i];
					faces.push_back(v0);
					faces.push_back(v1);
					faces.push_back(v2);
				}
			}
			else {
				faces.push_back(face[0]);
				faces.push_back(face[1]);
				faces.push_back(face[2]);
			}
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
	for (int i = 0; i < faces.size(); i += 3)
	{
		GLushort via = faces[i][0];
		GLushort vib = faces[i + 1][0];
		GLushort vic = faces[i + 2][0];
		
		add_vertex(points[via]);
		add_vertex(points[vib]);
		add_vertex(points[vic]);

		GLushort nia = faces[i][2];
		GLushort nib = faces[i + 1][2];
		GLushort nic = faces[i + 2][2];

		add_normal(norms[nia]);
		add_normal(norms[nib]);
		add_normal(norms[nic]);
				
		if (isTc){
			GLushort tia = faces[i][1];
			GLushort tib = faces[i + 1][1];
			GLushort tic = faces[i + 2][1];

			add_texcoord(texCoords[tia]);
			add_texcoord(texCoords[tib]);
			add_texcoord(texCoords[tic]);

			glm::vec3 deltaPos1 = points[vib] - points[via];
			glm::vec3 deltaPos2 = points[vic] - points[via];

			glm::vec2 deltaUV1 = texCoords[tib] - texCoords[tia];
			glm::vec2 deltaUV2 = texCoords[tic] - texCoords[tia];

			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;			
			tangents.push_back(tangent);
			tangents.push_back(tangent);
			tangents.push_back(tangent);			
		}
	}		
	cout << points.size() << " " << norms.size() << " " << texCoords.size() << endl;
	cout << vertices.size() << " " << normals.size() << " " << texcoords.size() <<endl;	
	return true;
}



void Model::draw()
{	
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
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, this->ColorBufferID);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), ((GLvoid*)(0)));

	//TODO: pass the normal values to vertex shader
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->NormalBufferID);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), ((GLvoid*)(0)));
	
	//texture
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, this->TexBufferID);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);


	//tangent bitangent	
	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, this->TangentID);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		
	if (this->type == DRAW_TYPE::ARRAY)
	{
		glDrawArrays(GL_TRIANGLES, 0, this->vertices.size());
	}
	else {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->IndexBufferID);
		glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, ((GLvoid *)0));
	}	
}

void Model::draw2(Model model)
{	
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
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, this->ColorBufferID);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), ((GLvoid*)(0)));

	//TODO: pass the normal values to vertex shader
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, this->NormalBufferID);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), ((GLvoid*)(0)));

	//texture
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, this->TexBufferID);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//tangent bitangent	
	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, this->TangentID);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	

	if (this->type == DRAW_TYPE::ARRAY)
	{
		glDrawArrays(GL_TRIANGLES, 0, model.vertices.size());
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
	glDisableVertexAttribArray(2);
	glDeleteBuffers(1, &this->ColorBufferID);
	
	//TODO: delete normal buffer
	glDisableVertexAttribArray(1);
	glDeleteBuffers(1, &this->NormalBufferID);
	
	glDisableVertexAttribArray(3);
	glDeleteBuffers(1, &this->TexBufferID);

		
	if (this->type == DRAW_TYPE::INDEX) glDeleteBuffers(1, &this->IndexBufferID);
	glDeleteProgram(this->GLSLProgramID);
	glDeleteVertexArrays(1, &this->VertexArrayID);	
}
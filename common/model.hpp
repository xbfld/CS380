#ifndef MODEL_HPP
#define MODEL_HPP

#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>

enum DRAW_TYPE {
	ARRAY,
	INDEX
};

class Model {
	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colors;
	std::vector<glm::vec2> texcoords;
	std::vector<glm::vec3> tangents;	

	glm::mat4* Projection;
	glm::mat4* Eye;
	glm::mat4* ModelTransform;
	
	DRAW_TYPE type;	

public:
	GLuint GLSLProgramID;
	GLuint PickingProgramID;
	GLuint VertexArrayID;
	GLuint VertexBufferID;
	GLuint IndexBufferID;
	GLuint NormalBufferID;
	GLuint ColorBufferID;
	GLuint TexBufferID;
	GLuint TangentID;	
	int objectID = -1;	

	Model();
	bool loadOBJ(const char * path, glm::vec3 color);
	bool loadOBJ2(const char * path);
	void add_vertex(float, float, float);
	void add_vertex(glm::vec3);
	void add_normal(float, float, float);
	void add_normal(glm::vec3);
	void add_texcoord(float, float);
	void add_texcoord(glm::vec2);
	void add_color(float, float, float);
	void add_color(glm::vec3);
	void add_index(unsigned int);
	void add_tangent(glm::vec3);
	void set_projection(glm::mat4*);
	void set_eye(glm::mat4*);
	glm::mat4* get_model(void);
	void set_model(glm::mat4*);
	void initialize(DRAW_TYPE, const char *, const char *);
	void initialize(DRAW_TYPE, GLuint);
	void initialize(DRAW_TYPE, Model);
	void initialize_picking(const char *, const char *);
	void draw(void);
	void draw2(Model );
	void drawPicking(void);
	void cleanup(void);			
};

#endif
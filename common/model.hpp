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

	glm::mat4* Projection;
	glm::mat4* Eye;
	glm::mat4* ModelTransform;

	DRAW_TYPE type;

	GLuint VertexArrayID;
	GLuint VertexBufferID;
	GLuint IndexBufferID;
	GLuint NormalBufferID;
	GLuint ColorBufferID;
public:
	GLuint GLSLProgramID;

	Model();
	void add_vertex(float, float, float);
	void add_vertex(glm::vec3);
	void add_normal(float, float, float);
	void add_normal(glm::vec3);
	void add_color(float, float, float);
	void add_color(glm::vec3);
	void add_index(unsigned int);
	void set_projection(glm::mat4*);
	void set_eye(glm::mat4*);
	void set_model(glm::mat4*);
	void initialize(DRAW_TYPE, const char *, const char *);
	void draw(void);
	void cleanup(void);
};

#endif

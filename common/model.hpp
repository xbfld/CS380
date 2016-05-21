#ifndef MODEL_HPP
#define MODEL_HPP

#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>

enum DRAW_TYPE {
	ARRAY,
	INDEX
};
enum SHADER_TYPE {
	PHONG,
	FLAT,
	TOON,
	LAST
};
struct Material
{
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float shiness;
};
class Model {
	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colors;

	glm::mat4* Projection;
	glm::mat4* Eye;
	glm::mat4* OffsetFrame;
	glm::mat4* ModelTransform;
	
	DRAW_TYPE type;

	GLuint VertexArrayID;
	GLuint VertexBufferID;
	GLuint IndexBufferID;
	GLuint NormalBufferID;
	GLuint ColorBufferID;
public:
	GLuint GLSLProgramID;
	GLuint PickingProgramID;
	int objectID = -1;
	Material material;
	SHADER_TYPE shader_type;

	Model();
	bool loadOBJ(const char * path, glm::vec3 color);
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
	glm::mat4* get_model(void);
	void set_offset(glm::mat4*);
	glm::mat4* get_offset(void);
	Material get_material(void);
	void set_material(Material);
	SHADER_TYPE get_shader_type(void);
	void set_shader_type(SHADER_TYPE);
	void initialize(DRAW_TYPE, const char *, const char *);
	void initialize_picking(const char *, const char *);
	void draw(void);
	void drawPicking(void);
	void cleanup(void);
};

#endif
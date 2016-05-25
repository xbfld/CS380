#include <common/arcball.hpp>

Arcball::Arcball(const char* vertexshader_file, const char* fragmentshader_file, glm::mat4& projection, glm::mat4& eye)
{
	this->model = Model();
	init_sphere(this->model);
	this->model.initialize(DRAW_TYPE::INDEX, vertexshader_file, fragmentshader_file);
	this->model.set_projection(&projection);
	this->model.set_eye(&eye);
	this->transform = glm::mat4(1.0f);
	this->model.set_model(&this->transform);
}

void Arcball::set_model(glm::mat4 model)
{
	this->transform = model;
}

void Arcball::apply_transform(glm::mat4 model)
{
	this->transform = model * this->transform;
}

float Arcball::compute_z_with_arcball(glm::mat4& eyeRBT)
{
	glm::mat4 eye_matrix = glm::inverse(eyeRBT) * this->transform;
	return eye_matrix[3][2];
}

void Arcball::draw()
{
	this->model.draw();
}
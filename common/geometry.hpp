#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <common/model.hpp>

glm::vec3 vertices[8] = {
	glm::vec3(-0.5, -0.5, 0.5),
	glm::vec3(-0.5, 0.5, 0.5),
	glm::vec3(0.5, 0.5, 0.5),
	glm::vec3(0.5, -0.5, 0.5),
	glm::vec3(-0.5, -0.5, -0.5),
	glm::vec3(-0.5, 0.5, -0.5),
	glm::vec3(0.5, 0.5, -0.5),
	glm::vec3(0.5, -0.5, -0.5)
};

glm::vec3 sky_vertices[8] = {
	glm::vec3(-25.0, -25.0, 25.0),
	glm::vec3(-25.0, 25.0, 25.0),
	glm::vec3(25.0, 25.0, 25.0),
	glm::vec3(25.0, -25.0, 25.0),
	glm::vec3(-25.0, -25.0, -25.0),
	glm::vec3(-25.0, 25.0, -25.0),
	glm::vec3(25.0, 25.0, -25.0),
	glm::vec3(25.0, -25.0, -25.0)
};

void compute_normal(Model &model, glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
	model.add_normal(glm::normalize(glm::cross(b - a, c - a)));
	model.add_normal(glm::normalize(glm::cross(b - a, c - a)));
	model.add_normal(glm::normalize(glm::cross(b - a, c - a)));
}

void quad(Model &model, int a, int b, int c, int d, glm::vec3 color)
{
	model.add_vertex(vertices[a]);
	model.add_vertex(vertices[b]);
	model.add_vertex(vertices[c]);
	model.add_vertex(vertices[a]);
	model.add_vertex(vertices[c]);
	model.add_vertex(vertices[d]);

	compute_normal(model, vertices[a], vertices[b], vertices[c]);
	compute_normal(model, vertices[a], vertices[c], vertices[d]);

	model.add_color(color);
	model.add_color(color);
	model.add_color(color);
	model.add_color(color);
	model.add_color(color);
	model.add_color(color);
}

void quad2(Model &model, int a, int b, int c, int d, glm::vec3 color)
{
	model.add_vertex(sky_vertices[a]);
	model.add_vertex(sky_vertices[b]);
	model.add_vertex(sky_vertices[c]);
	model.add_vertex(sky_vertices[a]);
	model.add_vertex(sky_vertices[c]);
	model.add_vertex(sky_vertices[d]);

	model.add_texcoord(glm::vec2(1.0, 1.0));
	model.add_texcoord(glm::vec2(1.0, -1.0));
	model.add_texcoord(glm::vec2(-1.0, -1.0));
	model.add_texcoord(glm::vec2(-1.0, 1.0));
	model.add_texcoord(glm::vec2(1.0, 1.0));
	model.add_texcoord(glm::vec2(1.0, -1.0));	

	compute_normal(model, sky_vertices[a], sky_vertices[b], sky_vertices[c]);
	compute_normal(model, sky_vertices[a], sky_vertices[c], sky_vertices[d]);

	model.add_color(color);
	model.add_color(color);
	model.add_color(color);
	model.add_color(color);
	model.add_color(color);
	model.add_color(color);
}
void quad3(Model &model, int a, int b, int c, int d)
{
	model.add_vertex(vertices[a]);
	model.add_vertex(vertices[b]);
	model.add_vertex(vertices[c]);
	model.add_texcoord(glm::vec2(0.0, 0.0));
	model.add_texcoord(glm::vec2(0.0, 1.0));
	model.add_texcoord(glm::vec2(1.0, 1.0));	

	glm::vec3 deltaPos1 = vertices[b] - vertices[a];
	glm::vec3 deltaPos2 = vertices[c] - vertices[a];

	glm::vec2 deltaUV1 = glm::vec2(0.0, 1.0) - glm::vec2(0.0, 0.0);
	glm::vec2 deltaUV2 = glm::vec2(1.0, 1.0) - glm::vec2(0.0, 0.0);

	float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
	glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
	model.add_tangent(tangent);
	model.add_tangent(tangent);
	model.add_tangent(tangent);
	
	//////////////////////
	model.add_vertex(vertices[a]);
	model.add_vertex(vertices[c]);
	model.add_vertex(vertices[d]);
	model.add_texcoord(glm::vec2(0.0, 0.0));
	model.add_texcoord(glm::vec2(1.0, 1.0));
	model.add_texcoord(glm::vec2(1.0, 0.0));

	deltaPos1 = vertices[c] - vertices[a];
	deltaPos2 = vertices[d] - vertices[a];

	deltaUV1 = glm::vec2(1.0, 1.0) - glm::vec2(0.0, 0.0);
	deltaUV2 = glm::vec2(1.0, 0.0) - glm::vec2(0.0, 0.0);

	r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
	tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
	model.add_tangent(tangent);
	model.add_tangent(tangent);
	model.add_tangent(tangent);


	compute_normal(model, vertices[a], vertices[b], vertices[c]);
	compute_normal(model, vertices[a], vertices[c], vertices[d]);

	model.add_color(glm::vec3(1.0,1.0,1.0));
	model.add_color(glm::vec3(1.0, 1.0, 1.0));
	model.add_color(glm::vec3(1.0, 1.0, 1.0));
	model.add_color(glm::vec3(1.0, 1.0, 1.0));
	model.add_color(glm::vec3(1.0, 1.0, 1.0));
	model.add_color(glm::vec3(1.0, 1.0, 1.0));
}

void quad4(Model &model, int a, int b, int c, int d,float num)
{
	//TODO: Modify texture coordinate according to task.bmp file
	
}

void init_cube(Model &model, glm::vec3 color)
{
	quad(model, 1, 0, 3, 2, color);
	quad(model, 2, 3, 7, 6, color);
	quad(model, 3, 0, 4, 7, color);
	quad(model, 6, 5, 1, 2, color);
	quad(model, 4, 5, 6, 7, color);
	quad(model, 5, 4, 0, 1, color);
}
void init_texture_cube(Model &model){
	quad3(model, 1, 0, 3, 2);
	quad3(model, 2, 3, 7, 6);
	quad3(model, 3, 0, 4, 7);
	quad3(model, 6, 5, 1, 2);
	quad3(model, 4, 5, 6, 7);
	quad3(model, 5, 4, 0, 1);
	//TODO: Change quad3 into quad4 with proper face numbering
	
}
void init_skybox(Model &model){		
	glm::vec3 coco = glm::vec3(1.0f, 1.0f, 1.0f);
	quad2(model, 1, 0, 3, 2, coco);
	quad2(model, 2, 3, 7, 6, coco);
	quad2(model, 3, 0, 4, 7, coco);
	quad2(model, 6, 5, 1, 2, coco);
	quad2(model, 4, 5, 6, 7, coco);
	quad2(model, 5, 4, 0, 1, coco);
}

void init_rubic(Model& model, glm::vec3* colors)
{
	quad(model, 1, 0, 3, 2, colors[0]);
	quad(model, 2, 3, 7, 6, colors[4]);
	quad(model, 3, 0, 4, 7, colors[5]);
	quad(model, 6, 5, 1, 2, colors[3]);
	quad(model, 4, 5, 6, 7, colors[1]);
	quad(model, 5, 4, 0, 1, colors[2]);
}

void init_ground(Model &model)
{
	glm::vec3 a = glm::vec3(-0.5f, 0.0f, -0.5f);
	glm::vec3 b = glm::vec3(0.5f, 0.0f, -0.5f);
	glm::vec3 c = glm::vec3(-0.5f, 0.0f, 0.5f);
	glm::vec3 d = glm::vec3(0.5f, 0.0f, 0.5f);
	model.add_vertex(a);
	model.add_vertex(c);
	model.add_vertex(b);
	model.add_vertex(b);
	model.add_vertex(c);
	model.add_vertex(d);

	compute_normal(model, a, c, b);
	compute_normal(model, b, c, d);

	//glm::vec3 color = glm::vec3(0.1, 0.95, 0.1);
	glm::vec3 color = glm::vec3(1.0, 1.0, 1.0);
	model.add_color(color);
	model.add_color(color);
	model.add_color(color);
	model.add_color(color);
	model.add_color(color);
	model.add_color(color);
}

void init_sphere(Model &model)
{
	float radius = 1.0f;
	unsigned int rings = 30, sectors = 30;
	float R = 1.0f / (float)(rings - 1);
	float S = 1.0f / (float)(sectors - 1);
	float PI = glm::pi<float>();

	for (unsigned int r = 0; r < rings; r++)
	{
		for (unsigned int s = 0; s < sectors; s++)
		{
			float x = (float)cos(2 * PI * s * S) * sin(PI * r * R);
			float y = (float)sin(-(PI / 2.0f) + PI * r * R);
			float z = (float)sin(2 * PI * s * S) * sin(PI * r * R);

			model.add_vertex(glm::vec3(x * radius, y * radius, z * radius));
			model.add_normal(glm::vec3(x, y, z));
			model.add_color(glm::vec3(1.0f, 1.0f, 1.0f));
		}
	}

	for (unsigned int r = 0; r < rings - 1; r++)
	{
		for (unsigned int s = 0; s < sectors - 1; s++)
		{
			model.add_index(r * sectors + (s + 1)); // 2
			model.add_index((r + 1) * sectors + s); // 3
			model.add_index((r + 1) * sectors + (s + 1)); // 4 
			model.add_index(r * sectors + s); // 1 
			model.add_index((r + 1) * sectors + s); // 3
			model.add_index(r * sectors + (s + 1)); // 2
		}
	}
}

void init_obj(Model &model, char *path, glm::vec3 color){
	bool load = model.loadOBJ(path, color);
	if (!load){
		std::cout << "imposible to load OBJ file" << std::endl;
		system("pause");
		exit(1);
	}
}

void init_obj2(Model &model, char *path){
	bool load = model.loadOBJ2(path);
	if (!load){
		std::cout << "imposible to load OBJ with texture " << std::endl;
		system("pause");
		exit(1);
	}
}


#endif

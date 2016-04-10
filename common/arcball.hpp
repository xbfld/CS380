#ifndef ARCBALL_H
#define ARCBALL_H
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

/*
 * eye_to_screen: mapping eye coordinate to screen coordinate
 */
inline glm::vec2 eye_to_screen(glm::vec3& eye_coord,
							   glm::mat4& Projection,
							   int frameBufferWidth, int frameBufferHeight)
{
	glm::vec2 center = glm::vec2((float)(frameBufferWidth - 1)/2.0f, (float)(frameBufferHeight - 1)/2.0f);
	if (eye_coord.z > -(1e-8)) return center;

	glm::vec4 q = Projection * glm::vec4(eye_coord.x, eye_coord.y, eye_coord.z, 1.0f);
	glm::vec3 clip_coord = glm::vec3(q.x, q.y, q.z) / q.w;
	return glm::vec2(clip_coord.x * frameBufferWidth / 2.0f + center.x, clip_coord.y * frameBufferHeight / 2.0f + center.y);
}

/*
 * It keeps same arcball size within same eye frame
 */
inline float compute_screen_eye_scale(float z, float fovy, int frameBufferHeight)
{
	if (z > -(1e-8)) return 1;
	return -(z * tan(fovy * glm::pi<float>() / 360.0f) * 2.0f) / (float)frameBufferHeight;
}
#endif
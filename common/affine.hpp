#ifndef AFFINE_H
#define AFFINE_H
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

/*
 * An affine matrix A can be factored as A = TL. You need to fill up two function named 'linearFact' and 'transFact'
 */

// DONE: Fill up linearFact function
// input: A (4 x 4 matrix)
// output: L (4 x 4 matrix)
glm::mat4 linearFact(glm::mat4 A)
{
	glm::mat4 L = A;
	for (size_t i = 0; i < 3; i++)
	{
		L[3][i] = 0.0f;
	}
	return L;
}

// DONE: Fill up transFact function
// input: A (4 x 4 matrix)
// output: T (4 x 4 matrix)
glm::mat4 transFact(glm::mat4 A)
{
	glm::mat4 T = glm::mat4(1.0f);
	for (size_t i = 0; i < 4; i++)
	{
		T[3][i] = A[3][i];
	}
	return T;
}
#endif

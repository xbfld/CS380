#ifndef PICKING_H
#define PICKING_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <common/model.hpp>

// Picking Pass Rendering
GLuint picking_fbo;
GLuint picking_tex;

inline void picking_initialize(int frameBufferWidth, int frameBufferHeight)
{
	glGenFramebuffers(1, &picking_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, picking_fbo);

	glGenTextures(1, &picking_tex);
	glBindTexture(GL_TEXTURE_2D, picking_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, frameBufferWidth, frameBufferHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, picking_tex, 0);

	glReadBuffer(GL_NONE);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR: Framebuffer is not complete" << std::endl;
		std::cin.get();
		std::terminate();
	}

	// Unbind this framebuffer
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

inline void reallocate_picking_texture(int frameBufferWidth, int frameBufferHeight)
{
	glDeleteTextures(1, &picking_tex);
	glBindFramebuffer(GL_FRAMEBUFFER, picking_fbo);

	glGenTextures(1, &picking_tex);
	glBindTexture(GL_TEXTURE_2D, picking_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, frameBufferWidth, frameBufferHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, picking_tex, 0);

	glReadBuffer(GL_NONE);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR: Framebuffer is not complete" << std::endl;
		std::cin.get();
		std::terminate();
	}

	// Unbind this framebuffer
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

inline int pick(int xpos, int ypos, int frameBufferWidth, int frameBufferHeight)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, picking_fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	unsigned char pixel[3];
	glReadPixels(xpos, frameBufferHeight - ypos - 1, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &pixel);

	int r = (int)pixel[0];
	int g = (int)pixel[1];
	int b = (int)pixel[2];
	int targetID = ((r << 16) & 0xFF0000) + ((g << 8) & 0x00FF00) + b & 0xFF;

	std::cout << r << ", " << g << ", " << b << std::endl;
	std::cout << targetID << std::endl;

	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	return targetID;
}

inline void delete_picking_resources()
{
	glDeleteTextures(1, &picking_tex);
	glDeleteFramebuffers(1, &picking_fbo);
}

#endif

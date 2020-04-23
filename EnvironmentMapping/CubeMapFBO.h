#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <iostream>

class CubeMapFBO
{
public:
	CubeMapFBO(const int width, const int height)
	{
		glGenTextures(1, &m_cubemapTexture);

		glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemapTexture);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		for (int i = 0; i < 6; i++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

		
		glGenFramebuffers(1, &m_cubemapFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_cubemapFBO);

		glGenRenderbuffers(1, &m_cubemapDepthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_cubemapDepthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_cubemapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_cubemapTexture, 0);

		VerifyFBStatus("..Error in creating frame buffer when initialization");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);


	}

	void BeginRendering() const
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_cubemapFBO);
	}

	static void EndRendering()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void DrawToFace(const int face, glm::vec3 origin, glm::mat4& persp_cm, glm::mat4& view_cm)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + (int)face, m_cubemapTexture, NULL);
		VerifyFBStatus(".. Error in creating frame buffer when Rendering");

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		persp_cm = glm::perspective(glm::radians(90.0f), 1.0f, 0.10f, 100.0f);
		view_cm = glm::mat4();
		switch (face)
		{
		case POSITIVE_X:
			view_cm = glm::lookAt(glm::vec3(0.0), glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
			break;

		case NEGATIVE_X:
			view_cm = glm::lookAt(glm::vec3(0.0), glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
			break;

		case POSITIVE_Y:
			view_cm = glm::lookAt(glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
			break;

		case NEGATIVE_Y:
			view_cm = glm::lookAt(glm::vec3(0.0), glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
			break;

		case POSITIVE_Z:
			view_cm = glm::lookAt(glm::vec3(0.0), glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
			break;

		case NEGATIVE_Z:
			view_cm = glm::lookAt(glm::vec3(0.0), glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
			break;

		default:
			break;
		}
		view_cm = glm::translate(view_cm, -glm::vec3(origin));
	}

	inline glm::mat4 getPerspMat() const { return m_cubemapPersp; }
	inline glm::mat4 getViewMat() const  { return m_cubemapView; }
	inline GLuint getCubemapTexture() const { return m_cubemapTexture; }

private:
	GLuint m_cubemapTexture;
	GLuint m_cubemapFBO;
	GLuint m_cubemapDepthBuffer;

	glm::mat4 m_cubemapPersp;
	glm::mat4 m_cubemapView;

	enum
	{
		POSITIVE_X = 0,
		NEGATIVE_X = 1,
		POSITIVE_Y = 2,
		NEGATIVE_Y = 3,
		POSITIVE_Z = 4,
		NEGATIVE_Z = 5
	};

	void VerifyFBStatus(const char* str)
	{
		GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
			std::cout << str << std::endl;
	}
};

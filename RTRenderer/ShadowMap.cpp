#include "ShadowMap.h"

ShadowMap::ShadowMap() :
	mFBO(0),
	mSM(0)
{
}

bool ShadowMap::Init(GLuint width, GLuint height)
{
	mSWidth = width; mSHeight = height;

	glGenFramebuffers(1, &mFBO);

	glGenTextures(1, &mSM);
	glBindTexture(GL_TEXTURE_2D, mSM);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, mSWidth, mSHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float bColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, bColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mSM, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("Frame buffer Error %i\n", status);
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void ShadowMap::Write()
{
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
}

void ShadowMap::Read(GLenum textureUnit)
{
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, mSM);
}

GLuint ShadowMap::GetShadowWidth()
{
	return mSWidth;
}

GLuint ShadowMap::GetShadowHeight()
{
	return mSHeight;
}

ShadowMap::~ShadowMap()
{
	if (mFBO)
		glDeleteFramebuffers(1, &mFBO);

	if (mSM)
		glDeleteTextures(1, &mSM);
}


OmniShadowMap::OmniShadowMap() : ShadowMap() {}

bool OmniShadowMap::Init(unsigned int width, unsigned int height)
{
	mSWidth = width; mSHeight= height;

	glGenFramebuffers(1, &mFBO);

	glGenTextures(1, &mSM);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mSM);

	for (size_t i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, mSWidth, mSHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mSM, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (Status != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("Framebuffer error: %i\n", Status);
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void OmniShadowMap::Write()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBO);
}

void OmniShadowMap::Read(GLenum texUnit)
{
	glActiveTexture(texUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mSM);
}

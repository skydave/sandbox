#pragma once
#include "../sys/msys.h"

struct Attribute;

struct Texture
{
	Texture( int width, int height, int textureFormat = GL_RGB_FLOAT16_ATI, int pixelFormat=GL_RGB, int componentType = GL_FLOAT, void *pixels = NULL );

	Attribute *getUniform();

	unsigned int m_id;
	Attribute *m_uniform; // will be used to attach textures to geometry/or shaders which then will be bound
};



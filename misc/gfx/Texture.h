#pragma once
#include "../sys/msys.h"

struct Attribute;

struct Texture
{
	Texture( int xres, int yres, int textureFormat = GL_RGBA_FLOAT32_ATI, int pixelFormat=GL_RGBA, int componentType = GL_FLOAT, void *pixels = NULL );

	Attribute *getUniform();

	unsigned int                                            m_id;
	Attribute                                         *m_uniform; // will be used to attach textures to geometry/or shaders which then will be bound
	int                                           m_xres, m_yres; // texture info just for the record
	int          m_textureFormat, m_pixelFormat, m_componentType; //

	void setSize( int newXres, int newYres );
	Texture *copy(); // will return a texture with its own unique texture id


	static Texture *createRGBA8( int xres, int yres, unsigned char *pixels = 0 );
};



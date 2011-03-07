#include "Texture.h"
#include "Attribute.h"



Texture::Texture( int width, int height, int textureFormat, int pixelFormat, int componentType, void *pixels  ) : m_uniform(0)
{
	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_2D, m_id);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_2D, 0, textureFormat,  width, height, 0, pixelFormat, componentType, pixels);

	// when texture area is small, bilinear filter the closest mipmap
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	// when texture area is large, bilinear filter the original
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	// the texture wraps over at the edges (repeat)
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	printf( "texid: %i\n", m_id );
}


Attribute *Texture::getUniform()
{
	if( !m_uniform )
	{
		m_uniform = new Attribute( 1, sizeof(int) );
		m_uniform->m_componentType = ATTR_TYPE_SAMPLER;
		m_uniform->appendElement( (int)m_id );
	}
	return m_uniform;
}

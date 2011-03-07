#pragma once
#include "../math/Math.h"
#include "../std/vector.h"


#define ATTR_TYPE_SAMPLER  0




// basicly a list manager
struct Attribute
{
	Attribute( char numComponents=3, char componentSize=sizeof(float) ) : m_numElements(0), m_numComponents(numComponents), m_componentSize(componentSize)
	{
		oglGenBuffers(1, &m_bufferId);

		m_componentType = GL_FLOAT;
	}

	Attribute *copy();

	virtual void bindAsAttribute( int index );
	virtual void unbindAsAttribute( int index );
	virtual void bindAsUniform( int index );
	virtual void unbindAsUniform( int index );

	int appendElement( const float &value )
	{
		int pos = m_data.size();
		m_data.resize( pos + sizeof(float) );
		*((float *)&m_data.m_data[pos]) = value;
		++m_numElements;
		return m_numElements;
	}
	int appendElement( const math::Vec3f &value )
	{
		int pos = m_data.size();
		m_data.resize( pos + sizeof(math::Vec3f) );
		*((math::Vec3f *)&m_data.m_data[pos]) = value;
		return m_numElements++;
	}
	int appendElement( const float &f0, const float &f1, const float &f2, const float &f3 )
	{
		int pos = m_data.size();
		m_data.resize( pos + sizeof(float)*4 );
		float *data = (float*)&m_data.m_data[pos];
		*data = f0;++data;
		*data = f1;++data;
		*data = f2;++data;
		*data = f3;++data;
		return m_numElements++;
	}
	int appendElement( const float &f0, const float &f1, const float &f2 )
	{
		int pos = m_data.size();
		m_data.resize( pos + sizeof(float)*3 );
		float *data = (float*)&m_data.m_data[pos];
		*data = f0;++data;
		*data = f1;++data;
		*data = f2;++data;
		return m_numElements++;
	}
	int appendElement( const float &f0, const float &f1 )
	{
		int pos = m_data.size();
		m_data.resize( pos + sizeof(float)*2 );
		float *data = (float*)&m_data.m_data[pos];
		*data = f0;++data;
		*data = f1;++data;
		return m_numElements++;
	}
	int appendElement( const int &i0, const int &i1, const int &i2 )
	{
		int pos = m_data.size();
		m_data.resize( pos + sizeof(int)*3 );
		int *data = (int*)&m_data.m_data[pos];
		*data = i0;++data;
		*data = i1;++data;
		*data = i2;++data;
		return m_numElements++;
	}
	int appendElement( const int &i0 )
	{
		int pos = m_data.size();
		m_data.resize( pos + sizeof(int)*1 );
		int *data = (int*)&m_data.m_data[pos];
		*data = i0;++data;
		return m_numElements++;
	}
	math::Vec3f getVec3f( int vertexIndex );
	int appendElement( void *mem );
	int appendElements( int num );
	void getElement( int vertexIndex, void *mem );
	void setElement( int vertexIndex, void *mem );

	void removeElement( int vertexIndex );

	void clear()
	{
		m_data.clear();
		m_numElements = 0;
	}

	char type()
	{
		// should change on a per type basis
		return 0;
	}

	int numElements()
	{
		// should change on e per type basis
		return m_numElements;
	}

	int numComponents()
	{
		return m_numComponents;
	}

	int elementComponentType()
	{
		// should change on e per type basis
		//return GL_FLOAT;
		return m_componentType;
	}

	int elementComponentSize()
	{
		return m_componentSize;
	}

	void *getRawPointer()
	{
		// should change on e per type basis
		return m_data.m_data;
	}
	void *getRawPointer( int index )
	{
		// should change on e per type basis
		return &m_data.m_data[index*numComponents()*elementComponentSize()];
	}

	vector<unsigned char> m_data;
	char m_componentSize; // size in memory of a component of an element in byte
	int m_componentType;
	char m_numComponents; // number of components per element
	int m_numElements;
	unsigned int m_bufferId;
	char m_type; // tells which type (vector3d, float, vector2d, matrix, int)
};

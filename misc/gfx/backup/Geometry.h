#pragma once
#include "../sys/msys.h"
#include "../std/vector.h"
#include "Attribute.h"

// these definitions are used to map Attributeids to unique identifiers which will be used as index
#define ATTR_P            0
#define ATTR_N            1
#define ATTR_Cd           2
#define ATTR_W            3
#define ATTR_CATMULLT     4
#define ATTR_BONEWEIGHTS  5
#define ATTR_BONEINDICES  6
#define ATTR_UV           7

struct Shader;

struct Geometry
{
	Geometry();


	virtual void render( Shader *shader = 0 );
	void clear(); // removes all attributes and primitives
	Geometry *copy();




	//
	// manipulation
	//
	void splitVertex( int vertexIndex, int numCopies );
	void removeVertex( int vertexIndex );
	void autoWeld( float distance );



	//
	// Attribute management
	//
	void setVertexAttr( int attrIndex, int index, float x, float y, float z );
	void setAttr( int attrIndex, Attribute *vertexAttrib );
	Attribute *getAttr( int attrIndex );
	bool hasAttr( int attrIndex );

	void setPAttr( Attribute *posAttrib );
	Attribute *getPAttr();

	Attribute **m_attributes; // hashmap would be nice, but instead we will use the key as an index into a sparse array

	//
	// uniform management
	//
	void setUniform( const char *name, Attribute *uniform );
	Attribute *getUniform( int uniformIndex );
	vector<void *>     m_uniforms; // list of uniforms
	vector<const char *> m_uniformNames; // list of uniform names

	//
	// primitive management
	//

	int addPoint( int vId0 );
	int addLine( int vId0, int vId1 );
	int addTriangle( int vId0, int vId1, int vId2 );
	int addQuad( int vId0, int vId1, int vId2, int vId3 );
	void resetPrimitiveType( int newPrimitiveType );



	

	unsigned int                   m_indexBufferId;
	vector<int>                      m_indexBuffer;
	int                            m_numPrimitives;
	int                            m_numComponents;
	unsigned int                   m_primitiveType;
};






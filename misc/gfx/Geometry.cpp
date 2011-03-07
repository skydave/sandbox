#include "Geometry.h"
#include "Shader.h"
#define GEO_ATTRIBUTEHASH_COUNT 50

Geometry::Geometry()
{
	// by default we allow 50 different attributes to be attached. should be enough for us
	m_attributes = (Attribute **)mmalloc( GEO_ATTRIBUTEHASH_COUNT*sizeof(Attribute*) );
	for( int i=0; i<GEO_ATTRIBUTEHASH_COUNT;++i )m_attributes[i] = 0;

	// initialize indexbuffer
	oglGenBuffers(1, &m_indexBufferId);
}


void Geometry::render( Shader *shader )
{
	if(shader && shader->isOk())
	{
		shader->use();

		// iterate all active attributes
		for( int i=0; i<shader->m_activeAttributes.size(); ++i )
		{
			int attrIndex = shader->m_activeAttributes.m_data[i];
			// getAttr and bind(with index)
			if (hasAttr(attrIndex))
				getAttr(attrIndex)->bindAsAttribute(attrIndex);
		}

		// iterate all active uniforms
		for( int i=0; i<shader->m_activeUniforms.size(); ++i )
			// iterate all uniforms of the geometry
			for( int j=0; j<m_uniforms.size(); ++j )
			{
				char *t1 = (char *)shader->m_activeUniformNames.m_data[i];
				char *t2 = (char *)m_uniformNames.m_data[j];
				if( !strcmp( (char *)shader->m_activeUniformNames.m_data[i], m_uniformNames.m_data[j] ) )
					getUniform(j)->bindAsUniform( shader->m_activeUniforms.m_data[i] );
			}
	}else
		// bind default vertex attributes
		getPAttr()->bindAsAttribute(ATTR_P);


	// render primitives

	// ...using indexbufferobject (does not work on laptop)
	//oglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);
	//oglBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer.size(), m_indexBuffer.m_data, GL_STATIC_DRAW);
	//glDrawElements(m_primitiveType, m_indexBuffer.size(), GL_UNSIGNED_INT, 0);

	// ...works for now
	glDrawElements(m_primitiveType, m_indexBuffer.size(), GL_UNSIGNED_INT, m_indexBuffer.m_data);

	if(shader && shader->isOk())
	{
		// iterate all active attributes
		for( int i=0; i<shader->m_activeAttributes.size(); ++i )
		{
			int attrIndex = shader->m_activeAttributes.m_data[i];
			// getAttr and unbind(with index)
			if (hasAttr(attrIndex))
				getAttr(attrIndex)->unbindAsAttribute(attrIndex);
		}

		// disable shader
		oglUseProgram(0);
	}else
		// unbind default vertex attributes
		getPAttr()->unbindAsAttribute(ATTR_P);
}

//
// removes all attributes and primitives
//
void Geometry::clear()
{
	for( int i=0; i<GEO_ATTRIBUTEHASH_COUNT;++i )m_attributes[i] = 0;
	m_indexBuffer.clear();
	resetPrimitiveType( GL_POINTS );
}


//
//!!!!!!!!!!!!! we dont copy them now. skeleton needs reference
//
Geometry *Geometry::copy()
{
	Geometry *ngeo = new Geometry();

	// copy attributes
	for( int i=0; i<GEO_ATTRIBUTEHASH_COUNT;++i )
		if (m_attributes[i])
			ngeo->setAttr( i, m_attributes[i]->copy() );
	//
	// copy uniforms !!!!!!!!!!!!! we dont copy them now. skeleton needs reference
	//
	for( int i=0; i<m_uniforms.size();++i )
		ngeo->setUniform( m_uniformNames.m_data[i], (Attribute*)m_uniforms.m_data[i] );

	// copy primitives
	ngeo->m_primitiveType = m_primitiveType;
	ngeo->resetPrimitiveType( m_primitiveType );
	ngeo->m_indexBuffer.resize( m_indexBuffer.size() );
	msys_memcpy( ngeo->m_indexBuffer.m_data, m_indexBuffer.m_data, m_indexBuffer.size()*sizeof(int) );
	ngeo->m_numPrimitives = m_numPrimitives;

	return ngeo;
}






//
//
//
void Geometry::splitVertex( int vertexIndex, int numCopies )
{
	for( int i=0;i<numCopies-1;++i )
	{
		// create a new vertex by creating a new entry in all attributes (and copying values over)
		for( int j=0; j<GEO_ATTRIBUTEHASH_COUNT;++j )
			if (m_attributes[j])
				getAttr( j )->appendElement( getAttr( j )->getRawPointer(vertexIndex) );
	}
}


void Geometry::removeVertex( int vertexIndex )
{
	for( int j=0; j<GEO_ATTRIBUTEHASH_COUNT;++j )
		if (m_attributes[j])
			getAttr( j )->removeElement( vertexIndex );
	// rearrange indices of all primitives
	for( int i=0; i<m_indexBuffer.size(); ++i )
	{
		if( m_indexBuffer.m_data[i] > vertexIndex )
			--m_indexBuffer.m_data[i];
	}
}


void Geometry::autoWeld( float distance )
{
	int *toRemove = (int *)mmalloc( getPAttr()->numElements()*sizeof(int) );
	int numRemove = 0;
	// for each vertex pair
	for( int i=0; i<getPAttr()->numElements(); ++i )
		for( int j=i+1; j<getPAttr()->numElements(); ++j )
		{
			if( (math::distance( getPAttr()->getVec3f(i), getPAttr()->getVec3f(j) ) < distance) && (fabs(getPAttr()->getVec3f(j).x) < distance ) )
			{
				// remove vertex j - later
				toRemove[numRemove++] = j;

				// bend all prims to point to i instead of j
				for( int k = 0; k<m_indexBuffer.size(); ++k )
				{
					int t = m_indexBuffer.m_data[k];
					if(m_indexBuffer.m_data[k] == j)
						m_indexBuffer.m_data[k] = i;
				}
			}
		}

	//Attribute *t = new Attribute(3);

	//for( int i=0; i<getPAttr()->numElements(); ++i )
	//	t->appendElement(math::Vec3f(0.0f, 0.5f, 0.0f));

	for( int i=0; i<numRemove; ++i )
	{
		//printf( "removing %i\n", toRemove[i] );
		//math::Vec3f c( 1.0f, 0.0f, 0.0f );
		//t->setElement(toRemove[i] , &c);

		//math::Vec3f p = getPAttr()->getVec3f(toRemove[i]);
		//p.z += 1.0f;
		//getPAttr()->setElement( toRemove[i], &p );
		//t->setElement( 0, &c);
		removeVertex(toRemove[i]-i);
	}
	//setAttr( ATTR_Cd, t );


	mfree( toRemove );
}




void Geometry::setAttr( int attrIndex, Attribute *vertexAttrib )
{
	m_attributes[attrIndex] = vertexAttrib;
}

Attribute *Geometry::getAttr( int attrIndex )
{
	return m_attributes[attrIndex];
}

bool Geometry::hasAttr( int attrIndex )
{
	return (m_attributes[attrIndex] != 0);
}

void Geometry::setVertexAttr( int attrIndex, int index, float x, float y, float z )
{
	//p
}




void Geometry::setPAttr( Attribute *posAttrib )
{
	m_attributes[ATTR_P] = posAttrib;
}

Attribute *Geometry::getPAttr()
{
	return m_attributes[ATTR_P];
}




Attribute *Geometry::getUniform( int uniformIndex )
{
	return (Attribute *)m_uniforms.m_data[uniformIndex];
}

void Geometry::setUniform( const char *name, Attribute *uniform )
{
	m_uniformNames.push_back( name );
	m_uniforms.push_back( uniform );	
}

int Geometry::addPoint( int vId0 )
{
	if( m_primitiveType != GL_POINTS )
		resetPrimitiveType( GL_POINTS );
	m_indexBuffer.push_back(vId0);

	return m_numPrimitives++;
}

int Geometry::addLine( int vId0, int vId1 )
{
	if( m_primitiveType != GL_LINES )
		resetPrimitiveType( GL_LINES );
	m_indexBuffer.push_back(vId0);
	m_indexBuffer.push_back(vId1);

	return m_numPrimitives++;
}

int Geometry::addTriangle( int vId0, int vId1, int vId2 )
{
	if( m_primitiveType != GL_TRIANGLES )
		resetPrimitiveType( GL_TRIANGLES );
	m_indexBuffer.push_back(vId0);
	m_indexBuffer.push_back(vId1);
	m_indexBuffer.push_back(vId2);

	return m_numPrimitives++;
}

int Geometry::addQuad( int vId0, int vId1, int vId2, int vId3 )
{
	if( m_primitiveType != GL_QUADS )
		resetPrimitiveType( GL_QUADS );
	m_indexBuffer.push_back(vId0);
	m_indexBuffer.push_back(vId1);
	m_indexBuffer.push_back(vId2);
	m_indexBuffer.push_back(vId3);

	return m_numPrimitives++;
}


void Geometry::resetPrimitiveType( int newPrimitiveType )
{
	m_indexBuffer.clear();
	m_primitiveType = newPrimitiveType;
	switch(m_primitiveType)
	{
	case GL_POINTS: m_numComponents=1;break;
	case GL_LINES: m_numComponents=2;break;
	case GL_TRIANGLES: m_numComponents=3;break;
	case GL_QUADS: m_numComponents=4;break;
	};
}

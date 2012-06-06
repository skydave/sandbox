//
// very simple utility class for making drawing of debug stuff easier
//
#include "Visualizer.h"
#include <gfx/Context.h>







Visualizer::Visualizer() : m_currentColor(1.0f, 1.0f, 1.0f), m_currentPointSize(5.0f), m_numLines(0)
{
	m_points = base::Geometry::createPointGeometry();
	m_points->setAttr( "Cd", base::Attribute::createVec3f());
	m_points->setAttr( "S", base::Attribute::createFloat());
	m_lines = base::Geometry::createLineGeometry();
	m_lines->setAttr( "Cd", base::Attribute::createVec3f());

	m_lineShader = base::Shader::load( base::Path( SRC_PATH ) + "src/Visualizer.line" );
	m_pointShader = base::Shader::load( base::Path( SRC_PATH ) + "src/Visualizer.point" );
}


VisualizerPtr Visualizer::create()
{
	return VisualizerPtr( new Visualizer() );
}




void Visualizer::render()
{
	base::ContextPtr context = base::Context::current();

	// render lines
	context->render( m_lines, m_lineShader );

	// render point
	glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );
	glTexEnvf( GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE );
	glEnable( GL_POINT_SPRITE );
	context->render( m_points, m_pointShader );
	glDisable( GL_POINT_SPRITE );
}

// set current color (will affect all consecutive calls)
void Visualizer::color( float r, float g, float b)
{
	m_currentColor = math::Vec3f(r, g, b);
}


// sets current points size
void Visualizer::pointSize( float size )
{
	m_currentPointSize = size;
}


// add a point
void Visualizer::point( const math::Vec3f p0 )
{
	base::AttributePtr c, p, s;
	p = m_points->getAttr("P");
	c = m_points->getAttr("Cd");
	s = m_points->getAttr("S");
	int i = p->appendElement(p0);
	c->appendElement(m_currentColor);
	s->appendElement(m_currentPointSize);
	m_points->addPoint( i );
}


// add a line
Visualizer::Handle Visualizer::line( const math::Vec3f p0, const math::Vec3f &p1 )
{
	Handle h = m_numLines++;
	base::AttributePtr c, p;
	p = m_lines->getAttr("P");
	c = m_lines->getAttr("Cd");
	int i0 = p->appendElement(p0);
	int i1 = p->appendElement(p1);
	c->appendElement(m_currentColor);
	c->appendElement(m_currentColor);
	m_lines->addLine( i0, i1 );

	m_lineAttributeIndices[h] = i0;
	return h;
}

void Visualizer::line( Visualizer::Handle h, const math::Vec3f p0, const math::Vec3f &p1 )
{
	std::map<Handle, int>::iterator it = m_lineAttributeIndices.find(h);
	if( it != m_lineAttributeIndices.end() )
	{
		int attrIndex = it->second;
		base::AttributePtr p;
		p = m_lines->getAttr("P");
		p->set<math::Vec3f>( attrIndex, p0 );
		p->set<math::Vec3f>( attrIndex+1, p1 );
	}
}
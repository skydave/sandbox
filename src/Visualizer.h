//
// very simple utility class for making drawing of debug stuff easier
//
#pragma once
#include <util/shared_ptr.h>
#include <gfx/Geometry.h>
#include <gfx/Shader.h>



BASE_DECL_SMARTPTR_STRUCT(Visualizer);
struct Visualizer
{
	Visualizer();                                               // constructor
	static VisualizerPtr                              create();

	void                                              render();


	void                     color( float r, float g, float b); // set current color (will affect all consecutive calls)
	void                               pointSize( float size ); // sets current points size
	void                         point( const math::Vec3f p0 ); // add a point
	void   line( const math::Vec3f p0, const math::Vec3f &p1 ); // add a line







	base::GeometryPtr                                  m_lines;
	base::GeometryPtr                                 m_points;
	math::Vec3f                                 m_currentColor;
	float                                   m_currentPointSize;
	//base::GeometryPtr                                 m_text; 

	base::ShaderPtr                               m_lineShader;
	base::ShaderPtr                              m_pointShader;
};
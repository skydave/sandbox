//
// very simple utility class for making drawing of debug stuff easier
//
#pragma once
#include <map>
#include <util/shared_ptr.h>
#include <gfx/Geometry.h>
#include <gfx/Shader.h>



BASE_DECL_SMARTPTR_STRUCT(Visualizer);
struct Visualizer
{
	typedef int Handle;

	Visualizer();                                               // constructor
	static VisualizerPtr                              create();

	void                                              render();


	void                                                  color( float r, float g, float b); // set current color (will affect all consecutive calls)
	void                                                            pointSize( float size ); // sets current points size
	void                                                      point( const math::Vec3f p0 ); // add a point
	Handle                              line( const math::Vec3f p0, const math::Vec3f &p1 ); // add a line
	void                      line( Handle h, const math::Vec3f p0, const math::Vec3f &p1 ); // updates line








	base::GeometryPtr                                  m_lines;
	base::GeometryPtr                                 m_points;
	math::Vec3f                                 m_currentColor;
	float                                   m_currentPointSize;
	//base::GeometryPtr                                 m_text; 

	base::ShaderPtr                               m_lineShader;
	base::ShaderPtr                              m_pointShader;

	std::map<Handle, int>               m_lineAttributeIndices; // maps handles to primitives to their first index in their respective attribute lists
	int                                             m_numLines;
};
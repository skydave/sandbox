
#pragma once

#include <ops/Op.h>
#include <ops/Manager.h>
#include <gfx/Geometry.h>
#include <gfx/Shader.h>


//
// renders geometry
//
BASE_DECL_SMARTPTR(RenderGeoOp);
class RenderGeoOp : public base::ops::Op
{
public:
	RenderGeoOp() : base::ops::Op()
	{
	}

	virtual void execute()
	{
		// TODO: get shader
		// TODO: get geometry
		//glDepthMask( GL_FALSE );
		glDisable( GL_DEPTH_TEST );
		glDisable( GL_CULL_FACE );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		//glBlendFunc( GL_ONE, GL_ONE );
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		base::ops::Manager::context()->render( m_geo, m_shader );

		/*
		base::ops::Manager::context()->bind( m_shader, m_geo );

		glEnable( GL_POINT_SPRITE_ARB );


		float quadratic[] =  { 0.0f, 0.0f, 0.01f };

		glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic );

		float maxSize = 0.0f;

		glGetFloatv( GL_POINT_SIZE_MAX_ARB, &maxSize );

		glPointSize( maxSize );

		glPointParameterf( GL_POINT_SIZE_MAX, maxSize );

		glPointParameterf( GL_POINT_SIZE_MIN, 1.0f );

		glTexEnvf( GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE );
		//glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );

		glEnable( GL_POINT_SPRITE );

		glDrawElements(GL_POINTS, (GLsizei)m_geo->m_indexBuffer.size(), GL_UNSIGNED_INT, &m_geo->m_indexBuffer[0]);
		
		//AttributePtr  pos = geo->getAttr("P");
		//glBegin(GL_POINTS);
		//for(int i=0; i< pos->numElements(); ++i)
		//{
		//	glVertex3f( pos->get<math::Vec3f>(i).x, pos->get<math::Vec3f>(i).y, pos->get<math::Vec3f>(i).z );
		//}
		//glEnd();

		glDisable( GL_POINT_SPRITE );

		base::ops::Manager::context()->unbind( m_shader, m_geo );
		*/



		glDisable( GL_BLEND );
		//glDepthMask( GL_TRUE );
		glEnable( GL_DEPTH_TEST );
	}

	static RenderGeoOpPtr create()
	{
		return RenderGeoOpPtr( new RenderGeoOp() );
	}

//private:
	base::GeometryPtr              m_geo;
	base::ShaderPtr             m_shader;
};

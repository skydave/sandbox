#include "CurveGeometry.h"
#include "../math/Math.h"

CurveGeometry::CurveGeometry():m_sampling(100)
{
	pSampled = new Attribute();
}





void CurveGeometry::render()
{


	// we sample the curve everytime we render and whenever the geometry has changed
	// TODO: track if geo has changed
	/*
	pSampled->clear();





	getPAttr()->bind(0);

	//glDrawElements(GL_POINTS, 24, GL_UNSIGNED_BYTE, 0);
	//
	glDrawArrays(GL_POINTS, 0, getPAttr()->numElements());
	//topology->render();



	getPAttr()->unbind(0);
	*/

	glBegin( GL_LINE_STRIP );
	for(int i=0; i<m_sampling; ++i)
	{
		math::Vec3f sample;
		math::evalCatmullRom( (float *)getPAttr()->getRawPointer(), (float *)getAttr(ATTR_CATMULLT)->getRawPointer(), getPAttr()->numElements(), 3, (float)i / (float)m_sampling, &sample[0] );
		glVertex3fv(&sample[0]);
	}
	glEnd();
}

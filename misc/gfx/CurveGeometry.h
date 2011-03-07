#pragma once
#include "Geometry.h"

struct CurveGeometry : Geometry
{
	CurveGeometry();


	virtual void render();

	Attribute *pSampled; // contains the positions of all curve samples

	int m_sampling;
};

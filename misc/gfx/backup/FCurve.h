#pragma once
#include "../math/Math.h"

struct FCurve
{
	FCurve();
	FCurve( unsigned char *data ); // reads fcurve data from serialisation


	float eval( float time );

	int  m_numKeys; // number of keyframes
	float *m_values; // array of values
	float *m_t; // array of times


	bool isSpecial;
};

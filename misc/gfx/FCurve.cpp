#include "FCurve.h"
#include "../sys/msys.h"

//
//
//
FCurve::FCurve()
{
	isSpecial = false;
	m_numKeys = 0;
	m_values = 0;
	m_t = 0;
	defaultValue = 0.0f;
}

//
// reads fcurve data from serialisation
//
FCurve::FCurve( unsigned char *indata, float _defaultValue ) : defaultValue(_defaultValue)
{
	unsigned char* data  = indata;

	// read number of keys
	m_numKeys = data[0];
	float *times = (float*)&data[1];
	float *values = (float*)&times[m_numKeys];
	m_values = (float *)mmalloc( m_numKeys*sizeof(float) );
	//msys_memcpy( m_values, values, m_numKeys*sizeof(float) );
	m_t = (float *)mmalloc( m_numKeys*sizeof(float) );
	//msys_memcpy( m_t, times, m_numKeys*sizeof(float) );

	for( int i=0; i<m_numKeys; ++i )
	{
		float val = values[i];
		float tim = times[i];
		//float val2 = m_values[i];
		//printf( "value %i = %i         time = %i\n", i, val, tim );

		m_values[i] = val;
		m_t[i] = tim;
	}
}

//
//
//
float FCurve::eval( float time )
{
	//if( isSpecial )
	//		return msys_cosf(time);
	float value = defaultValue;
	//if (m_numKeys > 0)
	//	value = 40.0f;
	//	math::evalCatmullRom( m_values, m_t, m_numKeys, 1, time, &value );
	//if( isSpecial && (m_numKeys > 0) )
	if( m_numKeys > 0 )
		math::evalCatmullRom( m_values, m_t, m_numKeys, 1, time, &value );
		//return msys_cosf(time);

	return value;
}

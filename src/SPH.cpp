
#include "SPH.h"




// poly6 =====================================================

float W_poly6_h2;
float W_poly6_coeff;

void W_poly6_precompute( float supportRadius )
{
	W_poly6_h2 = supportRadius*supportRadius;
	W_poly6_coeff = 315.0f/( 64.0f*MATH_PIf*powf(supportRadius, 9.0f) );
}
float W_poly6( float distance )
{
	float t = distance*distance;
	if( t > W_poly6_h2 )
		return 0.0f;
	if( t < 1.192092896e-07f )
		t = 1.192092896e-07f;
	t = W_poly6_h2-t;
	return W_poly6_coeff*t*t*t;
}


// spiky =====================================================
float W_spiky_h;
float W_spiky_coeff;
float W_spiky_grad_coeff;


void W_spiky_precompute( float supportRadius )
{
	W_spiky_h = supportRadius;
	W_spiky_coeff = 15.0f/( MATH_PIf*powf(supportRadius, 6.0f) );
	W_spiky_grad_coeff = -45.0f / (MATH_PIf*powf(supportRadius, 6.0f));
}
float W_spiky( float distance )
{
	if( distance > W_spiky_h )
		return 0.0f;
	float t = W_spiky_h - distance;
	return W_spiky_coeff*t*t*t;
}
math::Vec3f gradW_spiky( float distance, math::Vec3f dir )
{
	float t = (W_spiky_h - distance );
	return W_spiky_grad_coeff*dir*(1.0f/distance)*t*t;
}
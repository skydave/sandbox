#pragma once
#include <math/Math.h>














// weighting functions =========================

//6th degree polynomial
void W_poly6_precompute( float supportRadius );
float W_poly6( float distance );


//spiky (gradient doesnt vanish near center)
void W_spiky_precompute( float supportRadius );
float W_spiky( float distance );
math::Vec3f gradW_spiky( float distance, math::Vec3f dir );
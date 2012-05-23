#include "SDF.h"


// SDF ======================================================
ScalarFieldPtr ScalarField::create()
{
	return ScalarFieldPtr( new ScalarField() );
}


// DOMAIN ===================================================
Domain::Domain() : bound(math::Vec3f(0.0f,0.0f,0.0f), math::Vec3f(1.0f,1.0f,1.0f)), width(10), height(10), depth(10)
{
}
Domain::Domain( math::Vec3f min, math::Vec3f max, int _width, int _height, int _depth ) : bound(min, max), width(_width), height(_height), depth(_depth)
{
}





/*---------------------------------------------------------------------

defines topology information for an element. Shape is also responsible
for computing the shapefunctions and its derivative.

----------------------------------------------------------------------*/
#pragma once
#include "Math.h"
#include "Element.h"

//
//
//
class Tetra4 : public Element
{
public:
	Tetra4();                                                                                   // constructor
	virtual math::Matrix44f                                                      getJacobian(); // returns the Jacobian which is responsible for transforming the reference element into global or material space
	virtual void                                                             computeJacobian(); // computes the Jacobian of the current element
	virtual math::Vec3f              getReferencePosition( const math::Vec3f &globalPosition ); // transforms the given position from global into reference space
	virtual math::Vec3f              getGlobalPosition( const math::Vec3f &referencePosition ); // transforms the given position from reference into global space
	virtual float       computeShapeFunction( size_t node, const math::Vec3f &globalPosition ); // computes the shape function for the given global-space coordinate (must be within element boundaries!)
	virtual bool                                 encloses( const math::Vec3f &globalPosition ); // returns true if the given point lies within the volume of this element


private:

	math::Matrix44f                                                                 m_jacobian;
	math::Matrix44f                                                          m_inverseJacobian;
};
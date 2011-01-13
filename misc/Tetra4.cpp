/*---------------------------------------------------------------------

defines topology information for an element. Shape is also responsible
for computing the shapefunctions and its derivative.

----------------------------------------------------------------------*/
#include "Tetra4.h"
#include "Node.h"






//
// constructor
//
Tetra4::Tetra4()
{
	nodes.resize( 4 );
}

//
// computes the shape function for the given global-space
// coordinate (must be within element boundaries!)
// linear shape function
//
float Tetra4::computeShapeFunction( size_t node, const math::Vec3f &globalPosition )
{
	// first transform position into reference space
	math::Vec3f referencePosition = getReferencePosition(globalPosition);

	// depending on the index of the node (topologic relation) we evaluate
	// the form function which is defined from the shape of a tehadron
	if( node == 0 )
		return 1.0f - referencePosition[0] - referencePosition[1] - referencePosition[2];
	if( node == 1 )
		return referencePosition[0];
	if( node == 2 )
		return referencePosition[1];
	if( node == 3 )
		return referencePosition[2];

	return 0.0f;
}

//
// computes the Jacobian which is responsible for transforming
// the reference element into global or material space
//
math::Matrix44f Tetra4::getJacobian()
{
	return m_jacobian;
}

//
// computes the Jacobian of the current element
//
void Tetra4::computeJacobian()
{
	m_jacobian = math::Matrix44f::Identity();

	// compute right vector
	m_jacobian.setRight( nodes[1]->getPosition() - nodes[0]->getPosition() );
	// compute up vector
	m_jacobian.setUp( nodes[3]->getPosition() - nodes[0]->getPosition() );
	// compute forward vector
	m_jacobian.setDir( nodes[2]->getPosition() - nodes[0]->getPosition() );

	// compute inverse 
	m_inverseJacobian = invert( m_jacobian );
}

//
// transforms the given position from global into reference space
//
math::Vec3f Tetra4::getReferencePosition( const math::Vec3f &globalPosition )
{
	return math::transform( globalPosition - nodes[0]->getPosition(), m_inverseJacobian );
}

//
// transforms the given position from reference into global space
//
math::Vec3f Tetra4::getGlobalPosition( const math::Vec3f &referencePosition )
{
	return math::transform( referencePosition, m_jacobian ) + nodes[0]->getPosition();
}

//
// returns true if the given point lies within the volume of this element
//
bool Tetra4::encloses( const math::Vec3f &globalPosition )
{
	// first transform position into reference space
	math::Vec3f referencePosition = getReferencePosition(globalPosition);

	// now check wether the coordinates are in valid ranges
	if( (referencePosition.x >= 0.0f) && (referencePosition.x <= 1.0f)&&
		(referencePosition.y >= 0.0f) && (referencePosition.y <= 1.0f)&&
		(referencePosition.z >= 0.0f) && (referencePosition.z <= 1.0f - referencePosition.x - referencePosition.y))
		return true;
	return false;
}
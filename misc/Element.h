/*---------------------------------------------------------------------

an element is an abstract collection of nodes which is responsible for
different aspects of problem solving (computing loads, masses etc.)
Topology-information (which defines what kind of element it
is (triangle, tetraheder etc.)) is defined in the referenced shape

----------------------------------------------------------------------*/
#pragma once
#include <vector>
#include "math/Math.h"

class Node;
class Shape;

//
//
//
class Element
{
public:
	void                                                   setNode( int localIndex, Node *node );  // will set the node of the local index
	Node                                                              *getNode( int localIndex );  // will return the node of the given local index

	virtual math::Matrix44f                                                      getJacobian()=0; // returns the Jacobian which is responsible for transforming the reference element into global or material space
	virtual void                                                             computeJacobian()=0; // computes the Jacobian of the current element
	virtual math::Vec3f              getReferencePosition( const math::Vec3f &globalPosition )=0; // transforms the given position from global into reference space
	virtual math::Vec3f              getGlobalPosition( const math::Vec3f &referencePosition )=0; // transforms the given position from reference into global space
	virtual float       computeShapeFunction( size_t node, const math::Vec3f &globalPosition )=0; // computes the shape function for the given global-space coordinate (must be within element boundaries!)
	virtual bool                                 encloses( const math::Vec3f &globalPosition )=0; // returns true if the given point lies within the volume of this element

	size_t                                                                 getMaxEquationIndex(); // returns the highest index into the system of linear equations

protected:
	std::vector<Node *>                                                                    nodes;
};
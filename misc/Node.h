/*---------------------------------------------------------------------

3dimensional FEM-Node

----------------------------------------------------------------------*/
#pragma once
#include <vector>
#include "math/Math.h"
#include "DOF.h"

class Domain;

//
//
//
class Node
{
public:
	math::Vec3f                                      getPosition(); // returns the position within material coordinates
	void               setPosition( const math::Vec3f &_position );

	size_t                                           getDOFCount(); // returns number of dofs
	DOF                                    *getDOF( size_t index ); // returns the index-specified dof
	DOF *                                                 addDOF(); // adds a new degree of freedom to the node - returns the new created dof

	size_t                                   getMaxEquationIndex(); // returns the highest index into the system of linear equations

	void                              setDomain( Domain *_domain ); // sets the Domain this node belongs to - this will update the equation-indicees of all dofs
	Domain                                            *getDomain(); // returns the domain where this node belongs to



private:
	math::Vec3f                                           position; // position of the node

	std::vector<DOF *>                                        dofs; // list of degrees of freedoms (unkowns/bcs for this node)

	Domain                                                 *domain; // reference to the domain this node belongs to
};
/*---------------------------------------------------------------------

The Degree of Freedom class represents one unknown within the final
system if linear equations. The DOF class is introduces because it
makes management of unknowns and their position within the final
matrix and the management of boundary conditions more easy.
For example if we had a 3-dimensional linear elasticity problem then
one node would have 3 degrees of freedom (boundary conditions not
taken into account). One DOF for each node displacementcomponent (x,y,z).

----------------------------------------------------------------------*/
#include "DOF.h"
#include "Domain.h"
#include "LinearSystem.h"

//
// constructor
//
DOF::DOF( Node *_node )
{
	dirichletBoundaryCondition = false;
	equationIndex = 0;
	node = _node;
}


//
// returns the equation number where this dof points to
//
size_t DOF::getEquationIndex()
{
	return equationIndex;
}

//
// sets the equation number where this dof points to
//
void DOF::assignEquationIndex( const size_t &index )
{
	equationIndex = index;
}

//
// returns true if the dof is not constrained by a boundary condition
//
bool DOF::isUnknown()
{
	return true;
}

//
// after the linear system has been solved over the domain the dof represents one component of the solution vector
//
float DOF::getSolution()
{
	return node->getDomain()->getLinearSystem()->getSolution( equationIndex );
}

//
// returns true if this dof is constrained by a boundary condition
//
bool DOF::hasBoundaryCondition()
{
	return dirichletBoundaryCondition;
}

//
// will constrain this DOF by the given boundary condition value
//
void DOF::assignDirichletBoundaryCondition( const float &value )
{
	dirichletBoundaryCondition = true;
	boundaryConditionValue = value;
}


/*---------------------------------------------------------------------

The Degree of Freedom class represents one unknown within the final
system of linear equations. The DOF class is introduced because it
makes management of unknowns and their position within the final
matrix and the management of boundary conditions more easy.
For example if we had a 3-dimensional linear elasticity problem then
one node would have 3 degrees of freedom (boundary conditions not
taken into account). One DOF for each node displacementcomponent (x,y,z).

----------------------------------------------------------------------*/
#pragma once
#include <vector>


class Node;

//
//
//
class DOF
{
public:
	DOF( Node *_node );                                                // constructor

	size_t                                        getEquationIndex(); // returns the equation number where this dof points to
	void                  assignEquationIndex( const size_t &index ); // sets the equation number where this dof points to

	bool                                                 isUnknown(); // returns true if the dof is not constrained by a boundary condition

	float                                              getSolution(); // after the linear system has been solved over the domain the dof represents one component of the solution vector

	bool                                      hasBoundaryCondition(); // returns true if this dof is constrained by a boundary condition
	void      assignDirichletBoundaryCondition( const float &value ); // will constrain this DOF by the given boundary condition value
private:
	size_t                                             equationIndex; // index into the final system of linear equations
	Node                                                       *node; // reference to the node where this dof belongs to


	// currently onle Dirichlet boundary conditions are supported
	bool                                  dirichletBoundaryCondition;
	float                                     boundaryConditionValue;
};
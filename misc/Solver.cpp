/*---------------------------------------------------------------------

solver

----------------------------------------------------------------------*/
#include "Solver.h"
#include "LinearSystem.h"




//
//
//
void Solver::assembleMatrix()
{
	// iterate over all elements of the domain and build the elements contributions to the global matrix
	//for(  )
}



//
//
//
void Solver::solve( Domain *domain )
{
	// assemble global matrix and loadvector
	// for each element within the domain
	{
		// assemble element matrix

		// assemble element load vector
	}

	// apply boundary conditions
	// iterate over each dof of the problem
	{
		// handle the various types of boundary conditions
		// dirichlet:
		// zero out column and rows
		// set coefficient at i,j to 1
		// set the right side of the dof to the value of the boundary condition
	}

	// solve the linear system
	domain->getLinearSystem()->solve();

	// when asked for the solution value, the dofs will access the linearsystem of the domain
	// they belong to and retrieve the solution value from their equation index
}
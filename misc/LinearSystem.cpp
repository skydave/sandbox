/*---------------------------------------------------------------------

This class is a handy container for the coefficient-matrix A, the
rightside vector b and the vector of the solution x.

----------------------------------------------------------------------*/
#include "LinearSystem.h"
#include "Domain.h"



//
// constructor
//
LinearSystem::LinearSystem()
{
	equationCount = 0;
}

//
// will adjust the size of the matrix A, the vectors b and x to fit to the given problem
//
void setupFor( Domain *domain )
{
}

//
// this returns the index of the next free(unassigned) equation
//
size_t LinearSystem::getNextEquationIndex()
{
	return equationCount++;
}


//
// solves the linear equation system
//
void LinearSystem::solve()
{
	// convert the dense format into csparse packed column vector format
	//...
	// solve using csparse
	//...
	// convert the result (solutionvector x) back into a dense vector
	//...
	// done
}

//
// returns the index-specified component of the solution vector
//
float LinearSystem::getSolution( size_t equationIndex )
{
	return x[equationIndex];
}
/*---------------------------------------------------------------------

This class is a handy container for the coefficient-matrix A, the
rightside vector b and the vector of the solution x.

----------------------------------------------------------------------*/
#pragma once
#include <vector>
#include "MatrixNMf.h"
#include "csparse/csparse.h"

class Domain;

//
// Ax = b
//
class LinearSystem
{
public:
	LinearSystem();                                                  // constructor

	size_t                                   getNextEquationIndex(); // this returns the index of the next free(unassigned) equation

	void                                 setupFor( Domain *domain ); // will adjust the size of the matrix A, the vectors b and x to fit to the given problem

	void                                                    solve(); // solves the linear equation system

	float                       getSolution( size_t equationIndex ); // returns the index-specified component of the solution vector



private:
	MatrixNMf                                                     A; // matrix of coefficients
	std::vector<float>                                            x; // solution vector
	std::vector<float>                                            b; // right-hand-side
	size_t                                            equationCount;
};
/*---------------------------------------------------------------------

solver

----------------------------------------------------------------------*/
#pragma once
#include "Domain.h"

//
//
//
class Solver
{
public:
	void solve( Domain *domain );

private:

	void assembleMatrix();
};
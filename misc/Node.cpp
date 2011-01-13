/*---------------------------------------------------------------------

3dimensional FEM-Node

----------------------------------------------------------------------*/
#include "Node.h"
#include "Domain.h"
#include "LinearSystem.h"

//
// returns the position within material coordinates
//
math::Vec3f Node::getPosition()
{
	return position;
}


//
//
//
void Node::setPosition( const math::Vec3f &_position )
{
	position = _position;
}


//
// returns number of dofs
//
size_t Node::getDOFCount()
{
	return dofs.size();
}

//
// returns the index-specified dof
//
DOF *Node::getDOF( size_t index )
{
	return dofs[index];
}

//
// adds a new degree of freedom to the node
//
DOF *Node::addDOF()
{
	dofs.push_back( new DOF(this) );
	DOF *newDOF = dofs.back();
	domain->registerDOF( newDOF );
	return newDOF;
}

//
// returns the highest index into the system of linear equations
//
size_t Node::getMaxEquationIndex()
{
	size_t index = 0;
	// iterate over all dofs and max out their equationindex
	for( std::vector<DOF *>::iterator it = dofs.begin(); it != dofs.end(); ++it )
		index = std::max( index, (*it)->getEquationIndex() );
	return index;
}

//
// sets the Domain this node belongs to.
// This will update the equation-indicees of all dofs
//
void Node::setDomain( Domain *_domain )
{
	domain = _domain;

	// iterate over all dofs of the node and (re)assign an equation index
	for( std::vector<DOF *>::iterator it = dofs.begin(); it != dofs.end(); ++it )
	{
		DOF *dof = *it;

		if( dof->isUnknown() )
			dof->assignEquationIndex( domain->getLinearSystem()->getNextEquationIndex() );
	}
}

//
// returns the domain where this node belongs to
//
Domain *Node::getDomain()
{
	return domain;
}
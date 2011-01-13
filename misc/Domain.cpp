/*---------------------------------------------------------------------

the Domain class represents a simulation domain which is discretized by
a set of elements (e.g. tetrahedrals)

----------------------------------------------------------------------*/
#include "Domain.h"
#include "LinearSystem.h"









//
//
//
void Domain::addNode( Node *node )
{
	nodes.push_back( node );
	node->setDomain( this );
}

//
// retreive the node with the given global index
//
Node *Domain::getNode( size_t index )
{
	return nodes[index];
}

//
// returns the number of nodes within the domain
//
size_t Domain::getNodeCount()
{
	return nodes.size();
}

//
// add a element to the domain
//
void Domain::addElement( Element *element )
{
	elements.push_back( element );
}

//
// retreive the element with the given index
//
Element *Domain::getElement( size_t index )
{
	return elements[index];
}

//
// returns the number of elements within the domain
//
size_t Domain::getElementCount()
{
	return elements.size();
}


//
// this will be called by nodes of the domain and add the dof reference to the list of all dofs
//
void Domain::registerDOF( DOF *dof )
{
	dofs.push_back( dof );
	dof->assignEquationIndex( linearSystem->getNextEquationIndex() );
}

//
//
//
void Domain::addSurfaceElement(size_t i1, size_t i2, size_t i3)
{
	surface.push_back( Triangle() );

	surface.back().indicees[0] = i1;
	surface.back().indicees[1] = i2;
	surface.back().indicees[2] = i3;

	// compute normal
	surface.back().normal = math::normalize( math::crossProduct( nodes[i2]->getPosition() - nodes[i1]->getPosition(), nodes[i3]->getPosition() - nodes[i1]->getPosition() ) );
}

//
// returns the material which is assigned with this domain
//
Material *Domain::getMaterial()
{
	return material;
}

//
// sets the material which is assigned with this domain
//
void Domain::setMaterial( Material *_material )
{
	material = _material;
}


LinearSystem *Domain::getLinearSystem()
{
	return linearSystem;
}

//
// sets the system of linear equations
//
void Domain::setLinearSystem( LinearSystem *_linearSystem )
{
	linearSystem = _linearSystem;
}
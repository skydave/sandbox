/*---------------------------------------------------------------------

an element is an abstract collection of nodes which is responsible for
different aspects of problem solving (computing loads, masses etc.)
Topology-information (which defines what kind of element it
is (triangle, tetraheder etc.)) is defined in the referenced shape

----------------------------------------------------------------------*/
#include "Element.h"
#include "Node.h"










//
// will set the node of the local index
//
void Element::setNode( int localIndex, Node *node )
{
	nodes[localIndex] = node;
}

//
// will return the node of the given local index
//
Node *Element::getNode( int localIndex )
{
	return nodes[ localIndex ];
}

//
// returns the highest index into the system of linear equations
//
size_t Element::getMaxEquationIndex()
{
	size_t index = 0;

	for( std::vector<Node *>::iterator it = nodes.begin(); it != nodes.end(); ++it )
		index = std::max( index, (*it)->getMaxEquationIndex() );

	return index;
}
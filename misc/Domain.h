/*---------------------------------------------------------------------

the Domain class represents a simulation domain which is discretized by
a set of elements (e.g. tetrahedrals)

----------------------------------------------------------------------*/
#pragma once
#include <vector>
#include "Material.h"
#include "Element.h"
#include "Node.h"

class LinearSystem;

//
// FEM container
//
class Domain
{
	friend class Node;
public:

	void                                                                   addNode( Node *node );  // add a node to the domain
	Node                                                                *getNode( size_t index );  // retreive the node with the given global index
	size_t                                                                        getNodeCount();  // returns the number of nodes

	void                                                          addElement( Element *element );  // add a element to the domain
	Element                                                          *getElement( size_t index );  // retreive the element with the given index
	size_t                                                                     getElementCount();  // returns the number of elements within the domain

	Material                                                                      *getMaterial();  // returns the material which is assigned with this domain
	void                                                       setMaterial( Material *material );  // sets the material which is assigned with this domain

	LinearSystem                                                              *getLinearSystem();
	void                                          setLinearSystem( LinearSystem* _linearSystem ); // sets the system of linear equations



// Temp: the following methods are for the visual representation of the domain and actually dont belong
// to the FEM framework
	struct Triangle{size_t indicees[3];math::Vec3f normal;};
	std::vector<Triangle>         surface;
	void                    addSurfaceElement( size_t i1,  size_t i2, size_t i3 );

private:
	void                                                                registerDOF( DOF *dof ); // this will be called by nodes of the domain and add the dof reference to the list of all dofs

	std::vector<Element *>                                                             elements;
	std::vector<Node *>                                                                   nodes;
	std::vector<DOF *>                                                                     dofs; // all dofs within the problem domain

	Material                                                                          *material;

	LinearSystem                                                                  *linearSystem; // reference to the linear System of equations which will be used to solve the problem of the domain
};
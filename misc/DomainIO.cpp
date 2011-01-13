/*---------------------------------------------------------------------

The DomainIO class is a utitlity class for input and output of Domain
classes - for loading and saving domains into a variety of file formats.

----------------------------------------------------------------------*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include "DomainIO.h"
#include "Domain.h"
#include "Tetra4.h"
#include "LinearSystem.h"
#include "util/StringManip.h"




//
// tvm(Tetrahedral Volume Mesh) is the "neutral format" exported by NetGen
//
Domain *DomainIO::importFromTVM( const std::string &fileName )
{
	Domain *domain = 0;


	// try to open a text file stream
	std::ifstream file;
	file.open( fileName.c_str(), std::ios::in );

	// file open?
	if( !file )
		// error file could not be opened
		return 0;

	printf( "reading file %s\n", fileName.c_str() );

	// file exists - create a domain class
	domain = new Domain();
	domain->setMaterial( new Material() );
	domain->setLinearSystem( new LinearSystem() );

	// now read the file
	std::string                   line;
	std::vector<std::string> fragments;
	unsigned int         nodeCount = 0;
	unsigned int      elementCount = 0;
	unsigned int      surfaceCount = 0;

	// read the first line (nodecount)
	std::getline( file, line, '\n' );
	std::istringstream( line ) >> nodeCount;

	// for each node:
	for( unsigned int i=0; i<nodeCount; ++i )
	{
		Node *node = new Node();

		// read the line within the file which defines node position
		std::getline( file, line, '\n' );
		util::splitString( line, fragments, " " );

		math::Vec3f pos;
		std::istringstream( fragments[0] ) >> pos.x;
		std::istringstream( fragments[1] ) >> pos.y;
		std::istringstream( fragments[2] ) >> pos.z;

		node->setPosition( pos );

		domain->addNode( node );
	}

	// read the next line (elementcount)
	std::getline( file, line, '\n' );
	std::istringstream( line ) >> elementCount;

	// for each element:
	for( unsigned int i=0; i<elementCount; ++i )
	{
		Tetra4 *element = new Tetra4();

		int materialIndex = 1;
		int  nodeIndicees[4];

		// read the line within the file which defines element
		std::getline( file, line, '\n' );
		util::splitString( line, fragments, " " );

		// read material index
		std::istringstream( fragments[0] ) >> materialIndex;

		// read 4 node indicees
		std::istringstream( fragments[1] ) >> nodeIndicees[0];
		std::istringstream( fragments[2] ) >> nodeIndicees[1];
		std::istringstream( fragments[3] ) >> nodeIndicees[2];
		std::istringstream( fragments[4] ) >> nodeIndicees[3];

		element->setNode( 0, domain->getNode( nodeIndicees[0] - 1 ) );
		element->setNode( 1, domain->getNode( nodeIndicees[1] - 1 ) );
		element->setNode( 2, domain->getNode( nodeIndicees[2] - 1 ) );
		element->setNode( 3, domain->getNode( nodeIndicees[3] - 1 ) );

		element->computeJacobian();

		domain->addElement( element );
	}

	// read the next line (number of Tetrahedral-faces (Triangles) which belong to the surface/boundary of the domain and
	// will be used for a visual representation)
	std::getline( file, line, '\n' );
	std::istringstream( line ) >> surfaceCount;

	// for each surface element:
	for( unsigned int i=0; i<surfaceCount; ++i )
	{
		int materialIndex = 1;
		int   nodeIndicees[3];

		// read the line within the file which defines element
		std::getline( file, line, '\n' );
		util::splitString( line, fragments, " " );

		// read material index
		std::istringstream( fragments[0] ) >> materialIndex;

		// read 3 nodex indicees
		std::istringstream( fragments[1] ) >> nodeIndicees[0];
		std::istringstream( fragments[2] ) >> nodeIndicees[1];
		std::istringstream( fragments[3] ) >> nodeIndicees[2];

		domain->addSurfaceElement( nodeIndicees[0] - 1, nodeIndicees[1] - 1, nodeIndicees[2] - 1 );
	}

	file.close();

	return domain;
}
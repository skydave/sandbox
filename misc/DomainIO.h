/*---------------------------------------------------------------------

The DomainIO class is a utitlity class for input and output of Domain
classes - for loading and saving domains into a variety of file formats.

----------------------------------------------------------------------*/
#pragma once
#include <string>

class Domain;



class DomainIO
{
public:
	static Domain *importFromTVM( const std::string &fileName ); // tvm(Tetrahedral Volume Mesh) is the "neutral format" exported by NetGen
};
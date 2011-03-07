#include "gfx.h"

#include "../sys/msys.h"






Geometry *geogen_pointCloud()
{
	Geometry *result = new Geometry();

	Attribute *positions = new Attribute();

	positions->appendElement( math::Vec3f(0.0f,0.0f,0.0f) );
	positions->appendElement( math::Vec3f(.5f,0.0f,0.0f) );
	positions->appendElement( math::Vec3f(-.5f,0.0f,0.0f) );

	result->setPAttr(positions);

	//result->addPoint( 0 );
	//result->addPoint( 1 );
	//result->addPoint( 2 );

	return result;
}


Geometry *geogen_circle()
{
	CurveGeometry *result = new CurveGeometry();

	Attribute *positions = new Attribute();
	Attribute *catmullrom_t = new Attribute();

	positions->appendElement( math::Vec3f(0.0f,0.0f,0.0f) );
	catmullrom_t->appendElement( 0.0f );
	positions->appendElement( math::Vec3f(0.0f,0.5f,0.0f) );
	catmullrom_t->appendElement( 0.5f );
	positions->appendElement( math::Vec3f(0.0f,-0.5f,0.0f) );
	catmullrom_t->appendElement( 0.75f );
	positions->appendElement( math::Vec3f(-0.5f,-0.5f,0.0f) );
	catmullrom_t->appendElement( 1.0f );

	result->setPAttr(positions);
	result->setAttr( ATTR_CATMULLT, catmullrom_t );

	return result;
}

Geometry *geogen_quad()
{
	Geometry *result = new Geometry();

	Attribute *positions = new Attribute();
	Attribute *uvs = new Attribute(2);

	positions->appendElement( math::Vec3f(-1.0f,-1.0f,0.0f) );
	uvs->appendElement( .0f,.0f );
	positions->appendElement( math::Vec3f(-1.0f,1.0f,0.0f) );
	uvs->appendElement( .0f,1.0f );
	positions->appendElement( math::Vec3f(1.0f,1.0f,0.0f) );
	uvs->appendElement( 1.0f,1.0f );
	positions->appendElement( math::Vec3f(1.0f,-1.0f,0.0f) );
	uvs->appendElement( 1.0f,.0f );

	result->setPAttr(positions);
	result->setAttr( ATTR_UV, uvs );

	result->addQuad( 0, 1, 2, 3 );


	return result;
}

Geometry *geogen_quadtmp()
{
	Geometry *result = new Geometry();

	Attribute *positions = new Attribute();
	Attribute *uvs = new Attribute(2);

	positions->appendElement( math::Vec3f(0.0f,-1.0f,0.0f) );
	uvs->appendElement( .0f,.0f );
	positions->appendElement( math::Vec3f(0.0f,1.0f,0.0f) );
	uvs->appendElement( .0f,1.0f );
	positions->appendElement( math::Vec3f(1.0f,1.0f,0.0f) );
	uvs->appendElement( 1.0f,1.0f );
	positions->appendElement( math::Vec3f(1.0f,-1.0f,0.0f) );
	uvs->appendElement( 1.0f,.0f );

	result->setPAttr(positions);
	result->setAttr( ATTR_UV, uvs );

	result->addQuad( 0, 1, 2, 3 );


	return result;
}


Geometry *geogen_grid( int xres, int zres )
{
	Geometry *result = new Geometry();

	Attribute *positions = new Attribute();
	result->setPAttr(positions);

	Attribute *uvs = new Attribute(2);
	result->setAttr( ATTR_UV, uvs );

	
	for( int j=0; j<zres; ++j )
		for( int i=0; i<xres; ++i )
		{
			float u = i/(float)(xres-1);
			float v = j/(float)(zres-1);
			positions->appendElement( math::Vec3f(u-0.5f,0.0f,v-0.5f) );
			uvs->appendElement( u, v );
		}
	for( int j=0; j<zres-1; ++j )
		for( int i=0; i<xres-1; ++i )
		{
			int vo = (j*xres);
			result->addTriangle( vo+i, vo+i+1, vo+xres+i+1 );
			result->addTriangle( vo+i, vo+xres+i+1, vo+xres+i );

		}


	return result;
}


Geometry *geogen_sphere( int uSubdivisions, int vSubdivisions )
{
	float dPhi = MATH_2PIf/uSubdivisions;
	float dTheta = MATH_PIf/vSubdivisions;
	float theta, phi;

	Geometry *result = new Geometry();

	Attribute *positions = new Attribute();
	result->setPAttr(positions);

	// y
	for (theta=MATH_PIf/2.0f+dTheta;theta<=(3.0f*MATH_PIf)/2.0f-dTheta;theta+=dTheta)
	{
		math::Vec3f p;
		float y = sin(theta);
		// x-z
		for (phi=0;phi<MATH_2PIf;phi+=dPhi)
		{
			p.x = cos(theta) * cos(phi);
			p.y = y;
			p.z = cos(theta) * sin(phi);

			positions->appendElement( p );
		}
	}

	// add faces
	for( int j=0; j<vSubdivisions-3;++j )
	{
		int offset = j*(uSubdivisions);
		int i = 0;
		for( i=0; i<uSubdivisions-1; ++i )
		{
			result->addTriangle(offset+i,offset+i+1,offset+i + uSubdivisions);
			result->addTriangle(offset+i + uSubdivisions,offset+i+1,offset+i+uSubdivisions+1);
		}
		result->addTriangle(offset+i,offset+0,offset+i + uSubdivisions);
		result->addTriangle(offset+i + uSubdivisions,offset,offset + uSubdivisions);
	}
	int pole1 = positions->appendElement( 0.0f, 1.0f, 0.0f );
	int pole2 = positions->appendElement( 0.0f, -1.0f, 0.0f );
	for( int i=0; i<uSubdivisions-1; ++i )
	{
		result->addTriangle(pole1, i+1, i);
		result->addTriangle(pole2, uSubdivisions*(vSubdivisions-3)+i, uSubdivisions*(vSubdivisions-3)+i+1);
	}
	result->addTriangle(pole1, 0, uSubdivisions-1);
	result->addTriangle(pole2, uSubdivisions*(vSubdivisions-2)-1, uSubdivisions*(vSubdivisions-3));

	return result;
}

Geometry *geogen_cube()
{
	Geometry *result = new Geometry();

	Attribute *positions = new Attribute();

	positions->appendElement( math::Vec3f(-.5f,-.5f,0.5f) );
	positions->appendElement( math::Vec3f(-.5f,0.5f,0.5f) );
	positions->appendElement( math::Vec3f(.5f,.5f,0.5f) );
	positions->appendElement( math::Vec3f(.5f,-.5f,0.5f) );

	positions->appendElement( math::Vec3f(-.5f,-.5f,-0.5f) );
	positions->appendElement( math::Vec3f(-.5f,0.5f,-0.5f) );
	positions->appendElement( math::Vec3f(.5f,.5f,-0.5f) );
	positions->appendElement( math::Vec3f(.5f,-.5f,-0.5f) );

	result->setPAttr(positions);

	result->addQuad( 0, 1, 2, 3 );
	result->addQuad( 7, 6, 5, 4 );
	result->addQuad( 3, 2, 6, 7 );
	result->addQuad( 0, 4, 5, 1 );
	result->addQuad( 2, 1, 5, 6 );
	result->addQuad( 0, 3, 7, 4 );


	return result;
}
//
// extracts meshdata out of a binary stream
//
Geometry *geogen_mesh( unsigned char* _points, int numPoints, unsigned char *_indices, int numFaces )
{
	Geometry *result = new Geometry();
	Attribute *positions = new Attribute();
	math::Vec3f bbMin, size;

	float *header = (float *)_points;
	bbMin.x = *header;header++;
	bbMin.y = *header;header++;
	bbMin.z = *header;header++;
	size.x = *header;header++;
	size.y = *header;header++;
	size.z = *header;header++;

	unsigned char* points = (unsigned char *)header;


	// add vertices
	for( int i=0; i<numPoints; ++i )
	{
		float x,y,z;
		x = (float)points[i]/255.0f;
		y = (float)points[i+numPoints]/255.0f;
		z = (float)points[i+numPoints*2]/255.0f;
		positions->appendElement( math::Vec3f((x*size.x)+bbMin.x,(y*size.y)+bbMin.y,(z*size.z)+bbMin.z) );
		//float xc = (x*size.x)+bbMin.x;
		//if(xc<0)
		//	xc = 0.0f;
		//positions->appendElement( math::Vec3f(xc,(y*size.y)+bbMin.y,(z*size.z)+bbMin.z) );
	}
	result->setPAttr(positions);



	// add triangles
	//for( int i=0; i<numFaces; ++i )
	//	result->addTriangle( _indices[i*3+2], _indices[i*3+1], _indices[i*3+0] );
	int *indices = (int *) _indices;
	for( int i=0; i<numFaces; ++i )
		result->addQuad( indices[i*4+3], indices[i*4+2], indices[i*4+1], indices[i*4+0] );
		//result->addQuad( indices[i*4+0], indices[i*4+1], indices[i*4+2], indices[i*4+3] );

	return result;
}




void addNeighbour( int v, int n, int *neighbours )
{
	while( *neighbours != -1 )
		if( *neighbours == n )
			return;
		else
			++neighbours;
	*neighbours = n;
}


void apply_skin( Geometry *geo, Skeleton *skel, int smoothIterations )
{
	float boneInfluenceRadius = 1.5f;

	// data which we will add to the geometry
	Attribute *boneWeightsAttr = new Attribute();
	Attribute *boneIndicesAttr = new Attribute(3);
	Attribute *boneDistancesAttr = new Attribute(3);
	Attribute *boneMatricesUniform = new Attribute(16);


	//
	// for each vertex compute boneweights of the 3 closest bones
	//
	for( int i=0; i<geo->getPAttr()->numElements(); ++i )
	{
		// find 3 closest bones and their distances
		int boneIndices[3];
		float boneDistances[3];
		float boneWeights[3];

		// initialise boneDistance to a very far value
		for( int j=0;j<3;++j )
		{
			boneIndices[j] = -1;
			boneDistances[j] = 99999999.0f;
			boneWeights[j] = 0.0;
		}

		for( int j=0; j<skel->m_bones.size(); ++j )
		{
			Skeleton::Bone *bone = (Skeleton::Bone *)skel->m_bones.m_data[j];

			//
			// compute start and end point of bone segment
			//

			// startPoint: the global transform of the bone will rotate and translate the start position of all childbones
			math::Vec3f boneStart = math::transform( math::Vec3f(), bone->globalTransform );
			// endPoint: the parent transform will be concatenated by the addition translation the current bone adds on top
			math::Vec3f boneEnd = math::transform( math::Vec3f(), math::Matrix44f::TranslationMatrix(0.0f, bone->length, 0.0f)*bone->globalTransform );


			// compute distance of bone
			math::Vec3f p = geo->getPAttr()->getVec3f( i );
			float d = math::distancePointLineSegment( p, boneStart, boneEnd );


			for( int k = 0;k<3;++k )
				if( d<boneDistances[k] )
				{
					for( int l=2; l>k;--l )
					{
						boneDistances[l] = boneDistances[l-1];
						boneIndices[l] = boneIndices[l-1];
					}
					boneDistances[k] = d;
					boneIndices[k] = j;
					break;
				}
		}
		//float boneWeightSum = 0.0f;
		//float distanceSum = boneDistances[0] + boneDistances[1] + boneDistances[2];
		//for( int k = 0;k<3;++k )
		//{
		//	float w = boneDistances[k]/distanceSum;
		//	if ( w < 0.0 )
		//		w= 0.0;
		//	if ( w > 1.0 )
		//		w= 1.0;

		//	boneWeights[k] = (1.0f - w)/2.0f;
		//}
		boneWeights[0]  = 1.0f;
		boneWeights[1]  = 0.0f;
		boneWeights[2]  = 0.0f;

		//printf( "boneweights %f   %f   %f (sum:%f)          (%i %i %i)\n", boneWeights[0], boneWeights[1], boneWeights[2], boneWeights[0] + boneWeights[1] + boneWeights[2], boneIndices[0], boneIndices[1], boneIndices[2] );

		boneWeightsAttr->appendElement( boneWeights[0], boneWeights[1], boneWeights[2] );
		boneIndicesAttr->appendElement( (float)boneIndices[0], (float)boneIndices[1], (float)boneIndices[2] );
		boneDistancesAttr->appendElement( boneDistances[0], boneDistances[1], boneDistances[2] );
	}

	//
	// blur bone weights
	//
	Attribute *finalBoneWeightsAttr = boneWeightsAttr->copy();

	for( int iter = 0; iter < smoothIterations; ++iter )
	{
		msys_memcpy( boneWeightsAttr->m_data.m_data, finalBoneWeightsAttr->m_data.m_data, finalBoneWeightsAttr->m_data.size() );

		// 1. gather vertex neighbours
		int *neighbours = (int *)mmalloc( geo->getPAttr()->numElements()*10*sizeof(int) );
		for( int i=0;i<geo->getPAttr()->numElements()*10; ++i )
			neighbours[i] = -1;
		// for each quad
		for(int i = 0; i<geo->m_numPrimitives; ++i)
		{
			// for each vertex of current quad
			for(int j=0; j<4;++j)
			{
				// add prev and next to the neighbour list of cur
				int prev = geo->m_indexBuffer.m_data[i*4 + (j+3)%4];
				int cur = geo->m_indexBuffer.m_data[i*4 + j];
				int next = geo->m_indexBuffer.m_data[i*4 + (j+1)%4];

				addNeighbour( cur, prev, &neighbours[cur*10] );
				addNeighbour( cur, next, &neighbours[cur*10] );
			}
		}

		float tmp[3];
		float tmpW[3];
		float tmpNB[3];
		float tmpWNB[3];
		// for each vertex
		for( int i=0;i<geo->getPAttr()->numElements(); ++i )
		{
			boneIndicesAttr->getElement( i, tmp );
			boneWeightsAttr->getElement( i, tmpW );

			// for each affector bone(0-2)
			for( int j=0; j<3; ++j )
			{
				float sum = tmpWNB[j];
				int num = 1;
				// loop over all vertex neighbours
				int *nbs = &neighbours[i*10];
				while( *nbs != -1 )
				{
					// find current affector bone at neighbour
					boneIndicesAttr->getElement( *nbs, tmpNB );
					boneWeightsAttr->getElement( *nbs, tmpWNB );
					for( int k =0; k<3;++k )
					{
						if( int(tmp[j]) == int(tmpNB[k]) )
						{
							// add weight, increase num
							sum += tmpWNB[k];
							++num;
							break;
						}
					}

					++nbs;
				}

				// set weight for current affector bone to sum/num
				tmpW[j] = sum / (float)num;
			}

			//finalBoneWeightsAttr->setElement( i, &(math::normalize( math::Vec3f(tmpW[0], tmpW[1], tmpW[2]) )) );
			float ss = tmpW[0]+tmpW[1]+tmpW[2];
			tmpW[0] /= ss;
			tmpW[1] /= ss;
			tmpW[2] /= ss;
			finalBoneWeightsAttr->setElement( i, tmpW );
		}
	}


	//Attribute *color = new Attribute(4);
	//// draw all points using the weights from the first bone
	//for( int i=0; i<geo->getPAttr()->numElements(); ++i )
	//{
	//	math::Vec3f p = geo->getPAttr()->getVec3f(i);
	//	math::Vec3f discances = boneDistancesAttr->getVec3f(i);
	//	color->appendElement( 1.0f, 0.0f, 0.0f, 1.0f);

	//	// set color depending on boneindex
	//	int boneI[3];
	//	float c[4];
	//	boneIndicesAttr->getElement(i,&boneI[0]);

	//	c[0] = discances[0]/.1f;
	//	c[1] = discances[0]/.1f;
	//	c[2] = discances[0]/.1f;
	//	c[3] = 1.0f;
	//}
	//geo->setAttr( ATTR_Cd, color );


	geo->setAttr( ATTR_BONEWEIGHTS, finalBoneWeightsAttr );
	geo->setAttr( ATTR_BONEINDICES, boneIndicesAttr );
	geo->setUniform( "boneMatrices", skel->m_boneMatricesUniform );

}



//
// separates geometry into different polygon islands (which still will be in one geometry)
// where each island is driven by one bone.
//
// it assumes the geometry is skinned (has boneweights and indices attribute)
// assumes triangles only!
//
void apply_separateFromSkeleton( Geometry *geo, Skeleton *skel )
{
	// for each primitive
		// classify primitive
		// iterate over all vertices of prim
			// if vertex class doesnt match prim class
				// split vertex
					// iterate over all other prims
						// if other prim contains v
							// change it to the copy
			// set vertex class

	for( int j=0;j<geo->m_numPrimitives;++j )
	{
		int primClass = int(geo->getAttr( ATTR_BONEINDICES )->getVec3f( geo->m_indexBuffer.m_data[j*geo->m_numComponents+0] )[0]);

		// for each face vertex
		for( int k=0;k<geo->m_numComponents;++k )
		{
			int v = geo->m_indexBuffer.m_data[j*geo->m_numComponents+k];
			math::Vec3f vBoneWeights = geo->getAttr( ATTR_BONEWEIGHTS )->getVec3f( v );
			math::Vec3f vBoneIndices = geo->getAttr( ATTR_BONEINDICES )->getVec3f( v );

			//vBoneWeights[0] = 1.0f;
			//vBoneWeights[1] = 0.0f;
			//vBoneWeights[2] = 0.0f;

			//geo->getAttr( ATTR_BONEWEIGHTS )->setElement( v, &vBoneWeights );

			int vertexClass = int(vBoneIndices[0]);

			if( vertexClass != primClass )
			{
				geo->splitVertex( v, 2 );

				geo->m_indexBuffer.m_data[j*geo->m_numComponents+k] = geo->getPAttr()->numElements()-1;
				vBoneIndices[0] = (float)primClass;
				geo->getAttr( ATTR_BONEINDICES )->setElement( geo->getPAttr()->numElements()-1, &vBoneIndices );
			}
		}
	}
}


//
// computes vertex normals
// Assumes geometry to be triangles!
//
void apply_normals( Geometry *geo )
{
	Attribute *normalAttr = new Attribute( 3 );

	for( int i=0; i < geo->getPAttr()->numElements(); ++i )
		normalAttr->appendElement( 0.0f, 0.0f, 0.0f );

	for( int i=0; i < geo->m_numPrimitives; ++i )
	{
		int i0 = geo->m_indexBuffer.m_data[i*geo->m_numComponents];
		int i1 = geo->m_indexBuffer.m_data[i*geo->m_numComponents+1];
		int i2 = geo->m_indexBuffer.m_data[i*geo->m_numComponents+2];

		math::Vec3f v1 = geo->getPAttr()->getVec3f( i1 )-geo->getPAttr()->getVec3f( i0 );
		math::Vec3f v2 = geo->getPAttr()->getVec3f( i2 )-geo->getPAttr()->getVec3f( i0 );
		math::Vec3f fn = math::normalize( math::crossProduct( v2,v1 ) );

		for( int j=0; j<geo->m_numComponents; ++j )
		{
			int fv = geo->m_indexBuffer.m_data[i*geo->m_numComponents+j];
			normalAttr->setElement( fv, &(normalAttr->getVec3f(fv)+fn) );
		}

		//normalAttr->setElement( i0, &(normalAttr->getVec3f(i0)+fn) );
		//normalAttr->setElement( i1, &(normalAttr->getVec3f(i1)+fn) );
		//normalAttr->setElement( i2, &(normalAttr->getVec3f(i2)+fn) );
	}

	for( int i=0; i < geo->getPAttr()->numElements(); ++i )
	{
		math::Vec3f tmp = math::normalize(  normalAttr->getVec3f(i) );
		normalAttr->setElement( i, &tmp );
	}

	geo->setAttr( ATTR_N, normalAttr );
}



void apply_mirror( Geometry *geo )
{
	// copy vertices
	int numPoints = geo->getPAttr()->numElements();

	// for each vertex attribute
	for( int i=0; i<numPoints; ++i )
	{
		geo->splitVertex(i, 2);
		math::Vec3f p = geo->getPAttr()->getVec3f(i);
		p.x = -p.x;
		geo->getPAttr()->setElement( i+numPoints, &p );
	}

	//
	// copy faces
	//
	int numFaceVertices = geo->m_indexBuffer.size();
	geo->m_indexBuffer.resize( geo->m_indexBuffer.size()*2 );
	for( int i=0; i<geo->m_numPrimitives; ++i )
	{
		for(int j = 0; j<geo->m_numComponents; ++j)
			geo->m_indexBuffer.m_data[numFaceVertices + i*geo->m_numComponents+j] = geo->m_indexBuffer.m_data[(i+1)*geo->m_numComponents-1-j]+numPoints;
		//for(int j = 0; j<geo->m_numComponents; ++j)
		//	geo->m_indexBuffer.m_data[numFaceVertices + i*geo->m_numComponents+j] = geo->m_indexBuffer.m_data[i*geo->m_numComponents+j]+numPoints;
	}
	geo->m_numPrimitives = geo->m_numPrimitives*2;

	geo->autoWeld( 0.015f );
}

static int getEdgeId( int a, int b, int *edges, int *nedges, Attribute *p )
{
	// encode a/b into one integer .. limits number of edges to 2^16
	// makes it more elegant to look for the edge (its just an integer compare)
	int n = (a>b)?(a<<16)|b:(b<<16)|a;

	// look for given edge
	int i;
	for( i=0; edges[2*i+0]!=n && i<*nedges; i++ );
	if( i==*nedges )
	{
		edges[2*i+0] = n; // set the edge
		(*nedges)++; // increase number of edges if we just have added the edge
		p->appendElement( 0.0f, 0.0f, 0.0f ); // append edgepoint
	}
	edges[2*i+1]++;   // increase access count (== number of shared faces)
	return i;
}

const static float vpw[4] = { 9.0f, 3.0f, 1.0f, 3.0f };
const static float epw[4] = { 3.0f, 3.0f, 1.0f, 1.0f };

void apply_catmullclark( Geometry *geo, int numIterations )
{
	for( int iter=0; iter<numIterations; ++iter )
	{
		Geometry *org = geo->copy();

		// original points are  left - we just clear out primitives
		geo->m_indexBuffer.clear();
		geo->m_numPrimitives = 0;


		//
		// do catmull clark subdivision
		//
		int abcd[4]; // used to rearrange quad indices so that abcd[0] always points to the current vertex
		int eid[4];
		int *edges = (int *)mmalloc( sizeof(int)*10000 );
		int *faceValences = (int *)mmalloc( sizeof(int)*10000 );
		int numEdges = 0;

		msys_memset( edges, 0, sizeof(int)*10000 );
		msys_memset( faceValences, 0, sizeof(int)*10000 );

		// zero out vertex points
		msys_memset( geo->getPAttr()->m_data.m_data, 0, geo->getPAttr()->m_data.size() );

		// add face center points
		int fpOffset = geo->getPAttr()->numElements();
		int epOffset = geo->getPAttr()->appendElements( org->m_numPrimitives );

		// for each quad
		for(int i = 0; i<org->m_numPrimitives; ++i)
		{
			math::Vec3f faceCenter;
			// for each vertex of current quad
			for(int j=0; j<4;++j)
			{
				// rearrange into convenience structure
				for(int k=0; k<4;++k)
					abcd[k] = org->m_indexBuffer.m_data[i*4 + (j+k)%4];
				// get id of edge between current quad vertex with the next quadvertex
				eid[j] = getEdgeId( abcd[0], abcd[1], edges, &numEdges, geo->getPAttr() );
				// update face valences of current quad vertex
				faceValences[abcd[0]]++;
				// update face center point
				faceCenter += 0.25f * org->getPAttr()->getVec3f( abcd[0] );

				for( int k=0; k<4; ++k )
				{
					// increment vertex point
					geo->getPAttr()->setElement( abcd[0], &(geo->getPAttr()->getVec3f(abcd[0]) + vpw[k]*org->getPAttr()->getVec3f(abcd[k])) );
					// increment edge point
					geo->getPAttr()->setElement( epOffset +eid[j], &(geo->getPAttr()->getVec3f(epOffset +eid[j]) + epw[k]*org->getPAttr()->getVec3f(abcd[k])) );
				}
			}

			// set face centerpoint
			geo->getPAttr()->setElement(fpOffset+i, &faceCenter);

			// add child faces
			for(int j=0; j<4;++j)
			//for(int j=3; j>=0;--j)
				geo->addQuad( fpOffset+i, epOffset+eid[(3+j)&3], org->m_indexBuffer.m_data[i*4+j], epOffset+eid[(0+j)&3] );
		}

		// for each original point
		for(int i=0; i<fpOffset;++i)
		{
			geo->getPAttr()->setElement( i, &(geo->getPAttr()->getVec3f(i)*(0.0625f/(float)faceValences[i])) );
			math::Vec3f pp = geo->getPAttr()->getVec3f( i );

		}
		// for each edge point
		for(int i=epOffset; i<geo->getPAttr()->numElements();++i)
		{
			geo->getPAttr()->setElement( i, &(geo->getPAttr()->getVec3f(i)*(0.1250f/(float)edges[(i-epOffset)*2+1])) );
			math::Vec3f pp = geo->getPAttr()->getVec3f( i );
		}

		delete org;
		mfree(edges);
		mfree(faceValences);
	}
}








void apply_transform( Geometry *geo, math::Matrix44f tm )
{
	for( int i=0; i<geo->getPAttr()->numElements(); ++i )
		geo->getPAttr()->setElement( i, &(math::transform( geo->getPAttr()->getVec3f(i), tm)) );
}


void apply_noise( Geometry *geo, math::Vec3f strength )
{
	for( int i=0; i<geo->getPAttr()->numElements(); ++i )
		geo->getPAttr()->setElement( i, &(geo->getPAttr()->getVec3f(i)+math::Vec3f( msys_frand()*strength.x, msys_frand()*strength.y, msys_frand()*strength.z )) );
}









void apply_cpuskinning( Geometry *geo, Skeleton *skel, Geometry *geo_deformed )
{
    Attribute *positionAttr = geo->getAttr( ATTR_P );
    Attribute *deformedPositionAttr = geo_deformed->getAttr( ATTR_P );
    Attribute *boneWeightsAttr = geo->getAttr( ATTR_BONEWEIGHTS );
    Attribute *boneIndicesAttr = geo->getAttr( ATTR_BONEINDICES );

    // for each vertex
    for( int i=0; i<positionAttr->numElements(); ++i )
    {
            // get bone index
            math::Vec3f p = positionAttr->getVec3f(i);
            float boneWeights[3];
            float boneIndices[3];

            boneWeightsAttr->getElement(i, boneWeights);
            boneIndicesAttr->getElement(i, boneIndices);

            // now compute vertex position
            Skeleton::Bone *b0 = skel->getBone( int(boneIndices[0]) );
			Skeleton::Bone *b1 = skel->getBone( int(boneIndices[1]) );
			Skeleton::Bone *b2 = skel->getBone( int(boneIndices[2]) );
            math::Vec3f p_deformed0 = transform(p, b0->vertexTransform);
			math::Vec3f p_deformed1 = transform(p, b1->vertexTransform);
			math::Vec3f p_deformed2 = transform(p, b2->vertexTransform);
			math::Vec3f p_deformed = p_deformed0*boneWeights[0] + p_deformed1*boneWeights[1] + p_deformed2*boneWeights[2];

            deformedPositionAttr->setElement( i, &p_deformed.v[0] );
    }
}
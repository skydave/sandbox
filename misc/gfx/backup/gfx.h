#pragma once
#include "Shader.h"

#include "Geometry.h"
#include "Texture.h"
#include "CurveGeometry.h"
#include "Skeleton.h"
#include "FCurve.h"
#include "Camera.h"
//#include "Text.h"
#include "FBO.h"
//#include "Clip.h"
//#include "ShotManager.h"

#ifdef _DEBUG
#include "DebugNavigator.h"
#endif




// generators
Geometry *geogen_pointCloud();
Geometry *geogen_circle();
Geometry *geogen_cube();
Geometry *geogen_grid( int xres, int yres );
Geometry *geogen_sphere( int uSubdivisions, int vSubdivisions );
Geometry *geogen_quad();
Geometry *geogen_mesh( unsigned char* points, int numPoints, unsigned char *indices, int numTriangles );
Geometry *geogen_quadtmp();

// modifiers
void apply_skin( Geometry *geo, Skeleton *skel, int smoothIterations = 0 ); // applies boneweight and bone index attribute for each vertex
void apply_separateFromSkeleton( Geometry *geo, Skeleton *skel ); //
void apply_normals( Geometry *geo );
void apply_mirror( Geometry *geo );
void apply_catmullclark( Geometry *geo, int numIterations = 1 );
void apply_transform( Geometry *geo, math::Matrix44f tm );
void apply_noise( Geometry *geo, math::Vec3f strength );
void apply_cpuskinning( Geometry *geo, Skeleton *skel, Geometry *geo_deformed );


//============================================================================
//
//
//
//============================================================================


#include <fbxsdk.h>

// need this to make fbxsdk happy
#ifdef _WINDOWS
#pragma message("     _Adding library: wininet.lib" ) // apparently not linking this library causes unresolved symbols when statically linking fbxsdk - it starts getting messy :(
#pragma comment (lib, "wininet.lib")
#endif 

#include <QtGui>
#include <QApplication>

#include <stdio.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include <ui/GLViewer.h>
#include <gltools/gl.h>
#include <gltools/misc.h>
#include <util/StringManip.h>
#include <util/Path.h>
#include <gfx/Geometry.h>
#include <gfx/ObjIO.h>
#include <gfx/Shader.h>
#include <gfx/Texture.h>
#include <gfx/Image.h>
#include <gfx/Context.h>
#include <gfx/FCurve.h>
#include <gfx/glsl/common.h>
#include <gfx/FBO.h>

#include <ops/ops.h>

#include "ops/TimeOp.h"
#include "ops/CameraOp.h"
#include "ops/RenderGeoOp.h"
#include "ops/DemoOp.h"
#include "ops/FBXTransformOp.h"

#include "composer/widgets/CurveEditor/CurveEditor.h"
#include "composer/widgets/Trackball/Trackball.h"
#include "composer/widgets/GLViewer/GLViewer.h"

#include "OSCDispatcher.h"



float g_testValue = 0.0f;


OSCDispatcher oscDispatcher;

composer::widgets::GLViewer *glviewer;


base::ContextPtr context;

base::GeometryPtr geo;

base::ShaderPtr baseShader;
base::ShaderPtr defaultGeometryShader;
base::Texture2dPtr baseTexture;
base::GeometryPtr baseGeo;

base::ops::OpPtr opRoot;
DemoOpPtr demoOp;
base::ops::ConstantOpPtr orbitTransform;


void renderGeo()
{
	context->render( geo, baseShader );
}

void render( base::CameraPtr cam )
{
	glEnable( GL_DEPTH_TEST );
	orbitTransform->m_variant = cam->m_transform;
	opRoot->execute();
}
void mouseMove( base::MouseState ms )
{
	//if(ms.buttons)
	float t = (float)ms.x / (float)glviewer->width();
	base::ops::Manager::context()->setTime(t);
}


// Initialize the sdk manager. This object handles all our memory management.
KFbxSdkManager* lSdkManager = NULL;
KFbxIOSettings *ios = NULL;
KFbxGeometryConverter  *geoConverter = NULL;
KFbxAnimEvaluator *animEvaluator = NULL;



struct FBXMeshVertex
{
	FBXMeshVertex()
	{
		//elementTypes = {0,0,0,0,0,0,0};
	}

	//char elementTypes[7]; // identifies the different layerelement types (unused, float, vec2, vec3, vec4) for up to 7 possible layerelements
	std::vector<float> data;
};

// creates a rendergeometry op from given fbxmesh
RenderGeoOpPtr buildFromFBX( KFbxMesh *fbxMesh )
{
	// create geometry from fbxMesh
	KFbxMesh *fbxTriMesh = geoConverter->TriangulateMesh( fbxMesh );
	//geoConverter->EmulateNormalsByPolygonVertex( fbxTriMesh );

	// triangulate mesh
	base::GeometryPtr geo = base::Geometry::createTriangleGeometry();


	// what do we have?
	std::vector<KFbxLayerElementNormal *> normalElements;
	std::vector<KFbxLayerElementUV *> uvElements;
	std::vector<KFbxLayerElementVertexColor *> vertexColorElements;
	std::vector<base::AttributePtr> attributes;
	std::vector<std::string> attributeNames;

	attributes.push_back( geo->getAttr("P") );
	attributeNames.push_back("P");

	// get additional layers (normals, vertex colors)
	int numLayers = fbxTriMesh->GetLayerCount();
	for( int i=0;i<numLayers;++i )
	{
		KFbxLayer *layer = fbxTriMesh->GetLayer(0);

		std::cout << "got Layer!\n";
		if( layer->GetNormals() != NULL )
		{
			std::cout << "got N!\n";
			normalElements.push_back(layer->GetNormals());
			attributes.push_back( base::Attribute::createVec3f() );
			if( i==0 )
				attributeNames.push_back("N");
			else
			{
				std::stringstream ss;ss << "N" << i;
				attributeNames.push_back(ss.str());
			}
			//geo->setAttr( attributeNames.back(), attributes.back()  );
		}
		if( layer->GetUVs() != NULL )
		{
			std::cout << "got UV!\n";
			uvElements.push_back(layer->GetUVs());
			attributes.push_back( base::Attribute::createVec2f() );
			if( i==0 )
				attributeNames.push_back("UV");
			else
			{
				std::stringstream ss;ss << "UV" << i;
				attributeNames.push_back(ss.str());
			}
			//geo->setAttr( attributeNames.back(), attributes.back()  );
		}
		if( layer->GetVertexColors() != NULL )
		{
			std::cout << "got Cd!\n";
			vertexColorElements.push_back(layer->GetVertexColors());
			attributes.push_back( base::Attribute::createVec3f() );
			if( i==0 )
				attributeNames.push_back("Cd");
			else
			{
				std::stringstream ss;ss << "Cd" << i;
				attributeNames.push_back(ss.str());
			}
			geo->setAttr( attributeNames.back(), attributes.back()  );
		}
	}

	// temp
	std::vector<FBXMeshVertex> vertices;

	// deindex ---
	// get triangles
	int numTris = fbxTriMesh->GetPolygonCount();
	vertices.reserve( numTris * 3 );
	int indexByPolygonVertex = 0;
	for( int i = 0; i<numTris; ++i )
	{
		int numVertices = fbxTriMesh->GetPolygonSize(i);
		if( numVertices == 3 )
		{
			for( int j=0;j<3;++j )
			{
				FBXMeshVertex vertex;
				KFbxVector4 p = fbxTriMesh->GetControlPointAt( fbxTriMesh->GetPolygonVertex(i, j) );
				vertex.data.push_back(p[0]);
				vertex.data.push_back(p[1]);
				vertex.data.push_back(p[2]);

				for( std::vector<KFbxLayerElementNormal *>::iterator it = normalElements.begin(); it != normalElements.end(); ++it )
				{
					KFbxLayerElementNormal *n = *it;

					/*
			KFbxLayerElementNormal::EMappingMode mapping = n->GetMappingMode();
			switch( mapping )
			{
				case KFbxLayerElementNormal::eBY_CONTROL_POINT:
				{
					std::cout << "point attribute\n";
				}break;
				case KFbxLayerElementNormal::eBY_POLYGON_VERTEX:
				{
					std::cout << "vertex attribute\n";
				}break;
			};
					*/
					vertex.data.push_back( 0.0f );
					vertex.data.push_back( 0.0f );
					vertex.data.push_back( 0.0f );
				}
				for( std::vector<KFbxLayerElementUV *>::iterator it = uvElements.begin(); it != uvElements.end(); ++it )
				{
					KFbxLayerElementUV *uv = *it;

					/*
								KFbxLayerElementUV::EMappingMode mapping = uv->GetMappingMode();
			switch( mapping )
			{
				case KFbxLayerElementUV::eBY_CONTROL_POINT:
				{
					std::cout << "point attribute\n";
				}break;
				case KFbxLayerElementUV::eBY_POLYGON_VERTEX:
				{
					std::cout << "vertex attribute\n";
				}break;
			};*/
					vertex.data.push_back( 0.0f );
					vertex.data.push_back( 0.0f );

				}
				for( std::vector<KFbxLayerElementVertexColor *>::iterator it = vertexColorElements.begin(); it != vertexColorElements.end(); ++it )
				{
					KFbxLayerElementVertexColor *cd = *it;

					switch( cd->GetMappingMode() )
					{
						case KFbxLayerElementVertexColor::eBY_CONTROL_POINT:
						{
							//TODO
						}break;
						case KFbxLayerElementVertexColor::eBY_POLYGON_VERTEX:
						{
							int vertexColorIndex = 0;
							switch(cd->GetReferenceMode())
							{
							case KFbxLayerElementVertexColor::eDIRECT:
								{
									vertexColorIndex = indexByPolygonVertex;
								}break;
							case KFbxLayerElementVertexColor::eINDEX_TO_DIRECT:
								{
									vertexColorIndex = cd->GetIndexArray().GetAt(indexByPolygonVertex);
								}break;
							};
							KFbxColor c = cd->GetDirectArray().GetAt(vertexColorIndex);
							vertex.data.push_back( c[2] );
							vertex.data.push_back( c[1] );
							vertex.data.push_back( c[0] );
						}break;
					};

					
				} // for each vertexcolor layer

				vertices.push_back( vertex );

				++indexByPolygonVertex;
			} // for each vertex
		} // if numvertices == 3
	} // for each face

	// reindex ---


	// fill attributes
	for( std::vector<FBXMeshVertex>::iterator it = vertices.begin(); it != vertices.end(); ++it )
	{
		FBXMeshVertex &v = *it;
		int attributeIndex = 0;
		int dataIndex = 0;

		attributes[attributeIndex++]->appendElement<math::Vec3f>( math::Vec3f( v.data[dataIndex++], v.data[dataIndex++], v.data[dataIndex++] ) );

		for( std::vector<KFbxLayerElementNormal *>::iterator it = normalElements.begin(); it != normalElements.end(); ++it )
		{
			KFbxLayerElementNormal *n = *it;
			attributeIndex++;
			dataIndex++;
			dataIndex++;
			dataIndex++;
		}
		for( std::vector<KFbxLayerElementUV *>::iterator it = uvElements.begin(); it != uvElements.end(); ++it )
		{
			KFbxLayerElementUV *uv = *it;
			attributeIndex++;
			dataIndex++;
			dataIndex++;
		}
		for( std::vector<KFbxLayerElementVertexColor *>::iterator it = vertexColorElements.begin(); it != vertexColorElements.end(); ++it )
		{
			KFbxLayerElementVertexColor *cd = *it;
			attributes[attributeIndex++]->appendElement<math::Vec3f>( math::Vec3f( v.data[dataIndex++], v.data[dataIndex++], v.data[dataIndex++] ) );
		} // for each vertexcolor layer
	}

	// tris
	int triVertexIndex = 0;
	for( int i=0;i<numTris;++i )
	{
		//geo->addTriangle( fbxTriMesh->GetPolygonVertex(i, 0), fbxTriMesh->GetPolygonVertex(i, 1), fbxTriMesh->GetPolygonVertex(i, 2) );
		geo->addTriangle( triVertexIndex++, triVertexIndex++, triVertexIndex++ );
	}

/*
	// get points
	int numPoints = fbxTriMesh->GetControlPointsCount();
	for( int i=0; i<numPoints; ++i )
	{
		KFbxVector4 p = fbxTriMesh->GetControlPointAt(i);
		pAttr->appendElement<math::Vec3f>( math::Vec3f( p[0], p[1], p[2] ) );
		std::cout << "p: " << p[0] << " " << p[1] << " " << p[2] << std::endl;
	}
*/




	// create and setup renderop
	RenderGeoOpPtr renderGeoOp = RenderGeoOp::create();
	renderGeoOp->m_geo = geo;
	renderGeoOp->m_shader = defaultGeometryShader;

	return renderGeoOp;
}


// creates a camer op from given fbxcamera
CameraOpPtr buildFromFBX( KFbxCamera *fbxCamera, KFbxNode *node )
{
	// we will need the transform
	KTime myTime;
	KFbxXMatrix& worldTransform = animEvaluator->GetNodeGlobalTransform(node, myTime);
	double *values = worldTransform;


	// create and setup cameraop
	CameraOpPtr cameraOp = CameraOp::create();


	FBXTransformOpPtr fbxTransform = FBXTransformOp::create( node );
	fbxTransform->plug( cameraOp, "transformMatrix" );

	//orbitTransform->plug( cameraOp, "transformMatrix" );
	cameraOp->m_camera->m_fov = math::degToRad(54.0f);
	cameraOp->m_camera->m_znear = 0.1f;
	cameraOp->m_camera->m_zfar = 100.0f;

	return cameraOp;
}

base::ops::OpPtr buildFromFBX( std::string path )
{
	// Create an importer using our sdk manager.
	KFbxImporter* lImporter = KFbxImporter::Create(lSdkManager,"");

	// Use the first argument as the filename for the importer.
	if(!lImporter->Initialize(path.c_str(), -1, lSdkManager->GetIOSettings()))
	{
		std::cout << "Call to KFbxImporter::Initialize() failed.\n";
		std::cout << "Error returned: %s\n\n" << std::string(lImporter->GetLastErrorString());
		exit(-1);
	}




	// Create a new scene so it can be populated by the imported file.
	KFbxScene* lScene = KFbxScene::Create(lSdkManager,"myScene");

	// Import the contents of the file into the scene.
	std::cout << "importing " << path << std::endl;
	lImporter->Import(lScene);

	//KFbxAxisSystem max(KFbxAxisSystem::eDirectX);
	//max.ConvertScene(lScene);

	int dir, dirSign, front, frontSign;
	dir = lScene->GetGlobalSettings().GetAxisSystem().GetUpVector(dirSign);
	front = lScene->GetGlobalSettings().GetAxisSystem().GetFrontVector(frontSign);
	std::cout << "up axis :" << dir << " " << dirSign << std::endl;
	std::cout << "front axis :" << front << " " << frontSign << std::endl;


	// The file has been imported; we can get rid of the importer.
	lImporter->Destroy();

	// now we will build an operator graph which looks like this:
	//setCamera (using world transform from fbx transform hierarchy)
	//for each entity:
	//	transform (worldTransform from fbx transform hierarchy)
	//		renderEntity (e.g. renderMesh/Geometry)
	// Print the nodes of the scene and their attributes recursively.
	// Note that we are not printing the root node, because it should
	// not contain any attributes.


	// tmp
	animEvaluator = lScene->GetEvaluator();

	TimeOpPtr root = TimeOp::create();

	// 100 frames == 4s
	root->m_outTimeEnd = 4.0f;

	CameraOpPtr cameraOp;
	std::vector<base::ops::OpPtr> items;

	// iterate over all nodes
	int numNodes = lScene->GetNodeCount();
	for( int i=0;i < numNodes; ++i )
	{
		KFbxNode *node = lScene->GetNode(i);

		// disable pre/post rotation
		node->SetRotationActive(false);

		std::cout << node->GetName() << std::endl;

		// items ===
		int numAttrs = node->GetNodeAttributeCount();

		for( int j=0;j<numAttrs;++j )
		{
			KFbxNodeAttribute const *attr = node->GetNodeAttributeByIndex(j);

			switch( attr->GetAttributeType() )
			{
			/*
				eNULL,
				eMARKER,
				eSKELETON,
				eNURB,
				ePATCH,
				eCAMERA_STEREO,
				eCAMERA_SWITCHER,
				eLIGHT,
				eOPTICAL_REFERENCE,
				eOPTICAL_MARKER,
				eNURBS_CURVE,
				eTRIM_NURBS_SURFACE,
				eBOUNDARY,
				eNURBS_SURFACE,
				eSHAPE,
				eLODGROUP,
				eSUBDIV,
				eCACHED_EFFECT,
				eLINE
			*/
				case KFbxNodeAttribute::eMESH:
				{
					std::cout << "has a mesh!\n";
					RenderGeoOpPtr op = buildFromFBX( (KFbxMesh *)attr  );
					items.push_back(op);
				}break;
				case KFbxNodeAttribute::eCAMERA:
				{
					// TODO: handle multiple cameras
					std::cout << "has a camera!\n";
					cameraOp = buildFromFBX( (KFbxCamera *)attr, node );
				}break;
				case KFbxNodeAttribute::eUNIDENTIFIED:
				default:
				{
					std::cout << "has a something unknown!\n";
				}break;
			};

		} // for each node attribute
	} // for each node


	// if there is no camera
	// TODO: create default cam

	// plug camera into root
	cameraOp->plug( root );

	// now plug all items into camera
	for( std::vector<base::ops::OpPtr>::iterator it = items.begin(); it != items.end(); ++it )
		(*it)->plug( cameraOp );


	return root;
}



void init()
{
	std::cout << "init!\n";


	GLenum glewResult = glewInit();
	if (GLEW_OK != glewResult)
	{
		std::cout << "glew init failed\n";
	}

	std::string fbxTest = std::string(SRC_PATH) + std::string("/data/cube01_max.fbx");

	// Initialize the sdk manager. This object handles all our memory management.
	lSdkManager = KFbxSdkManager::Create();
	// Create the io settings object.
	ios = KFbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);
	geoConverter = new KFbxGeometryConverter(lSdkManager);
	animEvaluator = KFbxAnimEvaluator::Create( lSdkManager, "" );




	context = base::ContextPtr( new base::Context() );
	base::ops::Manager::setContext(context);


	// op testing =============
	//base::ops::SphereOpPtr s = base::ops::SphereOp::create(1.0f);
	//base::MeshPtr m = s->getMesh(0);
	//geo = m->getGeometry();


	// demo =============
	demoOp = DemoOp::create( "/usr/people/david-k/dev/testprojects/sandbox/temp/sketch039.ogg" );
	//demoOp = DemoOp::create( "c:\\projects\\sandbox\\temp\\code\\sketch039.ogg" );
	TimeOpPtr time = TimeOp::create();
	CameraOpPtr cam = CameraOp::create();
	base::ops::ClearOpPtr clear = base::ops::ClearOp::create();
	orbitTransform = base::ops::ConstantOp::create();
	base::ops::FuncOpPtr renderFunc = base::ops::FuncOp::create( renderGeo );

	//clear->plug( cam );
	//renderFunc->plug( cam );

	//cam->plug( demoOp );

	//orbitTransform->plug( cam, "transformMatrix" );

	opRoot = demoOp;


	//
	baseShader = base::Shader::load( base::Path( SRC_PATH ) + "/src/base/gfx/glsl/geometry_vs.glsl", base::Path( SRC_PATH ) + "/src/base/gfx/glsl/geometry_ps.glsl" );
	baseGeo = base::importObj( base::Path( SRC_PATH ) + "/data/test.1.obj" );
	//base::apply_transform( baseGeo, math::Matrix44f::ScaleMatrix( 30000.0f ) );
	base::apply_normals( baseGeo );

	defaultGeometryShader = base::Shader::load( base::Path( SRC_PATH ) + "/src/glsl/defaultGeometry.vs.glsl", base::Path( SRC_PATH ) + "/src/glsl/defaultGeometry.ps.glsl" );

	baseTexture = base::Texture2d::load( base::Path( SRC_PATH ) + "/src/base/data/uvref2.png" );
	baseShader->setUniform( "input", baseTexture->getUniform() );



	// fbx import test =============
	base::ops::OpPtr renderFBXSceneOp = buildFromFBX(fbxTest);

	clear->plug( opRoot );
	renderFBXSceneOp->plug( opRoot );

	base::ops::Manager::context()->setTime( .5 );

	//demoOp->startAudio();
}

void shutdown()
{
	//demoOp->stopAudio();
}





int main(int argc, char ** argv)
{
	QApplication app(argc, argv);
	app.setOrganizationName("test");
	app.setApplicationName("test");

	QMainWindow mainWin;
	mainWin.resize(800, 600);
	glviewer = new composer::widgets::GLViewer(init, render, shutdown);
	glviewer->getCamera()->m_znear = .1f;
	glviewer->getCamera()->m_zfar = 100000.0f;
	glviewer->setRenderInSeperateThread(true);
	glviewer->setMouseMoveCallback( mouseMove );
	mainWin.setCentralWidget( glviewer );
	mainWin.show();



	oscDispatcher.start();

	return app.exec();
}

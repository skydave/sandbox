//============================================================================
//
//
//
//============================================================================



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

#include "TimeOp.h"
#include "CameraOp.h"

#include "composer/widgets/CurveEditor/CurveEditor.h"
#include "composer/widgets/Trackball/Trackball.h"
#include "composer/widgets/GLViewer/GLViewer.h"


#include <osc/OscReceivedElements.h>
#include <osc/OscPacketListener.h>
#include <ip/UdpSocket.h>


#define PORT 7000

class ExamplePacketListener : public osc::OscPacketListener {
protected:

	virtual void ProcessMessage( const osc::ReceivedMessage& m,
				const IpEndpointName& remoteEndpoint )
	{
		try{
			// example of parsing single messages. osc::OsckPacketListener
			// handles the bundle traversal.

			if( std::strcmp( m.AddressPattern(), "/test1" ) == 0 ){
				// example #1 -- argument stream interface
				osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
				bool a1;
				osc::int32 a2;
				float a3;
				const char *a4;
				args >> a1 >> a2 >> a3 >> a4 >> osc::EndMessage;

				std::cout << "received '/test1' message with arguments: "
					<< a1 << " " << a2 << " " << a3 << " " << a4 << "\n";

			}else if( std::strcmp( m.AddressPattern(), "/test2" ) == 0 ){
				// example #2 -- argument iterator interface, supports
				// reflection for overloaded messages (eg you can call
				// (*arg)->IsBool() to check if a bool was passed etc).
				osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
				bool a1 = (arg++)->AsBool();
				int a2 = (arg++)->AsInt32();
				float a3 = (arg++)->AsFloat();
				const char *a4 = (arg++)->AsString();
				if( arg != m.ArgumentsEnd() )
					throw osc::ExcessArgumentException();

				std::cout << "received '/test2' message with arguments: "
					<< a1 << " " << a2 << " " << a3 << " " << a4 << "\n";
			}
		}catch( osc::Exception& e ){
			// any parsing errors such as unexpected argument types, or
			// missing arguments get thrown as exceptions.
			std::cout << "error while parsing message: "
				<< m.AddressPattern() << ": " << e.what() << "\n";
		}
	}
};



ExamplePacketListener listener;
UdpListeningReceiveSocket s( IpEndpointName( IpEndpointName::ANY_ADDRESS, PORT ), &listener );




composer::widgets::GLViewer *glviewer;


base::ContextPtr context;

base::GeometryPtr geo;

base::ShaderPtr baseShader;
base::Texture2dPtr baseTexture;
base::GeometryPtr baseGeo;

base::ops::OpPtr opRoot;
base::ops::ConstantOpPtr orbitTransform;


void render( base::CameraPtr cam )
{

	orbitTransform->m_variant = cam->m_transform;

	/*
	//glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
	glDisable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );


	context->setView( cam->m_viewMatrix, cam->m_transform, cam->m_projectionMatrix );


	// render to screen
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	context->render( geo, baseShader );
	*/

	opRoot->execute();


}


void render2()
{
	std::cout << "render2!\n";
	
	// render to screen
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	context->render( geo, baseShader );

}




void init()
{
	std::cout << "init!\n";


	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "glew init failed\n";
	}

	context = base::ContextPtr( new base::Context() );


	// op testing
	base::ops::SphereOpPtr s = base::ops::SphereOp::create(1.0f);
	base::MeshPtr m = s->getMesh(0);
	geo = m->getGeometry();


	// demo
	TimeOpPtr time = TimeOp::create();
	CameraOpPtr cam = CameraOp::create();
	orbitTransform = base::ops::ConstantOp::create();
	base::ops::FuncOpPtr renderFunc = base::ops::FuncOp::create( render2 );

	renderFunc->plug( cam );
	cam->plug( time );

	orbitTransform->plug( cam, "transformMatrix" );

	opRoot = time;


	// rendering
	/*

	RenderOpPtr rop = base::ops::RenderOp::create();
	rop->append( base::ops::ClearOp::create() );
	rop->append( base::ops::SkyOp::create() );
	rop->append( base::ops::RenderMeshOp::create() );

	rop->execute();

	*/

	/*

	  create 2d transmittance texture (256x64, rgba_float_16, clamp, linear)
		for each pixel
			-> get r and view angle
				x:r goes from Rg to Rt
				y:muS goes from -0.15 to 1.0
			->compute optical depth
				which is a sum of opticalDepth for Rayleigh and Miescattering (multiplied by respective beta values)
			->compute transmittance by exp(-opticalDepth)
		computeOpticalDepth
			->intersect ray from camera to outerSphere and innerSphere
			->construct ray from closest intersection and divide ray length by number of sampling points
			->do raymarching
				->compute density at current height
				->add density to accumulated height
				->update position with raystep
			->return accumulated density

	  */


	// tmp for obj io:

	baseShader = base::Shader::load( base::Path( SRC_PATH ) + "/src/base/gfx/glsl/geometry_vs.glsl", base::Path( SRC_PATH ) + "/src/base/gfx/glsl/geometry_ps.glsl" );
	baseGeo = base::importObj( base::Path( SRC_PATH ) + "/data/test.1.obj" );
	//base::apply_transform( baseGeo, math::Matrix44f::ScaleMatrix( 30000.0f ) );
	base::apply_normals( baseGeo );

	baseTexture = base::Texture2d::load( base::Path( SRC_PATH ) + "/src/base/data/uvref2.png" );
	baseShader->setUniform( "input", baseTexture->getUniform() );


}


int main(int argc, char ** argv)
{
	QApplication app(argc, argv);
	app.setOrganizationName("test");
	app.setApplicationName("test");

	QMainWindow mainWin;
	mainWin.resize(800, 600);
	glviewer = new composer::widgets::GLViewer(init, render);
	glviewer->getCamera()->m_znear = .1f;
	glviewer->getCamera()->m_zfar = 100000.0f;
	mainWin.setCentralWidget( glviewer );
	//mainWin.show();

	s.RunUntilSigInt();


	return app.exec();
}

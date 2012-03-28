//============================================================================
//
//
//
//============================================================================



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
#include "ops/TransformOp.h"

#include <3rdparty\ip\TcpSocket.h>

base::GLViewer *glviewer;
base::ContextPtr context;





void render( base::CameraPtr cam )
{
}






void init()
{
	std::cout << "init!\n";


	GLenum glewResult = glewInit();
	if (GLEW_OK != glewResult)
	{
		std::cout << "glew init failed\n";
	}

	// connect to composer and retrieve scene
}

void shutdown()
{
}





int main(int argc, char ** argv)
{
	TcpSocket s;
	s.Connect( IpEndpointName("127.0.0.1", 12345) );

	base::Application app;
	glviewer = new base::GLViewer( 800, 600, "demo" );
	glviewer->show();
	return app.exec();
}

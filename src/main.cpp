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
#include <tinythread/tinythread.h>

base::GLViewer *glviewer;
base::ContextPtr context;

base::ops::OpPtr m_opRoot;



struct Application
{
	// global application scope operator graph ptr etc.
	// render function will access this

	void loadOperatorGraph( /*TODOBSONPtr operatorGraph*/ )
	{
		std::vector<base::ops::OpPtr> rootOps;

		// TODO::iterate operatorGraph bson object

		// if there is only one root op, then we take this as root
		if( rootOps.size() == 1 )
		{
			m_opRoot = rootOps[0];
		}else
		// else we put them into a sequence op
		{
			base::ops::SequenceOpPtr seq = base::ops::SequenceOp::create();
			for(std::vector<base::ops::OpPtr>::iterator it = rootOps.begin(); it != rootOps.end();++it )
				seq->plugLast( *it );
			m_opRoot = seq;
		}
	}

	void play()
	{
	}

	void stop()
	{
	}
	void setTime()
	{
	}
};
Application application;



struct TcpIpDriver
{
	tthread::thread *m_clientThread;
	TcpIpDriver()
	{
		// start client thread
		m_clientThread = new tthread::thread(client, this);
	}

	~TcpIpDriver()
	{
		delete m_clientThread;
	}

	static void client(void *arg)
	{
		TcpIpDriver *_this = (TcpIpDriver *)arg;
		char buffer[1000]; // 10mb
		std::cout << "connecting to server...";
		TcpSocket s;

		try
		{
			s.Connect( IpEndpointName("127.0.0.1", 12345) );
		}
		catch (...)
		{
			std::cout << "failed" << std::endl;
			return;
		}
		std::cout << "success" << std::endl;
	
		// wait for instructions
		int numBytesReceived = 0;
		numBytesReceived = s.Receive( buffer, 1000 );
		//while( (numBytesReceived = s.Receive( buffer, 1000 )) != SOCKET_ERROR )
		{
			std::cout << "received data... " << numBytesReceived << std::endl;
			//TODO: buffer now contains a bison object
			//base::BSONPtr data = BSON::unpack( buffer );
			//std::string command = data["command"];
			//if( command == "loadOperatorGraph" )
			//{
			//	application.loadOperatorGraph( data["operatorGraph"] );
			//}

			// examine bson

			// respond
			//std::string test = "deine mudda";
			//std::cout << "sending response..." << std::endl;
			//s.Send( test.c_str(), test.size() );
		};

	}
};


TcpIpDriver *appDriver;




void render( base::CameraPtr cam )
{
	m_opRoot->execute();
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
	appDriver = new TcpIpDriver();

	m_opRoot = base::ops::NOP::create();
}

void shutdown()
{
	delete appDriver;
}










int main(int argc, char ** argv)
{
	base::Application app;
	glviewer = new base::GLViewer( 800, 600, "demo", init, render );
	glviewer->show();
	return app.exec();
}

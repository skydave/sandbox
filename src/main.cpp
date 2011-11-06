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

#include <portaudio/portaudio.h>
#include <stblib/stb_vorbis.h>

#include "TimeOp.h"
#include "CameraOp.h"

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
base::Texture2dPtr baseTexture;
base::GeometryPtr baseGeo;

base::ops::OpPtr opRoot;
base::ops::ConstantOpPtr orbitTransform;


void render( base::CameraPtr cam )
{

	orbitTransform->m_variant = cam->m_transform;
	opRoot->execute();
}


void render2()
{
	context->render( geo, baseShader );
}



struct StreamData
{
	short *data;
	int len;
	int current;
};


PaStream *stream;


// This routine will be called by the PortAudio engine when audio is needed.
// It may called at interrupt level on some machines so don't do anything
// that could mess up the system like calling malloc() or free().
//
static int patestCallback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData )
{
	StreamData *streamData = (StreamData *)userData;
    short *out = (short*)outputBuffer;
    unsigned long i;

    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) inputBuffer;
    
    for( i=0; i<framesPerBuffer; i++ )
    {
		*out++ = streamData->data[streamData->current];  // left
        *out++ = streamData->data[streamData->current+1];  // right
		streamData->current += 2;
    }
    
    return paContinue;
}

//
// This routine is called by portaudio when playback is done.
//
static void StreamFinished( void* userData )
{
   StreamData *streamData = (StreamData *) userData;
   delete streamData;
   printf( "Stream Completed\n" );
} 


void init()
{
	std::cout << "init!\n";


	GLenum glewResult = glewInit();
	if (GLEW_OK != glewResult)
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
	base::ops::ClearOpPtr clear = base::ops::ClearOp::create();
	orbitTransform = base::ops::ConstantOp::create();
	base::ops::FuncOpPtr renderFunc = base::ops::FuncOp::create( render2 );

	clear->plug( cam );
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



	baseShader = base::Shader::load( base::Path( SRC_PATH ) + "/src/base/gfx/glsl/geometry_vs.glsl", base::Path( SRC_PATH ) + "/src/base/gfx/glsl/geometry_ps.glsl" );
	baseGeo = base::importObj( base::Path( SRC_PATH ) + "/data/test.1.obj" );
	//base::apply_transform( baseGeo, math::Matrix44f::ScaleMatrix( 30000.0f ) );
	base::apply_normals( baseGeo );

	baseTexture = base::Texture2d::load( base::Path( SRC_PATH ) + "/src/base/data/uvref2.png" );
	baseShader->setUniform( "input", baseTexture->getUniform() );


	// load ogg vorbis ============
	int channels;
	StreamData *streamData = new StreamData();
	streamData->current = 0;
	streamData->len = stb_vorbis_decode_filename("C:\\projects\\sandbox\\temp\\code\\sketch039.ogg", &channels, &streamData->data);

	if(!streamData->len)
		printf("error loading ogg file\n");
	printf("channels %i\n", channels);
	printf("len %i\n", streamData->len);


	// setup audio ===============
	PaStreamParameters outputParameters;

	PaError err;
  
    err = Pa_Initialize();

    outputParameters.device = Pa_GetDefaultOutputDevice(); // default output device
    if (outputParameters.device == paNoDevice)
	{
      fprintf(stderr,"Error: No default output device.\n");
    }
    outputParameters.channelCount = 2;       // stereo output
	outputParameters.sampleFormat = paInt16;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
              &stream,
              NULL, // no input
              &outputParameters,
              44100,
              64,
              paClipOff,      // we won't output out of range samples so don't bother clipping them
              patestCallback,
              streamData );

    err = Pa_SetStreamFinishedCallback( stream, &StreamFinished );

	// play..
    err = Pa_StartStream( stream );

}

void shutdown()
{
	PaError err;
	// stop
    err = Pa_StopStream( stream );

    err = Pa_CloseStream( stream );

    Pa_Terminate();
    printf("Test finished.\n");
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
	mainWin.setCentralWidget( glviewer );
	mainWin.show();



	oscDispatcher.start();

	return app.exec();
}

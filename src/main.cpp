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



#include "composer/widgets/CurveEditor/CurveEditor.h"
#include "composer/widgets/Trackball/Trackball.h"
#include "composer/widgets/GLViewer/GLViewer.h"
#include <ui_clouds.h>
#include "clouds.ui.h"

composer::widgets::CurveEditor *curveEditor;
composer::widgets::GLViewer *glviewer;

base::ContextPtr context;
base::GeometryPtr geo;
base::Texture1dPtr texture1d;
base::Texture2dPtr texture2d;
base::Texture2dPtr noisePermutationTableTex;
base::Texture3dPtr texture3d;
base::Texture2dPtr colorBuffer;
base::ShaderPtr shader;
base::ShaderPtr shader_screen;
base::ShaderPtr cloudShader;
base::FCurvePtr curve1;
base::FBOPtr fbo;
std::vector<math::Vec3f> positions;


base::ShaderPtr baseShader;
base::Texture2dPtr baseTexture;
base::GeometryPtr baseGeo;


float *clouds_parameters_tex;
base::Texture2dPtr clouds_parmameters;


extern char cloud_ps[];
extern int cloud_ps_size;
extern char cloud_vs[];
extern int cloud_vs_size;

void onPlayButtonPressed( bool checked )
{
	if( checked )
	{
		// continue timer
		// switch glviewer into threadrendering mode
		glviewer->setRenderInSeperateThread(true);
	}else
	{
		// switch glviewer into non-threadrendering mode
		glviewer->setRenderInSeperateThread(false);
		// pause timer
	}
}

void render( base::CameraPtr cam )
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadMatrixf( cam->m_projectionMatrix.ma );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadMatrixf( (GLfloat *)cam->m_viewMatrix.ma );

	// draw scene
	base::drawGrid(false);

	/*
	glPointSize(5.0f);

	int numSamples = 20;

	for(int i=0;i<numSamples;++i)
	{
		float x = i*1.0f/numSamples;
		float y = curve1->eval( x );
		glBegin( GL_POINTS );
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f( x, y, 0.0f );
		glEnd();
	}
	*/

	math::Matrix44f T_f3d = math::Matrix44f::TranslationMatrix( -1,0,0 )*math::Matrix44f::ScaleMatrix(5);
	math::Matrix44f T_f3d_invers = T_f3d;
	T_f3d_invers.invert();
	math::Matrix44f T = math::Matrix44f::TranslationMatrix( 1,0,0 ) * T_f3d;

	math::Matrix44f delta = T*T_f3d_invers;
	math::Matrix44f T_ = delta * T_f3d;

	if( T == T_ )
		std::cout << "ARE EQUAL\n";
	else
		std::cout << "ARE NOT EQUAL\n";

	base::drawTransform( delta );

	/*
	glBegin( GL_POINTS );
	glColor3f(1.0f, 0.0f, 0.0f);
	for( std::vector<math::Vec3f>::iterator it = positions.begin(); it != positions.end(); ++it )
	{
		math::Vec3f &v=*it;
		glVertex3f( v.x, v.y, v.z );
	}
	glEnd();
	*/


/*
	//
	base::AttributePtr a = geo->getAttr("P");

	switch( geo->primitiveType() )
	{
		default:
		case base::Geometry::POINT:
		{
			for( unsigned int i=0; i<geo->numPrimitives();++i )
			{
				glBegin( GL_POINTS );
				glColor3f(1.0f, 0.0f, 0.0f);
				math::Vec3f &v=a->get<math::Vec3f>(i);
				glVertex3f( v.x, v.y, v.z );
				glEnd();
			}
		}break;
		case base::Geometry::TRIANGLE:
		{
			std::vector<unsigned int>::iterator it = geo->m_indexBuffer.begin();
			std::vector<unsigned int>::iterator end = geo->m_indexBuffer.end();
			while( it != end )
			{
				glBegin( GL_TRIANGLES );
				glColor3f(1.0f, 0.0f, 0.0f);
				math::Vec3f &v0=a->get<math::Vec3f>(*it++);
				math::Vec3f &v1=a->get<math::Vec3f>(*it++);
				math::Vec3f &v2=a->get<math::Vec3f>(*it++);
				glVertex3f( v0.x, v0.y, v0.z );
				glVertex3f( v1.x, v1.y, v1.z );
				glVertex3f( v2.x, v2.y, v2.z );
				glEnd();
			}
		}break;
		case base::Geometry::QUAD:
		{
			std::vector<unsigned int>::iterator it = geo->m_indexBuffer.begin();
			std::vector<unsigned int>::iterator end = geo->m_indexBuffer.end();
			while( it != end )
			{
				glBegin( GL_QUADS );
				glColor3f(1.0f, 0.0f, 0.0f);
				math::Vec3f &v0=a->get<math::Vec3f>(*it++);
				math::Vec3f &v1=a->get<math::Vec3f>(*it++);
				math::Vec3f &v2=a->get<math::Vec3f>(*it++);
				math::Vec3f &v3=a->get<math::Vec3f>(*it++);
				glVertex3f( v0.x, v0.y, v0.z );
				glVertex3f( v1.x, v1.y, v1.z );
				glVertex3f( v2.x, v2.y, v2.z );
				glVertex3f( v3.x, v3.y, v3.z );
				glEnd();
			}
		}break;
	};
*/

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
}

void render2( base::CameraPtr cam )
{
	//fbo->begin(false);
	glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
	glDisable( GL_CULL_FACE );
	//glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );

	std::cout << "render2a " << cam->m_transform.getRight().x << " " << cam->m_transform.getRight().y << " " << cam->m_transform.getRight().z << std::endl;
	std::cout << "render2b " << cam->m_transform.getUp().x << " " << cam->m_transform.getUp().y << " " << cam->m_transform.getUp().z << std::endl;
	std::cout << "render2c " << cam->m_transform.getDir().x << " " << cam->m_transform.getDir().y << " " << cam->m_transform.getDir().z << std::endl;


	context->setView( cam->m_viewMatrix, cam->m_transform, cam->m_projectionMatrix );


	// render to screen
	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//context->renderScreen( shader_screen );
	//glEnable( GL_BLEND );
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//context->render( geo, cloudShader );
	context->render( baseGeo, cloudShader );
	//context->render( baseGeo, baseShader );

	/*
	// debug
	GLint viewport[4];
	//GLubyte pixel[3];
	float pixel[4];

	glGetIntegerv(GL_VIEWPORT,viewport);

	glReadPixels((int)(viewport[0] + viewport[2]*0.5f),(int)(viewport[3]-viewport[3]*0.5f),1,1, GL_RGBA,GL_FLOAT,(void *)pixel);

	std::cout << "pixel " << pixel[0] << " " << pixel[1] << " " << pixel[2] << " " << pixel[3] << std::endl;

	glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_TRUE);glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_TRUE);glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_TRUE);
	*/

	//fbo->end();
}

// angle in rad
float phase( float theta, const std::vector<float> &samples )
{
	// make sure theta is in range 0 to pi
	float angle = fabs(theta);
	if(angle > MATH_PIf)
		angle = MATH_2PIf - angle;

	// samples go from 0 to pi
	int index = (int)((angle/MATH_PI)*samples.size());
	if(index <0)
		index = 0;
	if(index >= samples.size())
		index = (int) samples.size() - 1;

	// TODO: linear interp?
	//std::cout << theta << " " << samples[index] << std::endl;

	return samples[index];
}



void updatePtheta( const std::string &id, const base::FCurve &curve  )
{
	int numSamples = 512;
	int numParameters = 2;
	for(int i =0; i<numSamples; ++i)
	{
		int row = numSamples*4;
		clouds_parameters_tex[row+i*4+1] = curve.eval((float)i/(float)numSamples);
	}
	clouds_parmameters->uploadRGBAFloat32(numSamples, numParameters, clouds_parameters_tex);


	//
	// compute Pf
	{
		float theta_f = math::degToRad(5.0f); // in rad
		// integrate one slice
		float A_slice = 0.0f;

		float theta = 0.0f;
		int numS = 500;
		for(int n=0; n<numS;++n)
		{
			if( theta < theta_f )
				A_slice += curve.eval((float)n/(float)numS)*sin(theta);

			theta += (MATH_PIf/numS);
		}
		A_slice = (MATH_PIf/numS)*A_slice;
		float Pf = 2.0f * MATH_PIf * A_slice;

		// normalize over solid angle of sphere
		Pf /= 4.0f*MATH_PIf;

		std::cout << "Pf=" << Pf << std::endl;
		cloudShader->setUniform( "Pf", Pf );
		cloudShader->setUniform( "theta_f", theta_f );
	}

	glviewer->update();
}

void updateSunDir( const math::Vec3f &vec  )
{
	//std::cout << vec.x << " " << vec.y << " " << vec.z << " " << vec.getLength() << std::endl;
	cloudShader->setUniform( "sunDir", vec );
	glviewer->update();
}


void init()
{
	std::cout << "init!\n";

	// get samples of Ptheta
	std::vector<float> P_theta_samples;

    std::string STRING;
	std::ifstream infile;
	infile.open( base::Path( SRC_PATH ) + "/data/mieplot_results1_phasefun.txt" );

    while(!infile.eof()) // To get you all the lines.
    {
	    std::getline(infile,STRING); // Saves the line in STRING.
		{
			std::vector<std::string> parts;
			//split string at whitespaces
			base::splitString( STRING, parts );

			if( parts.size() == 4 )
			{
				float intensity = base::fromString<float>(parts[2]);
				P_theta_samples.push_back(intensity);
			}
		}
    }
	infile.close();

	
	//P_theta.resize(300, 1.0f/12.57f);

	// compute volume

	// integrate one slice
	float A_slice = 0.0f;

	float theta = 0.0f;
	for(int n=0; n<P_theta_samples.size();++n)
	{
		A_slice += P_theta_samples[n]*sin(theta);

		theta += (MATH_PIf/P_theta_samples.size());
	}
	A_slice = (MATH_PIf/P_theta_samples.size())*A_slice;
	float V = 2.0f * MATH_PIf * A_slice;

	std::cout << "Volume is: " << V << std::endl;

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "glew init failed\n";
	}

	context = base::ContextPtr( new base::Context() );

	colorBuffer = base::Texture2d::createRGBAFloat32(512, 512);
	fbo = base::FBOPtr( new base::FBO( 512, 512 ) );
	fbo->setOutputs( colorBuffer );
	fbo->finalize();



	texture1d = base::Texture1d::createRGBA8();

	int xres = 128;
	unsigned char *tex = (unsigned char *)malloc(xres*4);
	float *texf = (float *)malloc(xres*1*sizeof(float));
	for(int i=0;i<xres;++i)
	{
		int index = i;
		tex[index*4] = 255 - (int)(((float)(i)/(float)(xres))*255.0f);
		tex[index*4+1] = (unsigned char)255;
		tex[index*4+2] = (unsigned char)0;
		tex[index*4+3] = (unsigned char)255;
		texf[i] = (float)i / (float)xres;
	}
	texture1d->uploadRGBA8( xres, tex );
	free(tex);
	free(texf);



	//texture2d = base::Texture2d::createRGBA8();
	texture2d = base::Texture2d::createRGBAFloat32();

	xres = 128;
	int yres = 128;
	tex = (unsigned char *)malloc(xres*yres*4);
	texf = (float *)malloc(xres*yres*4*sizeof(float));
	for(int j=0;j<yres;++j)
		for(int i=0;i<xres;++i)
		{
			int index = xres*j + i;
			tex[index*4] = (int)(((float)(i)/(float)(xres))*255.0f);
			tex[index*4+1] = (unsigned char)0;
			tex[index*4+2] = (unsigned char)0;
			tex[index*4+3] = (unsigned char)255;

			texf[index*4] = i/255.0f;
			texf[index*4+1] = 0.0f;
			texf[index*4+2] = 0.0f;
			texf[index*4+3] = 1.0f;
		}
	//texture2d->uploadRGBA8( xres, yres, tex );
	texture2d->uploadRGBAFloat32( xres, yres, texf );
	free(tex);
	free(texf);


	texture3d = base::Texture3d::createRGBA8();
	int zres = 128;
	tex = (unsigned char *)malloc(xres*yres*zres*4);
	for(int k=0;k<zres;++k)
		for(int j=0;j<yres;++j)
			for(int i=0;i<xres;++i)
			{
				int index = xres*yres*k + xres*j + i;
				tex[index*4] = (int)(((float)(i)/(float)(xres))*255.0f);
				tex[index*4+1] = (unsigned char)255;
				tex[index*4+2] = (unsigned char)0;
				tex[index*4+3] = (unsigned char)255;
			}
	texture3d->uploadRGBA8( xres, yres, zres, tex );
	free(tex);

	//geo = base::geo_pointCloud();
	//geo = base::geo_quad();
	//geo = base::geo_cube();
	geo = base::geo_grid( 250, 250 );
	//base::apply_transform( geo, math::Matrix44f::ScaleMatrix( 2000.0f ) );
	base::apply_transform( geo, math::Matrix44f::ScaleMatrix( 30000.0f ) );
	base::apply_normals( geo );




	//
	//cloudShader = base::Shader::load(cloud_vs, cloud_vs_size, cloud_ps, cloud_ps_size).attachPS( base::glsl::common() ).attachVS( base::glsl::common() );
	cloudShader = base::Shader::load(base::Path( SRC_PATH ) + "/src/cloud_vs.glsl", base::Path( SRC_PATH ) + "/src/cloud_ps.glsl").attachPS( base::glsl::common() ).attachVS( base::glsl::common() );
	cloudShader->setUniform( "input", texture2d->getUniform() );


	curve1 = base::FCurvePtr( new base::FCurve() );
	curve1->addCP(0.0f, 0.0f);
	curve1->addCP(0.5f, 0.5f);
	curve1->addCP(1.0f, 1.0f);






	context->setUniform("common_permTexture", base::glsl::noisePermutationTableTex()->getUniform());
	base::AttributePtr timeAttr = base::Attribute::createFloat();
	timeAttr->appendElement( 0.0f );
	context->setUniform("time", timeAttr);







	//
	// setup cloud shader
	//
	clouds_parmameters = base::Texture2d::createRGBAFloat32();

	base::FCurvePtr b;
	b = base::FCurvePtr( new base::FCurve() );
	b->addCP(1.0f - cos(math::degToRad(0.0f)), 1.1796f);
	b->addCP(1.0f - cos(math::degToRad(10.0f)), 1.1293f);
	b->addCP(1.0f - cos(math::degToRad(20.0f)), 1.1382f);
	b->addCP(1.0f - cos(math::degToRad(30.0f)), 1.0953f);
	b->addCP(1.0f - cos(math::degToRad(40.0f)), 0.9808f);
	b->addCP(1.0f - cos(math::degToRad(50.0f)), 0.9077f);
	b->addCP(1.0f - cos(math::degToRad(60.0f)), 0.7987f);
	b->addCP(1.0f - cos(math::degToRad(70.0f)), 0.6629f);
	b->addCP(1.0f - cos(math::degToRad(80.0f)), 0.5043f);
	b->addCP(1.0f - cos(math::degToRad(90.0f)), 0.3021f);
	base::FCurvePtr c;
	c = base::FCurvePtr( new base::FCurve() );
	c->addCP(1.0f - cos(math::degToRad(0.0f)), 0.0138f);
	c->addCP(1.0f - cos(math::degToRad(10.0f)), 0.0154f);
	c->addCP(1.0f - cos(math::degToRad(20.0f)), 0.0131f);
	c->addCP(1.0f - cos(math::degToRad(30.0f)), 0.0049f);
	c->addCP(1.0f - cos(math::degToRad(40.0f)), 0.0012f);
	c->addCP(1.0f - cos(math::degToRad(50.0f)), 0.0047f);
	c->addCP(1.0f - cos(math::degToRad(60.0f)), 0.0207f);
	c->addCP(1.0f - cos(math::degToRad(70.0f)), 0.0133f);
	c->addCP(1.0f - cos(math::degToRad(80.0f)), 0.0280f);
	c->addCP(1.0f - cos(math::degToRad(90.0f)), 0.0783f);
	base::FCurvePtr kc;
	kc = base::FCurvePtr( new base::FCurve() );
	kc->addCP(1.0f - cos(math::degToRad(0.0f)), 0.0265f);
	kc->addCP(1.0f - cos(math::degToRad(10.0f)), 0.0262f);
	kc->addCP(1.0f - cos(math::degToRad(20.0f)), 0.0272f);
	kc->addCP(1.0f - cos(math::degToRad(30.0f)), 0.0294f);
	kc->addCP(1.0f - cos(math::degToRad(40.0f)), 0.0326f);
	kc->addCP(1.0f - cos(math::degToRad(50.0f)), 0.0379f);
	kc->addCP(1.0f - cos(math::degToRad(60.0f)), 0.0471f);
	kc->addCP(1.0f - cos(math::degToRad(70.0f)), 0.0616f);
	kc->addCP(1.0f - cos(math::degToRad(80.0f)), 0.0700f);
	kc->addCP(1.0f - cos(math::degToRad(90.0f)), 0.0700f);
	base::FCurvePtr t;
	t = base::FCurvePtr( new base::FCurve() );
	t->addCP(1.0f - cos(math::degToRad(0.0f)), 0.8389f);
	t->addCP(1.0f - cos(math::degToRad(10.0f)), 0.8412f);
	t->addCP(1.0f - cos(math::degToRad(20.0f)), 0.8334f);
	t->addCP(1.0f - cos(math::degToRad(30.0f)), 0.8208f);
	t->addCP(1.0f - cos(math::degToRad(40.0f)), 0.8010f);
	t->addCP(1.0f - cos(math::degToRad(50.0f)), 0.7774f);
	t->addCP(1.0f - cos(math::degToRad(60.0f)), 0.7506f);
	t->addCP(1.0f - cos(math::degToRad(70.0f)), 0.7165f);
	t->addCP(1.0f - cos(math::degToRad(80.0f)), 0.7149f);
	t->addCP(1.0f - cos(math::degToRad(90.0f)), 0.1000f);
	base::FCurvePtr r;
	r = base::FCurvePtr( new base::FCurve() );
	r->addCP(1.0f - cos(math::degToRad(0.0f)), 0.0547f);
	r->addCP(1.0f - cos(math::degToRad(10.0f)), 0.0547f);
	r->addCP(1.0f - cos(math::degToRad(20.0f)), 0.0552f);
	r->addCP(1.0f - cos(math::degToRad(30.0f)), 0.0564f);
	r->addCP(1.0f - cos(math::degToRad(40.0f)), 0.0603f);
	r->addCP(1.0f - cos(math::degToRad(50.0f)), 0.0705f);
	r->addCP(1.0f - cos(math::degToRad(60.0f)), 0.0984f);
	r->addCP(1.0f - cos(math::degToRad(70.0f)), 0.1700f);
	r->addCP(1.0f - cos(math::degToRad(80.0f)), 0.3554f);
	r->addCP(1.0f - cos(math::degToRad(90.0f)), 0.9500f);
	base::FCurve P_theta(base::FCurve::LINEAR);
	//P_theta = base::FCurvePtr( new base::FCurve(base::FCurve::LINEAR) );
	theta = 0.0f;
	for(int n=0; n<P_theta_samples.size();++n)
	{
		P_theta.addCP(theta/MATH_PIf, P_theta_samples[n]);
		theta += (MATH_PIf/P_theta_samples.size());
	}


	int numSamples = 512;
	int numParameters = 2;
	clouds_parameters_tex = (float *)malloc(numSamples*numParameters*4*sizeof(float));
	for(int i =0; i<numSamples; ++i)
	{
		int row = numSamples*4;
		float one_minus_cos_theta = (float)i/(float)numSamples;
		float cos_theta = 1.0f - one_minus_cos_theta;
		clouds_parameters_tex[i*4] = b->eval(cos_theta);
		clouds_parameters_tex[i*4+1] = c->eval(cos_theta);
		clouds_parameters_tex[i*4+2] = kc->eval(cos_theta);
		clouds_parameters_tex[i*4+3] = t->eval(cos_theta);

		clouds_parameters_tex[row+i*4] = r->eval(cos_theta);
		clouds_parameters_tex[row+i*4+1] = P_theta.eval((float)i/(float)numSamples);
		//clouds_parameters_tex[row+i*4+1] = 0.1f;
		clouds_parameters_tex[row+i*4+2] = 0.0;
		clouds_parameters_tex[row+i*4+3] = 0.0;
	}
	clouds_parmameters->uploadRGBAFloat32(numSamples, numParameters, clouds_parameters_tex);



	//free(clouds_parameters_tex);

	cloudShader->setUniform( "parameters", clouds_parmameters->getUniform() );
	cloudShader->setUniform( "sunDir", math::Vec3f( 0.0f, 1.0f, 0.0f ) );
	cloudShader->setUniform( "maxHeight", 500.0f );
	cloudShader->setUniform( "maxVertexHeight", 500.0f );
	cloudShader->setUniform( "Csun", 1.0f, 1.0f, 1.0f, 1.0f );
	cloudShader->setUniform( "Csky", .5f, .5f, .5f, 1.0f );
	cloudShader->setUniform( "Cground", .1f, .1f, .1f, 1.0f );
	cloudShader->setUniform( "Ir1Mult", 1.0f );
	cloudShader->setUniform( "Ir2Mult", 1.0f );
	cloudShader->setUniform( "Ir3Mult", 1.0f );

	cloudShader->setUniform( "pn_frequency", 20.0f );
	cloudShader->setUniform( "pn_octaves", 8 );
	//
	// compute Pf
	{
		float theta_f = math::degToRad(5.0f); // in rad
		// integrate one slice
		float A_slice = 0.0f;

		float theta = 0.0f;
		int numS = 500;
		for(int n=0; n<numS;++n)
		{
			if( theta < theta_f )
				A_slice += P_theta.eval((float)n/(float)numS)*sin(theta);

			theta += (MATH_PIf/numS);
		}
		A_slice = (MATH_PIf/numS)*A_slice;
		float Pf = 2.0f * MATH_PIf * A_slice;

		// normalize over solid angle of sphere
		Pf /= 4.0f*MATH_PIf;

		std::cout << "Pf=" << Pf << std::endl;
		cloudShader->setUniform( "Pf", Pf );
		cloudShader->setUniform( "theta_f", theta_f );
	}

	// effective radius in micrometer (mm)
	//cloudShader->setUniform( "re", 7.0f );
	//cloudShader->setUniform( "re", 0.007f );
	//cloudShader->setUniform( "re", 0.7f );
	cloudShader->setUniform( "re", 0.000007f );
	//  in cm^-3
	//cloudShader->setUniform( "N0", 300.0f );
	//cloudShader->setUniform( "N0", 0.03f );
	cloudShader->setUniform( "N0", 300000000.0f );
	// beta
	cloudShader->setUniform( "beta", 0.9961f );

	//updatePtheta( "", P_theta );



	//base::ImagePtr img = base::Image::load( base::Path( SRC_PATH ) + "/data/noise1.jpg" );
	//base::Texture2dPtr tex = base::Texture2d::createRGBA8();
	//tex::


	base::FCurve fcurve(base::FCurve::LINEAR);
	int ttt=10;
	for(int i=0;i<ttt;++i)
	{
		float x = i*MATH_2PIf/ttt;
		fcurve.addCP( x, sin(x) );
	}



	CloudsUI *widget = new CloudsUI(P_theta);

	glviewer->connect( widget->ui.playButton, SIGNAL(clicked(bool)), SLOT(setRenderInSeperateThread(bool)) );

	widget->show();
	glviewer->connect( widget, SIGNAL(makeDirty(void)), SLOT(update(void)) );





	// tmp for obj io:

	baseShader = base::Shader::load( base::Path( SRC_PATH ) + "/src/base/gfx/glsl/geometry_vs.glsl", base::Path( SRC_PATH ) + "/src/base/gfx/glsl/geometry_ps.glsl" );
	//baseGeo = base::geo_grid( 10, 10 );
	baseGeo = base::importObj( base::Path( SRC_PATH ) + "/data/test.1.obj" );
	base::apply_transform( baseGeo, math::Matrix44f::ScaleMatrix( 30000.0f ) );
	base::apply_normals( baseGeo );

	baseTexture = base::Texture2d::load( base::Path( SRC_PATH ) + "/src/base/data/uvref2.png" );
	baseShader->setUniform( "input", baseTexture->getUniform() );
}


int main(int argc, char ** argv)
{
	//base::GLViewer window( 800, 600, "test", render2 );
	//window.getCamera()->m_znear = 1.0f;
	//window.getCamera()->m_zfar = 100000.0f;
	//window.show();
	//base::Application app;




	//init();


	



















	//return app.exec();


	//Q_INIT_RESOURCE(application);
	QApplication app(argc, argv);
	app.setOrganizationName("test");
	app.setApplicationName("test");

	QMainWindow mainWin;
	mainWin.resize(800, 600);
	glviewer = new composer::widgets::GLViewer(init, render2);
	glviewer->getCamera()->m_znear = .1f;
	glviewer->getCamera()->m_zfar = 100000.0f;
	mainWin.setCentralWidget( glviewer );
	mainWin.show();


	return app.exec();

}

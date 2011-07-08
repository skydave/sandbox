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
#include <gfx/Shader.h>
#include <gfx/Texture.h>
#include <gfx/Image.h>
#include <gfx/Context.h>
#include <gfx/FCurve.h>
#include <gfx/glsl/common.h>
#include <gfx/FBO.h>



#include "composer/widgets/GLViewer/GLViewer.h"

composer::widgets::GLViewer *glviewer;

base::ContextPtr context;
base::Texture2dPtr particlePositions;
base::Texture2dPtr particleTex;

base::Texture2dPtr  volumeBack;
base::Texture2dPtr volumeFront;
base::FBOPtr         volumeFBO;


base::GeometryPtr particles;
base::GeometryPtr sphere;
base::ShaderPtr particleShader;




int n_bands = 4;
int n_coeff = 15;


struct SHSample
{
	math::Vec3f sph;
	math::Vec3f vec;
	std::vector<float> coeff;

	SHSample() : coeff(n_coeff){}
};



double P(int l,int m,double x)
{
	// evaluate an Associated Legendre Polynomial P(l,m,x) at x
	double pmm = 1.0;
	if(m>0)
	{
		double somx2 = sqrt((1.0-x)*(1.0+x));
		double fact = 1.0;
		for(int i=1; i<=m; i++)
		{
			pmm *= (-fact) * somx2;
			fact += 2.0;
		}
	}
	if(l==m) return pmm;
	double pmmp1 = x * (2.0*m+1.0) * pmm;
	if(l==m+1) return pmmp1;
	double pll = 0.0;
	for(int ll=m+2; ll<=l; ++ll)
	{
		pll = ( (2.0*ll-1.0)*x*pmmp1-(ll+m-1.0)*pmm ) / (ll-m);
		pmm = pmmp1;
		pmmp1 = pll;
	}
	return pll;
}

int factorial(int number)
{
	int temp;

	if(number <= 1) return 1;

	temp = number * factorial(number - 1);
	return temp;
}

double K(int l, int m)
{
	// renormalisation constant for SH function
	double temp = ((2.0*l+1.0)*factorial(l-m)) / (4.0*MATH_PI*factorial(l+m));
	return sqrt(temp);
}

double SH(int l, int m, double theta, double phi)
{
	// return a point sample of a Spherical Harmonic basis function
	// l is the band, range [0..N]
	// m in the range [-l..l]
	// theta in the range [0..Pi]
	// phi in the range [0..2*Pi]
	const double sqrt2 = sqrt(2.0);
	if(m==0) return K(l,0)*P(l,m,cos(theta));
	else if(m>0) return sqrt2*K(l,m)*cos(m*phi)*P(l,m,cos(theta));
	else return sqrt2*K(l,-m)*sin(-m*phi)*P(l,-m,cos(theta));
}


void SH_setup_spherical_samples(std::vector<SHSample> &samples, int sqrt_n_samples)
{
	// fill an N*N*2 array with uniformly distributed
	// samples across the sphere using jittered stratification
	int i=0; // array index
	double oneoverN = 1.0/sqrt_n_samples;

	for(int a=0; a<sqrt_n_samples; a++)
	{
		for(int b=0; b<sqrt_n_samples; b++)
		{
			// generate unbiased distribution of spherical coords
			float x = (a + math::g_randomNumber()) * oneoverN; // do not reuse results
			float y = (b + math::g_randomNumber()) * oneoverN; // each sample must be random
			float theta = 2.0 * acos(sqrt(1.0 - x));
			float phi = 2.0 * MATH_PI * y;
			samples[i].sph = math::Vec3f(theta,phi,1.0);

			// convert spherical coords to unit vector
			math::Vec3f vec(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
			samples[i].vec = vec;


			// precompute all SH coefficients for this sample
			for(int l=0; l<n_bands; ++l)
			{
				for(int m=-l; m<=l; ++m)
				{
					int index = l*(l+1)+m;
					samples[i].coeff[index] = SH(l,m,theta,phi);
				}
			}

			++i;
		}
	}

}



typedef float (*SH_polar_fn)(float theta, float phi);

void SH_project_polar_function(SH_polar_fn fn, const std::vector<SHSample> &samples, std::vector<float> &result)
{
	const double weight = 4.0*MATH_PI;
	int n_samples = samples.size();
	result.resize( n_coeff );
	// for each sample
	for(int i=0; i<n_samples; ++i)
	{
		double theta = samples[i].sph.x;
		double phi = samples[i].sph.y;
		for(int n=0; n<n_coeff; ++n)
		{
			result[n] += fn(theta,phi) * samples[i].coeff[n];
		}
	}

	// divide the result by weight and number of samples
	double factor = weight / n_samples;
	for(int i=0; i<n_coeff; ++i)
	{
		result[i] = result[i] * factor;
	}
}

float fun_light( float theta, float phi )
{
	return std::max( 0.0f, 5.0f*cosf(theta) - 4.0f ) + std::max( 0.0f, -4.0f*sinf(theta-MATH_PI)*cosf(phi-2.5f)-3.0f );
}


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
	context->setView( cam->m_viewMatrix, cam->m_transform, cam->m_projectionMatrix );


	volumeFBO->begin();
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// render front faces

	// render back faces
	volumeFBO->end();

	glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );


	// render to screen
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	//glEnable( GL_BLEND );
	//glDisable( GL_DEPTH_TEST );


	//glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );
	//glTexEnvf( GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE );

	//glEnable( GL_POINT_SPRITE );

	//context->render( particles, particleShader );
	context->render( sphere, particleShader );

	//glDisable( GL_POINT_SPRITE );

	//glDisable( GL_BLEND );

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


	int a = 100;
	int numSHSamples = a*a;
	std::vector<SHSample> shSamples(numSHSamples);
	SH_setup_spherical_samples( shSamples, int(sqrt(numSHSamples)) );


	//particles = base::geo_blank(base::Geometry::POINT);
	//particles = base::geo_grid(14,14,base::Geometry::POINT);
	particles = base::Geometry::createPointGeometry();
	//particles = base::geo_sphere(14, 14, 1.0f, math::Vec3f(0.0f,0.0f,0.0f), base::Geometry::POINT);
	//particles = base::geo_sphere(28, 28, 1.0f, math::Vec3f(0.0f,0.0f,0.0f) );
	


	particlePositions = base::Texture2d::createRGBAFloat32( 1024, 1024 );
	float *posArray = (float *)malloc( 1024*1024*sizeof(float)*4 );

	base::ImagePtr img = base::Image::load( base::Path( SRC_PATH ) + "data/circlealpha.bmp" );
	particleTex = base::Texture2d::createRGBA8();
	particleTex->upload( img );


	particleShader = base::Shader::load( base::Path( SRC_PATH ) + "src/sh.vs.glsl", base::Path( SRC_PATH ) + "src/sh.ps.glsl" );
	particleShader->setUniform( "tex", particleTex->getUniform() );


	base::AttributePtr positions = particles->getAttr("P");

	for( int i =0; i<numSHSamples;++i )
	{
		//std::cout << shSamples[i].vec.x << " " << shSamples[i].vec.y << " " << shSamples[i].vec.z << std::endl;
		particles->addPoint( positions->appendElement(shSamples[i].vec) );
	}

	// compute sh coefficients
	std::vector<float> shCoeff;
	SH_project_polar_function( fun_light, shSamples, shCoeff );

	for( int i = 0;i<shCoeff.size(); ++i )
		std::cout << "coeff #" << i<< " " << shCoeff[i] << std::endl;

	base::AttributePtr shCoeffAttr = base::Attribute::createVec3f();
	for( int i = 0;i<9; ++i )
		shCoeffAttr->appendElement( math::Vec3f( shCoeff[i], shCoeff[i], shCoeff[i] ) );

	particleShader->setUniform( "Li", shCoeffAttr );

	// sh probe
	sphere = base::geo_sphere( 30, 30, 1.0f );
	base::apply_normals( sphere );


	// setup textures for volume rendering ===========================
	volumeBack = base::Texture2d::createRGBAFloat32( 512, 512 );
	volumeFront = base::Texture2d::createRGBAFloat32( 512, 512 );

	// setup offscreen render pass ===============
	volumeFBO = base::FBOPtr( new base::FBO( 512, 512 ) );
	volumeFBO->setOutputs( volumeFront, volumeBack );



}


int main(int argc, char ** argv)
{
	//Q_INIT_RESOURCE(application);
	QApplication app(argc, argv);
	app.setOrganizationName("test");
	app.setApplicationName("test");

	QMainWindow mainWin;
	mainWin.resize(800, 600);
	glviewer = new composer::widgets::GLViewer(init, render);
	glviewer->getCamera()->m_znear = 0.1f;
	glviewer->getCamera()->m_zfar = 100000.0f;
	mainWin.setCentralWidget( glviewer );
	mainWin.show();


	return app.exec();
}

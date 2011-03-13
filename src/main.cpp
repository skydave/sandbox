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

#include <ui/GLViewer.h>
#include <gltools/gl.h>
#include <gltools/misc.h>
#include <util/StringManip.h>
#include <gfx/Geometry.h>
#include <gfx/Shader.h>
#include <gfx/Context.h>

base::ContextPtr context;
base::GeometryPtr geo;
base::ShaderPtr shader;
std::vector<math::Vec3f> positions;

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

	glPointSize(5.0f);
/*
	int numSamples = 20;

	for(int i=0;i<numSamples;++i)
	{
		float y = i*1.0f/numSamples;
		float x = 0.0f;

		glBegin( GL_POINTS );
		glColor3f(1.0f, 0.0f, 0.0f);
		//glVertex3f( x, y, 0.0f );
		glEnd();
	}
	glBegin( GL_POINTS );
	glColor3f(1.0f, 0.0f, 0.0f);
	for( std::vector<math::Vec3f>::iterator it = positions.begin(); it != positions.end(); ++it )
	{
		math::Vec3f &v=*it;
		glVertex3f( v.x, v.y, v.z );
	}
	glEnd();
*/

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


	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
}

void render2( base::CameraPtr cam )
{

	glDisable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );

	context->setView( cam->m_viewMatrix, cam->m_transform, cam->m_projectionMatrix );


	// render to screen
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	context->renderScreen( shader );
	//context->render( geo, shader );
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

int main(int argc, char ** argv)
{
	base::GLViewer window( 800, 600, "test", render2 ); 
	window.show();
	base::Application app;

	// get samples of Ptheta
	std::vector<float> P_theta;
	//c:\projects\sandbox\git\data
    std::string STRING;
	std::ifstream infile;
	infile.open ("c:\\projects\\sandbox\\git\\data\\mieplot_results1_phasefun.txt");
	//infile.open ("/usr/people/david-k/dev/testprojects/sandbox/git/data/mieplot_results1_phasefun.txt");
	int lineCount = 0;
    while(!infile.eof()) // To get you all the lines.
    {
	    std::getline(infile,STRING); // Saves the line in STRING.
		//if(lineCount++ > 0)
		{
			std::vector<std::string> parts;
			//split string at whitespaces
			base::splitString( STRING, parts );
			/*
			if( parts.size() == 2 )
			{
				float t = base::fromString<float>(parts[0]);
				float intensity = base::fromString<float>(parts[1]);
				//std::cout<<STRING<<std::endl; // Prints our STRING.

				// map to range 10^-12,6 to 10^-6,4
				intensity = math::mapValueToRange( 0.0f, 1.0f, pow( 10, -11.5f ), pow( 10, -5.5f ), intensity );

				std::cout<<t << " " << intensity<<std::endl; // Prints our STRING.
				// apply invers log
				//intensity = pow( 10, intensity );
				intensity = exp( intensity );


				P_theta.push_back(intensity);
			}
			*/
			if( parts.size() == 4 )
			{
				float intensity = base::fromString<float>(parts[2]);
				//std::cout<<STRING<<std::endl; // Prints our STRING.

				// map to range 10^-12,6 to 10^-6,4
				//intensity = math::mapValueToRange( 0.0f, 1.0f, pow( 10, -11.5f ), pow( 10, -5.5f ), intensity );

				//std::cout<<t << " " << intensity<<std::endl; // Prints our STRING.
				// apply invers log
				//intensity = pow( 10, intensity );
				//intensity = exp( intensity );


				P_theta.push_back(intensity);
			}
		}
    }
	infile.close();

	
	//P_theta.resize(300, 1.0f/12.57f);

	// compute volume

	// integrate one slice
	float A_slice = 0.0f;

	float theta = 0.0f;
	for(int n=0; n<P_theta.size();++n)
	{
		A_slice += P_theta[n]*sin(theta);

		theta += (MATH_PIf/P_theta.size());
	}
	A_slice = (MATH_PIf/P_theta.size())*A_slice;
	float V = 2.0f * MATH_PIf * A_slice;

	std::cout << "Volume is: " << V << std::endl;

	//
	// sphere
	//
	/*
	int uSubdivisions = 20;
	int vSubdivisions = 20;
	float radius = 1.0f;
	math::Vec3f center;



	float dPhi = MATH_2PIf/uSubdivisions;
	float dTheta = MATH_PIf/vSubdivisions;
	float phi;

	//Geometry *result = new Geometry();

	//Attribute *positions = new Attribute();
	//result->setPAttr(positions);


	// y
	for (theta=MATH_PIf/2.0f+dTheta;theta<=(3.0f*MATH_PIf)/2.0f-dTheta;theta+=dTheta)
	{
		math::Vec3f p;
		float y = sin(theta);
		// x-z
		phi = 0.0f;
		for( int j = 0; j<uSubdivisions; ++j  )
		{
			p.x = cos(theta) * cos(phi);
			p.y = y;
			p.z = cos(theta) * sin(phi);

			p = p*radius*phase(theta, P_theta) + center;

			positions.push_back( p );
			phi+=dPhi;
		}
	}


	// add faces
	for( int j=0; j<vSubdivisions-3;++j )
	{
		int offset = j*(uSubdivisions);
		int i = 0;
		for( i=0; i<uSubdivisions-1; ++i )
		{
			//result->addTriangle(offset+i, offset+i + uSubdivisions, offset+i+1);
			//result->addTriangle(offset+i + uSubdivisions, offset+i+uSubdivisions+1, offset+i+1);
		}
		//result->addTriangle(offset+i,offset+i + uSubdivisions,offset+0);
		//result->addTriangle(offset+i + uSubdivisions,offset + uSubdivisions,offset);
	}
	//int pole1 = positions->appendElement( math::Vec3f(0.0f, 1.0f, 0.0f)*radius + center );
	//int pole2 = positions->appendElement( math::Vec3f(0.0f, -1.0f, 0.0f)*radius + center );
	for( int i=0; i<uSubdivisions-1; ++i )
	{
		//result->addTriangle(pole1, i, i+1);
		//result->addTriangle(pole2, uSubdivisions*(vSubdivisions-3)+i+1, uSubdivisions*(vSubdivisions-3)+i);
	}
	//result->addTriangle(pole1, uSubdivisions-1, 0);
	//result->addTriangle(pole2, uSubdivisions*(vSubdivisions-3), uSubdivisions*(vSubdivisions-2)-1);
	//return result;
	*/


	//
	// circle
	//
	int numSamples = 4000;
	//float center = -1.6f;
	float center = 2.64e-03f;

	for( int i=0; i< numSamples; ++i )
	{
		float t = ((float)(i)/(float)(numSamples))*MATH_2PIf;
		//float r = 1.0f;
		float r = phase(t, P_theta);
		r = log10(r+0.5f);
		r = r - center;
		math::Vec3f p;
		p.x = sin(t)*r;
		p.z = cos(t)*r;
		//if( r > .0f )
		positions.push_back(p);
	}


	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "glew init failed\n";
	}


	context = base::ContextPtr( new base::Context() );

	//geo = base::geo_pointCloud();
	//geo = base::geo_quad();
	geo = base::geo_grid( 50, 50 );




	shader = base::Shader::load( "c:\\projects\\sandbox\\git\\src\\base\\gfx\\glsl\\geometry_vs.glsl", "c:\\projects\\sandbox\\git\\src\\base\\gfx\\glsl\\geometry_ps.glsl" );


	return app.exec();
}

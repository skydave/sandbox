//============================================================================
//
//
// TODO: update to gl4.2 render grid/transform
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
#include <util/fs.h>

#include <gfx/Geometry.h>
#include <gfx/ObjIO.h>
#include <gfx/Shader.h>
#include <gfx/Texture.h>
#include <gfx/Image.h>
#include <gfx/Context.h>
#include <gfx/FCurve.h>
#include <gfx/glsl/common.h>
#include <gfx/FBO.h>

#include "SPH.h"
#include "SDF.h"
#include "Terrain.h"
#include "Visualizer.h"

#include "eigen/Eigen"

base::GLViewer    *glviewer;
base::ContextPtr    context;

base::GeometryPtr      grid;
base::ShaderPtr  greyShader;




// SPH stuff ======================
SPHPtr sph;
bool                            g_pause = true;
int                                 pciStep = 90;

// rendering
base::GeometryPtr             m_renderParticles;
base::ShaderPtr                m_particleShader;
std::vector<int>                    m_renderIds; // indicees of particles to render

// debug
base::GeometryPtr                        sdfGeo;
base::Texture2dPtr                        slice;
base::ShaderPtr                       sdfShader;

base::GeometryPtr          circle_supportRadius;
base::GeometryPtr                         point;

bool                  g_renderVisualiser = true;
VisualizerPtr                        visualizer;
Visualizer::Handle               m_stressVector;
Visualizer::Handle               m_visVector;




struct TerrainSDF
{
	TerrainPtr m_terrain;
	TerrainSDF()
	{
		m_terrain = Terrain::create( base::Path( SRC_PATH ) + "data/terrain_1_perlinnoise.tga" );
	}

	float operator()( const math::Vec3f &p )
	{
		math::Vec3f _p = p;


		// height map ==============================
		{
			float d = FLT_MAX;
			int tileResX = 20;
			int tileResY = 20;
			// iterate over all tiles and compute distance by approximating each tile with triangles
			// and using triangle distances
			float uInc = 1.0f/(float)tileResX;
			float vInc = 1.0f/(float)tileResY;
			for( int j=0;j<tileResY;++j )
				for( int i=0;i<tileResX;++i )
				{
					float u = (float)i/(float)tileResX;
					float v = (float)j/(float)tileResY;

					// get current triangles
					math::Vec3f p0, p1, p2, p3;
					p0 = math::Vec3f( u, m_terrain->getHeight( u, v ), v );
					p1 = math::Vec3f( u+uInc, m_terrain->getHeight( u+uInc, v ), v );
					p2 = math::Vec3f( u+uInc, m_terrain->getHeight( u+uInc, v+vInc ), v+vInc );
					p3 = math::Vec3f( u, m_terrain->getHeight( u, v+vInc ), v+vInc );
					d = std::min( d, math::distancePointTriangle( p, p0, p1, p2 ) );
					d = std::min( d, math::distancePointTriangle( p, p0, p2, p3 ) );
				}

			// determine sign by looking at the height of p
			float sign = 1.0f;
			if( p.y < m_terrain->getHeight( p.x, p.z ) )
				sign = -1.0f;

			//return d<0.01?1.0:0.0f;
			return sign*d;
		}
	}
};

struct PlaneSDF
{
	float operator()( const math::Vec3f &p )
	{
		float t = p.y - 0.5f;
		return t;
	}
};

base::GeometryPtr      terrainGeo;
base::ShaderPtr     terrainShader;


// update renderparticles
void updateParticles()
{
	base::AttributePtr positions = m_renderParticles->getAttr( "P" );
	base::AttributePtr colors = m_renderParticles->getAttr( "Cd" );
	for( std::vector<int>::iterator it = m_renderIds.begin(); it != m_renderIds.end();++it )
	{
		int &id =*it;
		SPH::Particle &p = sph->m_particles[id];
		positions->set<math::Vec3f>( id, sph->m_particles[id].position );
		colors->set<math::Vec3f>( id, sph->m_particles[id].color );
		/*
		float s = 30.0f;
		float tt = sph->m_particles[id].massDensity-sph->m_restDensity;
		float t = fabsf(tt)/s;
		math::Vec3f c;

		if( tt>0.0f )
			c = math::Vec3f( 1.0f, t, t );
		else
			c = math::Vec3f( t, 1.0f, t );

		colors->set<math::Vec3f>( id, c );

		*/
		//pciStep
		if( p.pciTrajectory )
		{
			if( (pciStep >= 0)&&(pciStep<p.pciTrajectory->m_steps.size()) )
			{
				positions->set<math::Vec3f>( id, p.pciTrajectory->m_steps[pciStep].predictedPosition );

				/*
				float f = fabsf(p.pciTrajectory->m_steps[pciStep].rho_err)/100.0f;
				std::cout << p.pciTrajectory->m_steps[pciStep].rho_err << std::endl;

				if( p.pciTrajectory->m_steps[pciStep].rho_err > 0.0f )
					colors->set<math::Vec3f>( id, math::Vec3f(f, 0.0f, 0.0f) );
				else
					colors->set<math::Vec3f>( id, math::Vec3f(0.0f,f, 0.0f) );
				*/
			}
		}

	}

}

void step()
{
	sph->advance();

	// update renderparticles
	updateParticles();

	/*
	// update debug info
	static int check = 0;
	if( ++check % 1000 )
	{
		sph->updateTrajectories();

		for( int i=0;i<sph->m_numParticles;++i )
		{
			SPH::Particle &p = sph->m_particles[i];
			if( p.trajectory )
			{
				visualizer->color( 0.1f, 0.8f, 0.1f );
				visualizer->line(p.positionPrev, p.position);
				visualizer->point(p.position);
			}
		}
	}
	*/

	//visualizer->line( m_stressVector, sph->m_particles[81].position, sph->m_particles[81].position + sph->m_particles[81].pciFrictionForce);


	// test for visualizing strain tensor
	if(1)
	{
		if(0)
		{
			SPH::Particle &p = sph->m_particles[0];

			SPH::Tensor pt = p.strainRate;
			Eigen::Matrix<float, 2, 2> t;
			t( 0, 0 ) = pt.m[0][0];
			t( 0, 1 ) = pt.m[0][1];
			t( 1, 0 ) = pt.m[1][0];
			t( 1, 1 ) = pt.m[1][1];

			Eigen::EigenSolver<Eigen::Matrix<float, 2, 2> > es( t, true );

			float ev0 = es.eigenvalues()(0).real();
			float ev1 = es.eigenvalues()(1).real();

			math::Vec3f e0( es.eigenvectors().col(0)(0).real(), es.eigenvectors().col(0)(1).real(), 0.0f);
			math::Vec3f e1( es.eigenvectors().col(1)(0).real(), es.eigenvectors().col(1)(1).real(), 0.0f);

			//
			visualizer->line( p.strainRateVisHandle1, p.position, p.position + e0*ev0*100000.0f);
			visualizer->line( p.strainRateVisHandle2, p.position, p.position + e1*ev1*100000.0f);
		}

		//visualizer->line( m_stressVector, p.position, p.position + p.pciFrictionForce*1000.0f);
		SPH::Particle &p2 = sph->m_particles[1];
		//visualizer->line( m_visVector, p2.position, p2.position + p2.pciFrictionForce*1000.0f);
		//std::cout << "pciFrictionForce: " << p.pciFrictionForce.x << " " << p.pciFrictionForce.y << " " << p.pciFrictionForce.z << std::endl;
		//std::cout << "eigenvalues are: " << ev0 << " " << ev1 << std::endl;
		//std::cout << "strain rate: " << pt.m[0][0] << " " << pt.m[0][1] << std::endl;
		//std::cout << "             " << pt.m[1][0] << " " << pt.m[1][1] << std::endl;
		for( int i=0;i<sph->numParticles();++i )
		{
			SPH::Particle &pp = sph->m_particles[i];
			visualizer->line( pp.frictionForceVisHandle, pp.position, pp.position + pp.pciFrictionForce*1.001f);

			/*
			for( int j=0;j<pp.neighbours.size();++j )
			{
				SPH::Particle::Neighbour &n = pp.neighbours[j];
				SPH::Particle::NeighbourDebug &nd = pp.neighbourDebug[j];
				//visualizer->line( nd.gradVisHandle, pp.position, pp.position+nd.gradW*0.0001f);
			}
			*/

		}
	}



	glviewer->setCaption( base::toString(sph->m_currentTimeStep) + " " +  base::toString(pciStep) );
}



void render( base::CameraPtr cam )
{
	// put rendering code here
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	context->setCamera( cam );

	glEnable( GL_DEPTH_TEST );
	//context->render( grid, greyShader );

	//context->renderScreen( heightMap->m_heightMap );
	//context->render( heightMap->m_previewGeometry, heightMap->m_shader );
	if( terrainGeo )
		context->render( terrainGeo, terrainShader );
	if( sdfGeo )
		context->render( sdfGeo, sdfShader );
	glDisable( GL_DEPTH_TEST );




	// advance sph system
	if( !g_pause )
		step();


	// render sph
	glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );
	glTexEnvf( GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE );
	glEnable( GL_POINT_SPRITE );
	context->render( m_renderParticles, m_particleShader );
	glDisable( GL_POINT_SPRITE );


	// render debug stuff
	if(g_renderVisualiser)
		visualizer->render();
	int idx = 0;
	if( 22 < sph->m_particles.size() )
		idx = 22;
	context->render( circle_supportRadius, context->m_constantShader, math::Matrix44f::RotationMatrixX(math::degToRad(90.0f))*math::Matrix44f::ScaleMatrix( sph->m_supportRadius )*math::Matrix44f::TranslationMatrix(sph->m_particles[idx].position)  );
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
	base::Context::setCurrentContext(context);

	// put your initialization stuff here
	grid = base::geo_grid( 5, 5, base::Geometry::LINE );
	//base::apply_transform( grid, math::Matrix44f::RotationMatrixX( math::degToRad(90.0f) ) );
	greyShader = base::Shader::createSimpleConstantShader( 0.6f, 0.6f, 0.6f );


	// initialize sph system =============================
	sph = SPH::create();
	sph->initialize();

	///*
	// test for checking support radius
	base::fs::File *f_W_poly6_2d = base::fs::open( "W_poly6_2d.dat", "w" );
	base::fs::File *f_gradW_spiky_2d = base::fs::open( "gradW_spiky_2d.dat", "w" );
	base::fs::File *f_W_spiky_2d = base::fs::open( "W_spiky_2d.dat", "w" );
	

	int numSamples = 100;
	for( int i=0;i<numSamples;++i )
	{
		float t = (float)i/(float)numSamples;
		float d = t * sph->m_supportRadius;
		float W_poly6_2d = sph->W_poly6_2d(d);
		float W_spiky_2d = sph->W_spiky_2d(d);
		//float gradW_spiky_2d = sph->gradW_spiky_2d(d);

		base::fs::write( f_W_poly6_2d, base::toString(d) + " " + base::toString(W_poly6_2d) + "\n" );
		base::fs::write( f_W_spiky_2d, base::toString(d) + " " + base::toString(W_spiky_2d) + "\n" );
		//base::fs::write( f_gradW_spiky_2d, base::toString(d) + " " + base::toString(gradW_spiky_2d) + "\n" );
	}

	base::fs::close(f_W_poly6_2d);
	base::fs::close(f_W_spiky_2d);
	base::fs::close(f_gradW_spiky_2d);
	//*/



	// add collisionobject (SDF)
	//TerrainSDF terrainSDF;
	PlaneSDF terrainSDF;
	Domain m_sdfWindow; // defines the window in sdf's local space which we are sampling

	// define region in sdf local space and sampling rate
	m_sdfWindow.bound = math::BoundingBox3d( math::Vec3f( 0.0f, 0.0f, 0.0f ), math::Vec3f( 1.0f, 1.0f, 1.0f ) );
	m_sdfWindow.width = 30;
	m_sdfWindow.height = 30;
	m_sdfWindow.depth = 30;
	// distance values will now be in sdf local space
	ScalarFieldPtr sdf1 = distanceTransform( m_sdfWindow, terrainSDF );

	// TODO: sdf1->transform( ... );

	sph->addCollider( sdf1 );






	// setup rendering ======================
	// particles
	m_renderIds.clear();
	m_renderParticles = base::Geometry::createPointGeometry();
	base::AttributePtr positions = m_renderParticles->getAttr("P");
	m_renderParticles->setAttr( "Cd", base::Attribute::createVec3f((int)sph->m_particles.size()) );
	int index = 0;
	for( SPH::ParticleContainer::iterator it = sph->m_particles.begin(); it != sph->m_particles.end();++it, ++index )
	{
		m_renderParticles->addPoint(positions->appendElement(math::Vec3f()));
		m_renderIds.push_back( index);
	}

	m_particleShader = base::Shader::load( base::Path( SRC_PATH ) + "src/SPH.particle" );
	m_particleShader->setUniform( "scale", 10.0f );
	m_particleShader->setUniform( "alpha", 1.0f );

	/*
	// terrain (collider)
	terrainGeo = Terrain::createGeometry( (base::Path( SRC_PATH ) + "data/terrain_1_perlinnoise.tga").str(), 30, 30 );
	//base::apply_transform( terrainGeo, math::Matrix44f::TranslationMatrix(0.5f, 0.0f, 0.5f) );
	terrainShader = base::Shader::createSimpleLambertShader();
	*/


	/*
	// sdf test
	sdfGeo = base::geo_quad();
	//base::apply_transform( sdfGeo, math::Matrix44f::ScaleMatrix(2.0f)*math::Matrix44f::TranslationMatrix(0.0f, 0.0f, 0.0f) );
	//slice = sdf1.createSlice( math::Vec2f(-2.0f, -2.0f), math::Vec2f(2.0f, 2.0f), 512, 512 );
	base::apply_transform( sdfGeo, math::Matrix44f::ScaleMatrix(0.5f)*math::Matrix44f::TranslationMatrix(0.0f, 0.5f, 0.0f) );
	slice = sdf1->createSlice( math::Vec2f(-0.5f, 0.0f), math::Vec2f(.5f, 1.0f), 0.5f, 512, 512 );
	sdfShader = base::Shader::createSimpleTextureShader(slice);
	*/


	// debug stuff
	//circle_supportRadius = base::geo_circle( 20, sph->m_supportRadius );
	circle_supportRadius = base::geo_circle( 20, 1.0f );
	//base::apply_transform(circle_supportRadius, math::Matrix44f::RotationMatrixX( math::degToRad(90.0f) ));



	visualizer = Visualizer::create();
	//m_stressVector = visualizer->line( math::Vec3f(0.0f, 0.0f, 0.0f), math::Vec3f(1.0f, 0.0f, 0.0f) );
	//m_visVector = visualizer->line( math::Vec3f(0.0f, 0.0f, 0.0f), math::Vec3f(1.0f, 0.0f, 0.0f) );
	visualizer->point( math::Vec3f(0.0f, 0.0f, 0.0f) );
	for( int i=0;i<sph->numParticles();++i )
	{
		SPH::Particle &p = sph->m_particles[i];
		p.frictionForceVisHandle = visualizer->line( p.position, p.position );
		/*
		for( int i=0;i<p.neighbourDebug.size();++i )
		{
			SPH::Particle::NeighbourDebug &nd = p.neighbourDebug[i];
			nd.gradVisHandle = visualizer->line( math::Vec3f(0.0f, 0.0f, 0.0f), math::Vec3f(0.0f, 0.0f, 0.0f));
		}
		*/
	}
	//visualizer->color( 0.7f, 0.9f, 0.7f );
	visualizer->color( 1.9f, 0.0f, 0.0f );
	sph->m_particles[1].strainRateVisHandle1 = visualizer->line( sph->m_particles[1].position, sph->m_particles[1].position );
	sph->m_particles[1].strainRateVisHandle2 = visualizer->line( sph->m_particles[1].position, sph->m_particles[1].position );
}

void shutdown()
{
	/*
	// put your deinitialization stuff here
	base::fs::File *f_0_pressure = base::fs::open( "0_pressure.dat", "w" );
	base::fs::File *f_0_massDensity = base::fs::open( "0_massDensity.dat", "w" );
	

	int numSamples = sph->m_particles[0].trajectory->m_steps.size();
	for( int i=0;i<numSamples;++i )
	{
		SPH::Particle &step = sph->m_particles[0].trajectory->m_steps[i];
		base::fs::write( f_0_pressure, base::toString(i) + " " + base::toString(step.pressure) + "\n" );
		base::fs::write( f_0_massDensity, base::toString(i) + " " + base::toString(step.massDensity) + "\n" );
	}

	base::fs::close(f_0_pressure);
	base::fs::close(f_0_massDensity);
	*/
	///*

	//*/
}


void keypress( int key )
{
	if( key == KEY_SPACE )
		g_pause = !g_pause;
	if( key == KEY_V )
		g_renderVisualiser = !g_renderVisualiser;
	if( key == KEY_RIGHT )
		step();
	if( key == KEY_N )
	{
		pciStep--;
		updateParticles();
		glviewer->setCaption( base::toString(sph->m_currentTimeStep) + " " +  base::toString(pciStep) );
	}
	if( key == KEY_M )
	{
		pciStep++;
		updateParticles();
		glviewer->setCaption( base::toString(sph->m_currentTimeStep) + " " +  base::toString(pciStep) );
	}

	if( key == KEY_S )
	{
		/*
		std::vector<int> indices;
		indices.push_back( 0 );
		indices.push_back( 22 );
		indices.push_back( 9);
		indices.push_back( 99 );
		for( std::vector<int>::iterator it = indices.begin(); it != indices.end(); ++it )
		{
			int index = *it;
			if( (index < sph->m_particles.size())&&(sph->m_particles[index].pciTrajectory) )
			{
				base::fs::File *f_pressure = base::fs::open( "f_" + base::toString(index) + "_pressure.dat", "w" );
				base::fs::File *f_rho_err = base::fs::open( "f_" + base::toString(index) + "_rho_err.dat", "w" );
				base::fs::File *f_predictedMassDensity = base::fs::open( "f_" + base::toString(index) + "_predictedMassDensity.dat", "w" );
				base::fs::File *f_pciPressureForceMag = base::fs::open( "f_" + base::toString(index) + "_pciPressureForceMag.dat", "w" );
				base::fs::File *f_PredictedPosX = base::fs::open( "f_" + base::toString(index) + "_predictedPosX.dat", "w" );
				base::fs::File *f_PredictedPosY = base::fs::open( "f_" + base::toString(index) + "_predictedPosY.dat", "w" );
				base::fs::File *f_PredictedPosZ = base::fs::open( "f_" + base::toString(index) + "_predictedPosZ.dat", "w" );
				base::fs::File *f_dd = base::fs::open( "f_" + base::toString(index) + "_dd.dat", "w" );
	

				int numSamples = sph->m_particles[index].pciTrajectory->m_steps.size();
				for( int i=0;i<numSamples;++i )
				{
					SPH::Particle &step = sph->m_particles[index].pciTrajectory->m_steps[i];
					base::fs::write( f_pressure, base::toString(i) + " " + base::toString(step.pressure) + "\n" );
					base::fs::write( f_rho_err, base::toString(i) + " " + base::toString(step.rho_err) + "\n" );
					base::fs::write( f_predictedMassDensity, base::toString(i) + " " + base::toString(step.predictedMassDensity) + "\n" );
					base::fs::write( f_pciPressureForceMag, base::toString(i) + " " + base::toString(step.pciPressureForce.getLength()) + "\n" );

					base::fs::write( f_PredictedPosX, base::toString(i) + " " + base::toString(step.predictedPosition.x) + "\n" );
					base::fs::write( f_PredictedPosY, base::toString(i) + " " + base::toString(step.predictedPosition.y) + "\n" );
					base::fs::write( f_PredictedPosZ, base::toString(i) + " " + base::toString(step.predictedPosition.z) + "\n" );

					base::fs::write( f_dd, base::toString(i) + " " + base::toString(step.dd) + "\n" );

				}

				base::fs::close(f_pressure);
				base::fs::close(f_rho_err);
				base::fs::close(f_predictedMassDensity);
				base::fs::close(f_pciPressureForceMag);
				base::fs::close(f_PredictedPosX);
				base::fs::close(f_PredictedPosY);
				base::fs::close(f_PredictedPosZ);
				base::fs::close(f_dd);
			}
		}
		*/

		// save positions
		{
			base::fs::File *f_p = base::fs::open( "f_p.dat", "w" );

			// numParticles
			base::fs::write( f_p, base::toString(sph->m_particles.size()) + "\n" );

			for( std::vector<SPH::Particle>::iterator it = sph->m_particles.begin(); it != sph->m_particles.end(); ++it)
			{
				SPH::Particle &p = *it;
				// positions
				base::fs::write( f_p, base::toString(p.position.x) + " " + base::toString(p.position.y) + " " + base::toString(p.position.z) + "\n" );
			}

			base::fs::close(f_p);
		}
	}
}


int main(int argc, char ** argv)
{
	base::Application app;
	glviewer = new base::GLViewer( 800, 600, "app", init, render );
	glviewer->getCamera()->m_znear = 0.0001f;
	glviewer->getCamera()->update();
	glviewer->show();
	glviewer->setKeyPressCallback( keypress );
	glviewer->setShutdownCallback( shutdown );
	return app.exec();
}

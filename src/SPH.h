#pragma once

#include <vector>

#include <math/Math.h>
#include <util/shared_ptr.h>
#include <gfx/Geometry.h>
#include <gfx/Shader.h>

#include "SDF.h"




BASE_DECL_SMARTPTR_STRUCT(SPH);
struct SPH
{
	struct Particle
	{
		math::Vec3f                                          position;
		math::Vec3f                                      positionPrev;
		math::Vec3f                                          velocity;
		math::Vec3f                                      acceleration;
		float                                                    mass; // in kg
		float                                             massDensity; // in kg/m³
		float                                                pressure; // in ? pascal ?

		typedef std::vector< std::pair<float, Particle*> > Neighbours;
		Neighbours                                         neighbours; // list of particles within support radius and their distances; computed once per timestep

		// used for debugging:
		int                                                        id;
		math::Vec3f                                             color;
	};



	// public
	static SPHPtr                             create();
	void                                  initialize();
	void             addCollider( ScalarFieldPtr sdf );
	void                                     advance();


	// private
	void                             timeIntegration();
	void                            handleCollisions();

	void updateSupportRadius( float newSupportRadius );



	// simulation scene content
	typedef std::vector<Particle>    ParticleContainer;
	ParticleContainer                      m_particles;
	ScalarFieldPtr                          m_collider;


	// material properties
	float                           m_idealGasConstant; // nRT - expresses amount of substance per mol plus temperature
	float                                m_restDensity;



	float                              m_supportRadius;
	int                                 m_numParticles;
	float                                   m_timeStep;

	// internal
	float                    m_supportRadiusSquared;


};







// weighting functions =========================

//6th degree polynomial
void W_poly6_precompute( float supportRadius );
float W_poly6( float distance );


//spiky (gradient doesnt vanish near center)
void W_spiky_precompute( float supportRadius );
float W_spiky( float distance );
math::Vec3f gradW_spiky( float distance, math::Vec3f dir );
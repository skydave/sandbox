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
		struct Neighbour
		{
			float distance;
			Particle    *p;

			// used for predictive-correction-scheme
			float predictedDistance;
		};
		math::Vec3f                                          position;
		math::Vec3f                                      positionPrev;
		math::Vec3f                                          velocity;
		math::Vec3f                                            forces; // combined external and internal forces
		float                                                    mass; // in kg
		float                                             massDensity; // in kg/m³
		float                                                pressure; // in ? pascal ?

		typedef std::vector< Neighbour > Neighbours;
		Neighbours                                         neighbours; // list of particles within support radius and their distances; computed once per timestep

		// used for predictive-correction-scheme
		math::Vec3f                                 predictedPosition;
		float                                    predictedMassDensity;
		math::Vec3f                                  pciPressureForce;

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

	// solver parameters
	float                               m_particleMass; // in kg
	float                              m_supportRadius;
	float                                   m_timeStep;
	float                                    m_damping;

	float                                   m_pciDelta; // used for PCISPH



	int                                 m_numParticles;

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
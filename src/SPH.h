#pragma once

#include <vector>

#include <math/Math.h>
#include <util/shared_ptr.h>
#include <util/Flags.h>
#include <gfx/Geometry.h>
#include <gfx/Shader.h>

#include "SDF.h"




BASE_DECL_SMARTPTR_STRUCT(SPH);
struct SPH
{
	struct Trajectory;
	struct Particle
	{
		struct Neighbour
		{
			float distance;
			Particle    *p;

			// used for predictive-correction-scheme
			float predictedDistance;
		};

		enum States
		{
			STATE_NONE,
			STATE_BOUNDARY
		};

		Particle();                                                    // constructor

		base::Flags<States>                                    states;

		math::Vec3f                                          position;
		math::Vec3f                                      positionPrev;
		math::Vec3f                                          velocity;
		math::Vec3f                                            forces; // combined external and internal forces
		math::Vec3f                                          impulses; // combined impulses (comming from boundary condition)
		float                                                    mass; // in kg
		float                                             massDensity; // in kg/m³
		float                                                pressure; // in ? pascal ?

		typedef std::vector< Neighbour > Neighbours;
		Neighbours                                         neighbours; // list of particles within support radius and their distances; computed once per timestep

		// used for predictive-correction-scheme
		math::Vec3f                                 predictedPosition;
		math::Vec3f                             predictedPositionPrev;
		math::Vec3f                                 predictedVelocity;
		float                                    predictedMassDensity;
		math::Vec3f                                  pciPressureForce;
		// used for granular material
		math::Vec3f                                  pciFrictionForce; // friction force is computed within PCI scheme (no PCI, no friction)
		math::Matrix33f                                  stressTensor;
		// boundary handling
		math::Vec3f                                  pciBoundaryForce;
		math::Vec3f                                pciBoundaryImpulse;

		// used for debugging:
		Trajectory                                        *trajectory;
		int                                                        id;
		math::Vec3f                                             color;

		math::Vec3f                                             temp1;
	};


	// list of all steps for one particle
	struct Trajectory
	{
		std::vector<Particle> m_steps;
	};



	// public
	static SPHPtr                                                   create();
	void                                                        initialize();
	void      initializeParticle( Particle &p, const math::Vec3f &position );
	void                                   addCollider( ScalarFieldPtr sdf );
	void                                                           advance();
	void                                                updateTrajectories(); // just for debugging


	// private
	void                                                   timeIntegration();

	void                       updateSupportRadius( float newSupportRadius );



	// simulation scene content===========================
	typedef std::vector<Particle>    ParticleContainer;
	ParticleContainer                      m_particles;
	ScalarFieldPtr                          m_collider;


	// material properties================================
	float                           m_idealGasConstant; // nRT - expresses amount of substance per mol plus temperature
	float                                m_restDensity;

	// granular
	float                            m_cricitalDensity;
	float                        m_frictionCoefficient;

	// solver parameters =================================
	float                               m_particleMass; // in kg
	float                              m_supportRadius;
	float                                   m_timeStep;
	float                                    m_damping;

	// PCISPH
	float                                   m_pciDelta;
	// granular
	math::Matrix33f            m_pciStressDeltaInverse;

	// switches for different solvertypes----
	bool                 m_unilateralIncompressibility;
	bool                                    m_friction;



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
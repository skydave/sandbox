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
	typedef math::Matrix22f Tensor;
	typedef math::Vec3f Vector;
	typedef float Real;

	struct Trajectory;
	struct Particle
	{
		struct NeighbourDebug
		{
			Vector gradW;
			size_t gradVisHandle;
		};

		struct Neighbour
		{
			Real distance;
			Particle    *p;

			// used for predictive-correction-scheme
			Real predictedDistance;

			NeighbourDebug *nd;
		};

		enum States
		{
			STATE_NONE,
			STATE_BOUNDARY
		};

		Particle();                                                    // constructor

		base::Flags<States>                                    states;

		Vector                                          position;
		Vector                                      positionPrev;
		Vector                                          velocity;
		Vector                                      velocityPrev;
		Vector                                            forces; // combined external and internal forces
		Vector                                          impulses; // combined impulses (comming from boundary condition)
		Real                                                    mass; // in kg
		Real                                             massDensity; // in kg/m³
		Real                                                pressure; // in ? pascal ?

		typedef std::vector< Neighbour > Neighbours;
		Neighbours                                         neighbours; // list of particles within support radius and their distances; computed once per timestep

		// used for predictive-correction-scheme
		Vector                                 predictedPosition;
		Vector                             predictedPositionPrev;
		Vector                                 predictedVelocity;
		Real                                    predictedMassDensity;
		Vector                                  pciPressureForce;
		// used for granular material
		Vector                                  pciFrictionForce; // friction force is computed within PCI scheme (no PCI, no friction)
		//Matrix33f                                  stressTensor;
		Tensor                                           stressTensor;
		// boundary handling
		Vector                                  pciBoundaryForce;
		Vector                                pciBoundaryImpulse;

		// used for debugging:
		Trajectory                                        *trajectory;
		size_t                                                     id;
		Vector                                             color;

		std::vector<NeighbourDebug>                    neighbourDebug;
		Vector                                             temp1;
		Vector                                             temp2;
		size_t                                                  frictionForceVisHandle;
		size_t                                                  strainRateVisHandle1;
		size_t                                                  strainRateVisHandle2;
		Tensor                                             strainRate;
	};


	// list of all steps for one particle
	struct Trajectory
	{
		std::vector<Particle> m_steps;
	};



	// public
	SPH();                                                                    // constructor
	static SPHPtr                                                   create();
	void                                                        initialize();
	void      initializeParticle( Particle &p, const Vector &position );
	void                                   addCollider( ScalarFieldPtr sdf );
	void                                                           advance();
	size_t                                               numParticles()const;
	void                                                updateTrajectories(); // just for debugging



	// private
	void                                                   timeIntegration();

	void                       updateSupportRadius( Real newSupportRadius );



	// simulation scene content===========================
	typedef std::vector<Particle>    ParticleContainer;
	ParticleContainer                      m_particles;
	ScalarFieldPtr                          m_collider;


	// material properties================================
	Real                           m_idealGasConstant; // nRT - expresses amount of substance per mol plus temperature
	Real                                m_restDensity;

	// granular
	Real                            m_cricitalDensity;
	Real                        m_frictionCoefficient;

	// solver parameters =================================
	Real                               m_particleMass; // in kg
	Real                              m_supportRadius;
	Real                                   m_timeStep;
	Real                                    m_damping;

	// PCISPH
	Real                                   m_pciDelta;
	// granular
	Tensor                     m_pciStressDeltaInverse;

	// switches for different solvertypes----
	bool                 m_unilateralIncompressibility;
	bool                                    m_friction;
	bool                                    m_pressure;
	bool                                    m_boundary;
	bool                                     m_gravity;
	bool                                  m_deformtest;



	// internal
	Real                    m_supportRadiusSquared;
	int                              m_currentTimeStep;

	void *debug1;

	// weighting functions =========================

	//6th degree polynomial
	void W_poly6_3d_precompute( Real supportRadius );
	Real W_poly6_3d( Real distance );

	void W_poly6_2d_precompute( Real supportRadius );
	Real W_poly6_2d( Real distance );
	Vector gradW_poly6_2d( Real distance, Vector dir );


	//spiky (gradient doesnt vanish near center)
	void W_spiky_3d_precompute( Real supportRadius );
	Real W_spiky_3d( Real distance );
	Vector gradW_spiky_3d( Real distance, Vector dir );

	void W_spiky_2d_precompute( Real supportRadius );
	Real W_spiky_2d( Real distance );
	Vector gradW_spiky_2d( Real distance, Vector dir );

	// viscosity kernel (see mueller03)
	void W_viscosity_precompute( Real supportRadius );
	Real W_viscosity( Real distance );
	Vector gradW_viscosity( Real distance, Vector dir );

	// integrators =========================
	void integrate_verlet( const Vector &p, const Vector &v, const Vector &pOld, const Vector &a, const Vector &i, Real dt, Real damping, Vector &pOut, Vector &vOut );
	void integrate_leapfrog( const Vector &p, const Vector &v, const Vector &pOld, const Vector &a, const Vector &i, Real dt, Real damping, Vector &pOut, Vector &vOut );
	void integrate_explicit_euler( const Vector &p, const Vector &v, const Vector &pOld, const Vector &a, const Vector &i, Real dt, Real damping, Vector &pOut, Vector &vOut );
};








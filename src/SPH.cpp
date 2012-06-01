
#include "SPH.h"




// constructor
SPH::Particle::Particle() : trajectory(0)
{
}

// SPH =====================================================

SPHPtr SPH::create()
{
	return SPHPtr( new SPH() );
}


// assumes that previous position is tracked outside
void integrate_verlet( const math::Vec3f &p, const math::Vec3f &v, const math::Vec3f &pOld, const math::Vec3f &a, const math::Vec3f &i, float dt, float damping, math::Vec3f &pOut, math::Vec3f &vOut )
{
	// Position = Position + (1.0f - Damping) * (Position - PositionOld) + dt * dt * a;
	math::Vec3f nextOldPos = p;
	//pOut = p + (1.0f - damping) * (p - pOld) + dt*dt*a;

	pOut = (2.0f-damping)*p - (1.0f-damping)*pOld + dt*dt*a;

	// Velocity = (Position - PositionOld) / dt;
	vOut = (pOut - nextOldPos) / dt;
}

void integrate_leapfrog( const math::Vec3f &p, const math::Vec3f &v, const math::Vec3f &pOld, const math::Vec3f &a, const math::Vec3f &i, float dt, float damping, math::Vec3f &pOut, math::Vec3f &vOut )
{
	vOut = v + a*dt*(1.0f-damping);
	pOut = p + vOut*dt*(1.0f-damping);
}


void integrate_explicit_euler( const math::Vec3f &p, const math::Vec3f &v, const math::Vec3f &pOld, const math::Vec3f &a, const math::Vec3f &i, float dt, float damping, math::Vec3f &pOut, math::Vec3f &vOut )
{
	pOut = p + (v+i)*dt*(1.0f-damping);
	vOut = v + a*dt*(1.0f-damping);

	/*
	// semi-implicit euler
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;
		// compute acceleration
		// TODO:  check if we need to divide by mass or massDensity
		math::Vec3f acceleration = p.forces*(1.0f/p.mass);
		p.velocity = p.velocity + acceleration*m_timeStep;
		p.position = p.position + p.velocity*m_timeStep;
	}
	*/

}


void SPH::timeIntegration()
{
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

		// handle fixed particles
		if( p.states.testFlag( Particle::STATE_BOUNDARY ) )
			continue;

		// compute acceleration
		// TODO:  check if we need to divide by mass or massDensity
		math::Vec3f acceleration = p.forces*(1.0f/p.mass);
		math::Vec3f impuls = p.impulses;

		math::Vec3f oldPos = p.position;


		integrate_verlet( p.position, p.velocity, p.positionPrev, acceleration, impuls, m_timeStep, m_damping, p.position, p.velocity );
		p.positionPrev = oldPos;

		p.position += impuls*m_timeStep;
		p.positionPrev = p.positionPrev + impuls*m_timeStep;
		/*

		math::Vec3f v0 = p.velocity;

		// adding impulse for verlet means just adding it to position. we need to adjust positionPrev to keep velocty intact
		if( impuls.getLength() > 0.0f )
		{
			math::Vec3f d = p.position - p.positionPrev;
			//p.position = p.temp1;
			//p.positionPrev = p.position - d;
			p.position = p.temp1;
			p.positionPrev = p.position - d;

		}else
		{
		}
		*/


		math::Vec3f v1 = (p.position - p.positionPrev) / m_timeStep;

		//integrate_leapfrog( p.position, p.velocity, p.positionPrev, acceleration, impuls, m_timeStep, m_damping, p.position, p.velocity );
		//integrate_explicit_euler( p.position, p.velocity, p.positionPrev, acceleration, impuls, m_timeStep, m_damping, p.position, p.velocity );



		p.positionPrev = oldPos;
	}
}



void SPH::advance()
{
	// update particle properties =========================================
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

		// debug
		p.color = math::Vec3f( 0.54f, 0.85f, 1.0f );
		if( p.trajectory )
			p.trajectory->m_steps.push_back( p );

		// update neighbour information - we look them up once and reuse them throughout the timestep
		p.neighbours.clear();
		// bruteforce for now - use efficient lookup structure later
		for( ParticleContainer::iterator it2 = m_particles.begin(); it2 != m_particles.end();++it2 )
		{
			if( it == it2 )continue;

			Particle &p2 = *it2;

			// this will make sure boundary particles dont affect fluid particles (pressure etc.)
			if( p2.states.testFlag( Particle::STATE_BOUNDARY ) )
				continue;

			float distanceSquared = (p.position - p2.position).getSquaredLength();
			if( distanceSquared < m_supportRadiusSquared )
			{
				Particle::Neighbour n;
				n.distance = sqrt(distanceSquared);
				n.p = &p2;
				p.neighbours.push_back( n );
			}
		}

		// compute mass-densities of particles
		p.massDensity = p.mass*W_poly6(0.0f);
		for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
			p.massDensity += it2->p->mass*W_poly6(it2->distance);

		// TODO: add weight function
		//p.massDensity += wallWeightFunction( it2->distance );

		// compute pressure at particle
		p.pressure = m_idealGasConstant*(p.massDensity-m_restDensity);
	}


	// compute forces =====================================================
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

		p.forces = math::Vec3f(0.0f, 0.0f, 0.0f);
		p.impulses = math::Vec3f(0.0f, 0.0f, 0.0f);

		// TODO: viscosity

		//gravity
		math::Vec3f f_gravity( 0.0f, 0.0f, 0.0f );
		f_gravity = -math::Vec3f( 0.0f, 1.0f, 0.0f ).normalized()*0.98f;
		p.forces += f_gravity;

		// TODO: external forces (user interaction, dynamic objects etc.)

		/*
		// standard SPH pressure force 
		math::Vec3f f_pressure( 0.0f, 0.0f, 0.0f );
		for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
		{
			float distance = it2->distance;
			Particle &n = *(it2->p);
			math::Vec3f gradW = gradW_spiky( distance, p.position - n.position );
			f_pressure +=  -(p.pressure + n.pressure)*0.5f*(n.mass/n.massDensity)*gradW;
		}
		p.forces += f_pressure;
		*/
	}

	// PCISPH pressure force --- 
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;
		p.pciPressureForce = math::Vec3f( 0.0f, 0.0f, 0.0f );
		p.pciFrictionForce = math::Vec3f( 0.0f, 0.0f, 0.0f );
		p.pciBoundaryForce = math::Vec3f( 0.0f, 0.0f, 0.0f );

		p.pciBoundaryImpulse = math::Vec3f( 0.0f, 0.0f, 0.0f );

		p.predictedPositionPrev = p.positionPrev;
	}
	int minIterations = 3;
	int iteration = 0;
	float densityFluctuationThreshold = 0.03f*m_restDensity;
	float maxDensityFluctuation = FLT_MIN;
	float maxStressTensorComponentDifference = FLT_MIN;
	//while( (maxDensityFluctuation<densityFluctuationThreshold)&&(iteration++ < minIterations) )
	while( iteration++ < 1 )
	{
		maxDensityFluctuation = FLT_MIN;
		maxStressTensorComponentDifference = FLT_MIN;

		// compute predicted position/velocity ===================================================
		for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
		{
			Particle &p = *it;

			if( p.states.testFlag( Particle::STATE_BOUNDARY ) )
			{
				p.predictedPosition = p.position;
				p.predictedVelocity = math::Vec3f(0.0f, 0.0f, 0.0f);
				continue;
			}

			// predict velocity/position
			math::Vec3f f = p.forces;
			f += p.pciPressureForce;
			if( m_friction )
				f += p.pciFrictionForce;
			f += p.pciBoundaryForce;
			math::Vec3f acceleration = f*(1.0f/p.mass);

			math::Vec3f impuls = p.impulses;
			impuls += p.pciBoundaryImpulse;

			// verlet
			integrate_verlet( p.position, p.velocity, p.predictedPositionPrev, acceleration, impuls, m_timeStep, m_damping, p.predictedPosition, p.predictedVelocity );
			//integrate_explicit_euler( p.position, p.velocity, p.positionPrev, acceleration, impuls, m_timeStep, m_damping, p.predictedPosition, p.predictedVelocity );
			// for verlet, adding impulses is just about adding it to position (and prevPosition, to keep velocity intact)
			//p.predictedPosition += impuls*m_timeStep;
			//p.predictedPositionPrev = p.positionPrev + impuls*m_timeStep;

			//integrate_leapfrog( p.position, p.velocity, p.positionPrev, acceleration, impuls, m_timeStep, m_damping, p.predictedPosition, p.predictedVelocity );
			//integrate_explicit_euler( p.position, p.velocity, p.positionPrev, acceleration, impuls, m_timeStep, m_damping, p.predictedPosition, p.predictedVelocity );
		}


		// update neighbours ===================================================
		for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
		{
			Particle &p = *it;

			// update neighbour information
			for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
			{
				Particle::Neighbour &n = *it2;
				n.predictedDistance = (p.predictedPosition - n.p->predictedPosition).getLength();
			}
		}


		// compute predicted massDensity and pressure ===================================================
		for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
		{
			Particle &p = *it;
			// predict density
			p.predictedMassDensity = p.mass*W_poly6(0.0f);
			for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
				p.predictedMassDensity += it2->p->mass*W_poly6(it2->predictedDistance);

			if( (p.predictedMassDensity > m_cricitalDensity) || !m_unilateralIncompressibility )
			{
				// predict density variation
				float predictedDensityVariation = p.predictedMassDensity - m_restDensity;
				maxDensityFluctuation = std::max(predictedDensityVariation, maxDensityFluctuation);

				// update pressure
				p.pressure += m_pciDelta*predictedDensityVariation;
			}else
			{
				// TODO: low stiffness discrete particle forces

			}
		}



		// compute correction forces ===================================================
		for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
		{
			Particle &p = *it;

			// ignore boundary particles
			if( p.states.testFlag( Particle::STATE_BOUNDARY ) )
				continue;

			// corrective pressure force
			math::Vec3f f_pressure( 0.0f, 0.0f, 0.0f );

			// corrective friction force (only used when m_friction is true)
			math::Vec3f f_friction( 0.0f, 0.0f, 0.0f );

			// corrective boundary condition force
			math::Vec3f f_boundary( 0.0f, 0.0f, 0.0f );


			for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
			{
				Particle &n = *(it2->p);
				float distance = it2->distance;

				math::Vec3f gradW = gradW_spiky( distance, p.predictedPosition - n.predictedPosition );

				// pressure force ---
				f_pressure += n.mass * ( p.pressure/(p.predictedMassDensity*p.predictedMassDensity) + n.pressure/(n.predictedMassDensity*n.predictedMassDensity) ) * gradW;

				// friction force ---
				//if( m_friction )
				//{
				//	f_friction += n.mass * math::transform( gradW, p.stressTensor/(p.predictedMassDensity*p.predictedMassDensity) + n.stressTensor/(n.predictedMassDensity*n.predictedMassDensity) );
				//}

			}


			// boundary condition force --- (using direct forcing)
			if(1)
			{
				float fluidRadius = 0.3f*m_supportRadius;
				float boundaryRadius = 0.3f*m_supportRadius;
				float d = FLT_MAX;
				Particle *c = 0;
				// test against all other particles for collision with boundary particles
				// TODO: optimize
				for( ParticleContainer::iterator bit = m_particles.begin(); bit != m_particles.end();++bit )
				{
					Particle &n = *bit;

					// dont test against yourself
					if( p.id == n.id )
						continue;

					// if other particle is boundary
					if( n.states.testFlag( Particle::STATE_BOUNDARY ) )
					{
						// test collision
						float dist = (n.predictedPosition - p.predictedPosition).getLength();
						if( (dist < boundaryRadius + fluidRadius)&&(dist < d) )
						{
							c = &n;
							d = dist;
							p.color = math::Vec3f(1.0f, 0.0f, 0.0f);
						}
					}
				}
				// any collision?
				if( c )
				{
					f_boundary = -p.predictedVelocity*(p.mass/(m_timeStep));

					float o = boundaryRadius + fluidRadius -d;
					math::Vec3f cp = p.predictedPosition - o*p.predictedVelocity.normalized();
					p.temp1 = cp;

					//p.pciBoundaryImpulse = -p.predictedVelocity.normalized()*((boundaryRadius + fluidRadius)-d)*(1.0f/m_timeStep)*1.0f;
					p.pciBoundaryImpulse = -o*p.predictedVelocity.normalized()*(1.0f/m_timeStep);
				}
			}



			p.pciPressureForce = f_pressure;
			//p.pciFrictionForce = f_friction;
			p.pciBoundaryForce = f_boundary;
		}
	};
	// for each particle
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;
		// add forces which we got fom pci-scheme
		p.forces += p.pciPressureForce;
		p.forces += p.pciBoundaryForce;
		if( m_friction )
			p.forces += p.pciFrictionForce;

		p.impulses += p.pciBoundaryImpulse;
	}


	// xdebug
	float maxpressure = 0.0f;
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

		//maxDensityFluctuation = std::max( maxDensityFluctuation, p.massDensity - m_restDensity );
		maxpressure = std::max( maxpressure, p.pressure );
	}
	//std::cout << "maxDensityFluctuation " << maxDensityFluctuation << std::endl;
	//std::cout << "maxpressure " << maxpressure << std::endl;



	// 
	timeIntegration();
}


void SPH::updateSupportRadius( float newSupportRadius )
{
	m_supportRadius = newSupportRadius;
	m_supportRadiusSquared = m_supportRadius*m_supportRadius;

	// update weighting kernels
	W_poly6_precompute(m_supportRadius);
	W_spiky_precompute(m_supportRadius);
}

void SPH::initialize()
{
	updateSupportRadius( .190625f );

	m_idealGasConstant = 0.1f;
	//m_restDensity = 998.29;
	m_restDensity = 1000.0f;

	m_numParticles = 0;


	// granular
	m_cricitalDensity = 1000.0f;
	m_frictionCoefficient = 1.0f;


	// solver parameters ---
	m_particleMass = 3.8125f; // 0.02f for water
	m_timeStep = 0.01f;
	m_damping = 0.01f;

	// switches for different solvertypes
	m_unilateralIncompressibility = true;
	m_friction = false;


	// compute pci delta and stress delta
	{

		// TODO: find out how many particles we are supposed to put into the neighbourhood and where to put them

		// prototype particle
		math::Vec3f p0 = math::Vec3f(0.0f, 0.0f, 0.0f);

		// completely random: number of particles per unit length in each dimension
		int res = 5;
		float s = m_supportRadius;


		// PCI - pressure delta
		float beta = (m_particleMass*m_timeStep)/m_restDensity;
		beta = beta*beta;
		math::Vec3f sumGrad = 0.0f;
		float sumGradDots = 0.0f;

		// granular - corrective stress coefficient
		math::Matrix33f outerproduct_gradW = math::Matrix33f::Zero();

		for( int k=0;k<res;++k )
			for( int j=0;j<res;++j )
				for( int i=0;i<res;++i )
				{
					float u = (float)i/(float)res;
					float v = (float)j/(float)res;
					float w = (float)k/(float)res;
					math::Vec3f pn = math::Vec3f(u*s-0.5f*s, v*s-0.5f*s, w*s-0.5f*s);
					float distance = (p0 - pn).getLength();		

					math::Vec3f gradW = gradW_spiky( distance, p0 - pn );

					// pci - pressure delta
					sumGrad += gradW;
					sumGradDots += math::dot( gradW, gradW );

					// granular - corrective stress coefficient
					outerproduct_gradW += (1.0f/m_restDensity)*math::outerProduct( gradW, gradW );
				}

		m_pciDelta = (-1.0f)/(beta*(math::dot(sumGrad, sumGrad) - sumGradDots ));

		// if 2d#
		outerproduct_gradW._13 = 0.0f;
		outerproduct_gradW._23 = 0.0f;
		outerproduct_gradW._33 = 0.0f;
		outerproduct_gradW._32 = 0.0f;
		outerproduct_gradW._31 = 0.0f;

		m_pciStressDeltaInverse = (2.0f*m_particleMass*m_particleMass*m_timeStep)/(m_restDensity*m_restDensity)*outerproduct_gradW;
		m_pciStressDeltaInverse.invert();
	}



	// initial fluid
	float spacing = 0.15f;
	if(1)
	{
		int n = 10;
		for( int i=0;i<n;++i )
			for( int j=0;j<n;++j, ++m_numParticles )
			{
				Particle p;

				initializeParticle(p, math::Vec3f( -1.5f + i * spacing, 1.0f + j * spacing, 0.0f ));

				m_particles.push_back(p);
			}
	}

	// debug
	m_particles[90].trajectory = new Trajectory();

	// some wall 
	if(1)
	{
		math::Matrix44f xform = math::Matrix44f::RotationMatrixZ( math::degToRad(-45.0f) );
		//math::Matrix44f xform = math::Matrix44f::Identity();
		spacing *= 0.5f;
		int n = 50;
		for( int i=0;i<n;++i )
			for( int j=0;j<2;++j, ++m_numParticles )
			{
				Particle p;

				initializeParticle(p, math::transform( math::Vec3f( -2.5f + i * spacing, 0.5f - j * spacing, 0.0f ), xform ) );
				p.states = Particle::STATE_BOUNDARY;

				m_particles.push_back(p);
			}
	}





}


void SPH::initializeParticle( Particle &p, const math::Vec3f &position )
{
	p.id = m_numParticles;
	p.position = position;
	p.positionPrev = p.position;
	p.color = math::Vec3f( 0.54f, 0.85f, 1.0f );
	p.mass = m_particleMass;
	p.stressTensor = math::Matrix33f::Zero();
	p.states = Particle::STATE_NONE;
}

void SPH::addCollider( ScalarFieldPtr sdf )
{
	m_collider = sdf;
}


// just for debugging
void SPH::updateTrajectories()
{
	// update particle properties =========================================
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

		if( p.trajectory )
			p.trajectory->m_steps.push_back( p );
	}
}



// poly6 =====================================================

float W_poly6_h2;
float W_poly6_coeff;

void W_poly6_precompute( float supportRadius )
{
	W_poly6_h2 = supportRadius*supportRadius;
	W_poly6_coeff = 315.0f/( 64.0f*MATH_PIf*powf(supportRadius, 9.0f) );
}
float W_poly6( float distance )
{
	float t = distance*distance;
	if( t > W_poly6_h2 )
		return 0.0f;
	if( t < 1.192092896e-07f )
		t = 1.192092896e-07f;
	t = W_poly6_h2-t;
	return W_poly6_coeff*t*t*t;
}


// spiky =====================================================
float W_spiky_h;
float W_spiky_coeff;
float W_spiky_grad_coeff;


void W_spiky_precompute( float supportRadius )
{
	W_spiky_h = supportRadius;
	W_spiky_coeff = 15.0f/( MATH_PIf*powf(supportRadius, 6.0f) );
	W_spiky_grad_coeff = -45.0f / (MATH_PIf*powf(supportRadius, 6.0f));
}
float W_spiky( float distance )
{
	if( distance > W_spiky_h )
		return 0.0f;
	float t = W_spiky_h - distance;
	return W_spiky_coeff*t*t*t;
}
math::Vec3f gradW_spiky( float distance, math::Vec3f dir )
{
	float t = (W_spiky_h - distance );
	return W_spiky_grad_coeff*dir*(1.0f/distance)*t*t;
}
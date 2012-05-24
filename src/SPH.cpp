
#include "SPH.h"

// SPH =====================================================

SPHPtr SPH::create()
{
	return SPHPtr( new SPH() );
}





void SPH::timeIntegration()
{
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
	// verlet
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

		// compute acceleration
		// TODO:  check if we need to divide by mass or massDensity
		math::Vec3f acceleration = p.forces*(1.0f/p.mass);

		// Position = Position + (1.0f - Damping) * (Position - PositionOld) + dt * dt * a;
		math::Vec3f oldPos = p.position;
		p.position = p.position + (1.0f - m_damping) * (p.position - p.positionPrev) + m_timeStep*m_timeStep*acceleration;
		p.positionPrev = oldPos;

		// Velocity = (Position - PositionOld) / dt;
		p.velocity = (p.position - p.positionPrev) / m_timeStep;


	}

	// TODO: leapfrog
}



void SPH::handleCollisions()
{
	if( !m_collider )
		return;

	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

		math::Vec3f _p = p.position;

		float sd = m_collider->value( _p );

		float proximityThreshold = 0.001f;

		// if particle is within proximity
		if( sd-proximityThreshold < 0.0f )
		{
			// move to surface
			math::Vec3f grad = m_collider->gradient(p.position);
			_p -= grad*(sd-proximityThreshold);

			p.position = _p;

			// adjust velocity
			p.velocity = p.velocity - 2.0f*( math::dotProduct(p.velocity, grad) )*grad;
		}
	}
}

void SPH::advance()
{
	// update particle properties =========================================
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

		// update neighbour information - we look them up once and reuse them throughout the timestep
		p.neighbours.clear();
		// bruteforce for now - use efficient lookup structure later
		for( ParticleContainer::iterator it2 = m_particles.begin(); it2 != m_particles.end();++it2 )
		{
			if( it == it2 )continue;
			Particle &p2 = *it2;
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

		// compute pressure at particle
		p.pressure = m_idealGasConstant*(p.massDensity-m_restDensity);
	}


	// compute forces =====================================================
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

		p.forces = math::Vec3f(0.0f, 0.0f, 0.0f);

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
	}
	int minIterations = 3;
	int iteration = 0;
	float densityFluctuationThreshold = 0.03f*m_restDensity;
	float maxDensityFluctuation = FLT_MIN;
	while( (maxDensityFluctuation<densityFluctuationThreshold)&&(iteration++ < minIterations) ) // TODO: add threshold
	{
		maxDensityFluctuation = FLT_MIN;

		// for each particle
		for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
		{
			Particle &p = *it;

			// predict velocity/position
			math::Vec3f acceleration = (p.forces+p.pciPressureForce)*(1.0f/p.mass);

			// verlet
			p.predictedPosition = p.position + (1.0f - m_damping) * (p.position - p.positionPrev) + m_timeStep*m_timeStep*acceleration;

			// handle collisions
			if( m_collider )
			{
				float sd = m_collider->value( p.predictedPosition );

				float proximityThreshold = 0.001f;

				// if particle is within proximity
				if( sd-proximityThreshold < 0.0f )
				{
					// move to surface
					math::Vec3f grad = m_collider->gradient(p.position);
					p.predictedPosition -= grad*(sd-proximityThreshold);
				}
			}
		}


		// for each particle
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


		// for each particle
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

		// for each particle
		for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
		{
			Particle &p = *it;

			// compute PCI pressure force
			math::Vec3f f_pressure( 0.0f, 0.0f, 0.0f );
			for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
			{
				Particle &n = *(it2->p);
				float distance = it2->distance;

				math::Vec3f gradW = gradW_spiky( distance, p.predictedPosition - n.predictedPosition );
				f_pressure += n.mass * ( p.pressure/(p.predictedMassDensity*p.predictedMassDensity) + n.pressure/(n.predictedMassDensity*n.predictedMassDensity) ) * gradW;
			}
			p.pciPressureForce = f_pressure;
		}
	};
	// for each particle
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;
		// add pressure force which we got fom pci
		p.forces += p.pciPressureForce;
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
	std::cout << "maxpressure " << maxpressure << std::endl;



	// 
	timeIntegration();

	handleCollisions();
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


	// solver parameters ---
	m_particleMass = 3.8125f; // 0.02f for water
	m_timeStep = 0.01f;
	m_damping = 0.01f;

	// switches for different solvertypes
	m_unilateralIncompressibility = true;


	// compute pci delta
	{
		float beta = (m_particleMass*m_timeStep)/m_restDensity;
		beta = beta*beta;


		math::Vec3f sumGrad = 0.0f;
		float sumGradDots = 0.0f;

		// TODO: find out how many particles we are supposed to put into the neighbourhood and where to put them

		// prototype particle
		math::Vec3f p0 = math::Vec3f(0.0f, 0.0f, 0.0f);

		// completely random: number of particles per unit length in each dimension
		int res = 5;
		float s = m_supportRadius;

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
					sumGrad += gradW;
					sumGradDots += math::dot( gradW, gradW );
				}

		m_pciDelta = (-1.0f)/(beta*(math::dot(sumGrad, sumGrad) - sumGradDots ));
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
				p.id = m_numParticles;
				p.position = math::Vec3f( -1.5f + i * spacing, 1.0f + j * spacing, 0.0f );
				p.positionPrev = p.position;
				p.color = math::Vec3f( 0.54f, 0.85f, 1.0f );
				//p.mass = 0.02f; // water
				p.mass = m_particleMass; // ?
				m_particles.push_back(p);
			}
	}





}

void SPH::addCollider( ScalarFieldPtr sdf )
{
	m_collider = sdf;
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
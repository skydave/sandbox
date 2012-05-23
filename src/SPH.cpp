
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
		p.velocity = p.velocity + p.acceleration*m_timeStep;
		p.position = p.position + p.velocity*m_timeStep;
	}
	*/
	// verlet
	float Damping = 0.01f;
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

		// Position = Position + (1.0f - Damping) * (Position - PositionOld) + dt * dt * a;
		math::Vec3f oldPos = p.position;
		p.position = p.position + (1.0f - Damping) * (p.position - p.positionPrev) + m_timeStep*m_timeStep*p.acceleration;
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
				p.neighbours.push_back( std::make_pair(sqrt(distanceSquared), &p2) );
		}

		// compute mass-densities of particles
		p.massDensity = p.mass*W_poly6(0.0f);
		for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
			p.massDensity += it2->second->mass*W_poly6(it2->first);

		// compute pressure at particle
		p.pressure = m_idealGasConstant*(p.massDensity-m_restDensity);


		// particle colors for debug
		if( p.id == 0 )
		{
			p.color = math::Vec3f( 1.0f, 0.0f, 0.0f );
			for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
				it2->second->color = math::Vec3f( 0.0f, 1.0f, 0.0f );
		}
	}


	// compute forces =====================================================
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

		// pressure force
		math::Vec3f f_pressure( 0.0f, 0.0f, 0.0f );
		for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
		{
			float distance = it2->first;
			Particle &n = *(it2->second);
			math::Vec3f gradW = gradW_spiky( distance, p.position - n.position );
			f_pressure +=  -(p.pressure + n.pressure)*0.5f*(n.mass/n.massDensity)*gradW;
		}


		//gravity
		math::Vec3f f_gravity( 0.0f, 0.0f, 0.0f );
		f_gravity = -math::Vec3f( 0.0f, 1.0f, 0.0f ).normalized()*0.98f;


		// compute acceleration
		// TODO:  check if we need to divide by mass or massDensity
		p.acceleration = (f_pressure+f_gravity)*(1.0f/p.mass);

	}



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
	m_restDensity = 100.0f;

	m_numParticles = 0;

	m_timeStep = 0.01f;


	// initial fluid
	float spacing = 0.2f;
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
				p.mass = 3.8125f; // ?
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
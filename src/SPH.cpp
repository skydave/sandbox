
#include "SPH.h"

float test_diff = 0.2f;


// constructor
SPH::Particle::Particle() : trajectory(0), pciTrajectory(0)
{
}

// SPH =====================================================

SPHPtr SPH::create()
{
	return SPHPtr( new SPH() );
}

// constructor
SPH::SPH() : m_currentTimeStep(0)
{
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
		Vector acceleration = p.forces*(1.0f/p.mass);
		Vector impuls = p.impulses;

		Vector oldPos = p.position;
		Vector oldVel = p.velocity;


		integrate_verlet( p.position, p.velocity, p.positionPrev, acceleration, impuls, m_timeStep, m_damping, p.position, p.velocity );
		//integrate_leapfrog( p.position, p.velocity, p.positionPrev, acceleration, impuls, m_timeStep, m_damping, p.position, p.velocity );


		p.positionPrev = oldPos;
		p.velocityPrev = p.velocityPrev;
	}
}



void SPH::advance()
{
	// update particle properties =========================================
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

		// ignore bounary particles
		if( p.states.testFlag( Particle::STATE_BOUNDARY ) )
			continue;

		// debug
		p.color = Vector( 0.54f, 0.85f, 1.0f );
		if( p.trajectory )
			p.trajectory->m_steps.push_back( p );
		if( p.pciTrajectory )
			p.pciTrajectory->m_steps.clear();
		if( p.id == 81 )
			p.color = Vector( 1.0f, 0.0f, 0.0f );

		// update neighbour information - we look them up once and reuse them throughout the timestep
		p.neighbours.clear();
		// bruteforce for now - use efficient lookup structure later
		// note that we add the current particle to its neighbour list. hoping that kernels will take care of
		// canceling it out when it is not needed
		for( ParticleContainer::iterator it2 = m_particles.begin(); it2 != m_particles.end();++it2 )
		{
			Particle &p2 = *it2;

			// this will make sure boundary particles dont affect fluid particles (pressure etc.)
			if( p2.states.testFlag( Particle::STATE_BOUNDARY ) )
				continue;

			Real distanceSquared = (p.position - p2.position).getSquaredLength();
			if( distanceSquared < m_supportRadiusSquared )
			{
				Particle::Neighbour n;
				n.distance = sqrt(distanceSquared);
				n.p = &p2;
				p.neighbours.push_back( n );
			}
		}

		// compute mass-densities of particles
		p.massDensity = 0.0f;
		for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
			p.massDensity += it2->p->mass*W_poly6_3d(it2->distance);

		// TODO: add weight function
		//p.massDensity += wallWeightFunction( it2->distance );

		// compute pressure at particle
		p.pressure = m_idealGasConstant*(p.massDensity-m_restDensity);
	}


	// compute forces =====================================================
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

		// ignore bounary particles
		if( p.states.testFlag( Particle::STATE_BOUNDARY ) )
			continue;

		p.forces = Vector(0.0f, 0.0f, 0.0f);
		p.impulses = Vector(0.0f, 0.0f, 0.0f);

		// TODO: viscosity

		//gravity
		Vector f_gravity( 0.0f, 0.0f, 0.0f );
		f_gravity = -Vector( 0.0f, 1.0f, 0.0f ).normalized()*0.98f;
		if( m_gravity )
			p.forces += f_gravity;

		// deformation test force
		Vector f_deformtest = -Vector( 0.0f, p.position.x - (test_diff*0.5f), 0.0f )*1.0f;
		if( m_deformtest )
			p.forces += f_deformtest;

		// TODO: external forces (user interaction, dynamic objects etc.)

		if(m_classicPressure)
		{
			// standard SPH pressure force 
			Vector f_pressure( 0.0f, 0.0f, 0.0f );
			for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
			{
				Real distance = it2->distance;
				Particle &n = *(it2->p);
				Vector gradW = W_spiky_derivative_3d( distance )*(p.position - n.position).normalized();
				f_pressure += (p.pressure/(p.massDensity*p.massDensity) + n.pressure/(n.massDensity*n.massDensity))*gradW;
			}

			p.forces += -p.mass*p.mass*f_pressure;
		}
	}

	// PCISPH pressure force --- 
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;
		p.pciPressureForce = Vector( 0.0f, 0.0f, 0.0f );
		p.pciFrictionForce = Vector( 0.0f, 0.0f, 0.0f );
		p.pciBoundaryForce = Vector( 0.0f, 0.0f, 0.0f );

		p.pciBoundaryImpulse = Vector( 0.0f, 0.0f, 0.0f );

		p.predictedPositionPrev = p.positionPrev;

		if( m_pciPressure )
		{
			p.predictedMassDensity = 0.0f;
			p.rho_err = 0.0f;
			p.dd = 0.0f;
			p.pressure = 0.0f;
		}

		p.temp2 = Vector( 0.0f, 0.0f, 0.0f );
	}
	int minIterations =  3;
	//int maxIterations = 3;
	int iteration = 0;
	Real densityFluctuationThreshold = 0.3f*m_restDensity;
	//Real densityFluctuationThreshold = 0.5f*m_restDensity;
	Real stressDeltaNormThreshold = 0.1f;
	Real maxDensityFluctuation = -FLT_MAX;
	Particle *maxDensityFluctuationParticle =0;
	Real stressDeltaNorm = -FLT_MAX;
	//while( 0 )
	bool done = !m_pciPressure && !m_friction;
	while( !done  )
	{
		// for debugging
		for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
		{
			Particle &p = *it;

			if( p.pciTrajectory  )
				p.pciTrajectory->m_steps.push_back( p );
		}

		maxDensityFluctuation =  -FLT_MAX;
		stressDeltaNorm = -FLT_MAX;

		// compute predicted position/velocity ===================================================
		for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
		{
			Particle &p = *it;

			if( p.states.testFlag( Particle::STATE_BOUNDARY ) )
			{
				p.predictedPosition = p.position;
				p.predictedVelocity = Vector(0.0f, 0.0f, 0.0f);
				continue;
			}

			// predict velocity/position
			Vector f = p.forces;
			if( m_pciPressure )
				f += p.pciPressureForce;
			if( m_friction )
				f += p.pciFrictionForce;
			Vector acceleration = f*(1.0f/p.mass);
			Vector impuls = p.impulses;

			// verlet
			integrate_verlet( p.position, p.velocity, p.predictedPositionPrev, acceleration, impuls, m_timeStep, m_damping, p.predictedPosition, p.predictedVelocity );
			//integrate_leapfrog( p.position, p.velocity, p.predictedPositionPrev, acceleration, impuls, m_timeStep, m_damping, p.predictedPosition, p.predictedVelocity );
		}


		// update neighbours ===================================================
		for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
		{
			Particle &p = *it;

			// update neighbour information
			int c = 0;
			for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2, ++c )
			{
				Particle::Neighbour &n = *it2;
				n.predictedDistance = (p.predictedPosition - n.p->predictedPosition).getLength();
			}
		}


		// compute predicted massDensity and density variation ===================================================
		for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
		{
			Particle &p = *it;

			// ignore boundary particles
			if( p.states.testFlag( SPH::Particle::STATE_BOUNDARY ) )
				continue;

			// predict density
			p.predictedMassDensity = 0.0f;
			for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
			{
				float w = W_poly6_3d(it2->predictedDistance);
				p.predictedMassDensity += it2->p->mass*w;
			}

			// predict density variation
 			Real predictedDensityVariation = p.predictedMassDensity - m_restDensity;
			//predictedDensityVariation *= exp( -0.25*(float)(iteration) );
			//predictedDensityVariation *= exp( -0.25*(float)(iteration) );

			if(predictedDensityVariation > maxDensityFluctuation)
				maxDensityFluctuationParticle = &p;

			//predictedDensityVariation = std::max( 0.0f, predictedDensityVariation ); 

			maxDensityFluctuation = std::max(fabsf(predictedDensityVariation), maxDensityFluctuation);
			//maxDensityFluctuation = std::max(predictedDensityVariation, maxDensityFluctuation);

			p.rho_err = predictedDensityVariation;
			if( p.neighbours.size() > 1 )
				p.dd = p.neighbours[1].predictedDistance;
		}

		//if( maxDensityFluctuation>densityFluctuationThreshold )
		{
			// update pressures
			for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
			{
				Particle &p = *it;

				// ignore boundary particles
				if( p.states.testFlag( SPH::Particle::STATE_BOUNDARY ) )
					continue;

				p.pressure += m_pciDelta*p.rho_err*1.0f;
			}

			// compute correction forces ===================================================
			for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
			{
				Particle &p = *it;

				// ignore boundary particles
				if( p.states.testFlag( Particle::STATE_BOUNDARY ) )
					continue;

				// corrective pressure force
				Vector f_pressure( 0.0f, 0.0f, 0.0f );

				for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
				{
					Particle &n = *(it2->p);
					Real distance = it2->distance;

					//Vector gradW = W_spiky_derivative_3d( distance )*(p.position - n.position).normalized();
					//Vector gradW = W_poly6_derivative_3d( distance )*(p.position - n.position).normalized();
					Vector gradW = W_mon92_derivative_3d( distance )*(p.position - n.position).normalized();
					


					// pressure force ---
					if(m_pciPressure)
						f_pressure += ( p.pressure/(p.massDensity*p.massDensity) + n.pressure/(n.massDensity*n.massDensity) ) * gradW;
				}



				if(m_pciPressure)
					p.pciPressureForce = -p.mass*p.mass*f_pressure*1.0f;

				//if( p.id == 1 )
				//	p.pciPressureForce = 0.0f;
			}
		}




		// update criterion
		done = true;
		if( m_pciPressure )
		{
			int pid = -1;
			int numN = 0;

			if( maxDensityFluctuationParticle )
			{
				pid = maxDensityFluctuationParticle->id;
				numN = maxDensityFluctuationParticle->neighbours.size();
			}

			std::cout<< "maxDensityFluctuation: " << maxDensityFluctuation  << " " << iteration << " "  << pid << " " << numN << std::endl;
			done = done && (maxDensityFluctuation<=densityFluctuationThreshold);
			/*
			if( maxDensityFluctuation > 60.0f )
			{
				std::cout << iteration << std::endl;
				done = true;
			}
			*/
		}
		done = done && (iteration > minIterations);
		++iteration;
		//done = done && (iteration > 100);
		done = done || (iteration > 1000);
	}; // pci while loop -> while(!criterion)
	// for each particle
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;


		if( p.states.testFlag( Particle::STATE_BOUNDARY ) )
		{
			p.predictedPosition = p.position;
			p.predictedVelocity = p.velocity;
			p.predictedPositionPrev = p.position;
			continue;
		}


		// add forces which we got fom pci-scheme
		if( m_pciPressure )
			p.forces += p.pciPressureForce*1.0f;
		if(	m_friction )
			p.forces += p.pciFrictionForce;




		if(m_boundary)
		{
			// predict velocity/position
			Vector f = p.forces;
			Vector acceleration = f*(1.0f/p.mass);

			Vector impuls = p.impulses;

			// verlet
			integrate_verlet( p.position, p.velocity, p.predictedPositionPrev, acceleration, impuls, m_timeStep, m_damping, p.predictedPosition, p.predictedVelocity );
			//integrate_verlet( p.position, p.velocity, p.positionPrev, acceleration, impuls, m_timeStep, m_damping, p.predictedPosition, p.predictedVelocity );



			Real fluidRadius = 0.3f*m_supportRadius;
			Real boundaryRadius = 0.3f*m_supportRadius;
			Real d = FLT_MAX;
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
					Real dist = (n.predictedPosition - p.predictedPosition).getLength();
					if( (dist < boundaryRadius + fluidRadius)&&(dist < d) )
					{
						c = &n;
						d = dist;
						p.color = Vector(1.0f, 0.0f, 0.0f);
					}
				}
			}
			// any collision?
			if( c )
			{
				Vector f_boundary = -p.predictedVelocity*(p.mass/(m_timeStep));

				Real o = boundaryRadius + fluidRadius -d;
				Vector cp = p.predictedPosition - o*p.predictedVelocity.normalized();
				p.temp1 = cp;

				//Vector p.pciBoundaryImpulse
				//p.pciBoundaryImpulse = -p.predictedVelocity.normalized()*((boundaryRadius + fluidRadius)-d)*(1.0f/m_timeStep)*1.0f;
				//p.pciBoundaryImpulse = -o*p.predictedVelocity.normalized()*(1.0f/m_timeStep);

				p.forces += f_boundary;
				//p.impulses += p.pciBoundaryImpulse;
			}
		}


	}


	// 
	timeIntegration();

	m_currentTimeStep++;










	// test =========================================
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

		// ignore bounary particles
		if( p.states.testFlag( Particle::STATE_BOUNDARY ) )
			continue;

		// update neighbour information - we look them up once and reuse them throughout the timestep
		p.neighbours.clear();
		// bruteforce for now - use efficient lookup structure later
		// note that we add the current particle to its neighbour list. hoping that kernels will take care of
		// canceling it out when it is not needed
		for( ParticleContainer::iterator it2 = m_particles.begin(); it2 != m_particles.end();++it2 )
		{
			Particle &p2 = *it2;

			// this will make sure boundary particles dont affect fluid particles (pressure etc.)
			if( p2.states.testFlag( Particle::STATE_BOUNDARY ) )
				continue;

			Real distanceSquared = (p.position - p2.position).getSquaredLength();
			if( distanceSquared < m_supportRadiusSquared )
			{
				Particle::Neighbour n;
				n.distance = sqrt(distanceSquared);
				n.p = &p2;
				p.neighbours.push_back( n );
			}
		}

		// compute mass-densities of particles
		p.massDensity = 0.0f;
		for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
			p.massDensity += it2->p->mass*W_poly6_3d(it2->distance);
	}
}


void SPH::updateSupportRadius( Real newSupportRadius )
{
	m_supportRadius = newSupportRadius;
	m_supportRadiusSquared = m_supportRadius*m_supportRadius;

	// update weighting kernels
	W_poly6_2d_precompute(m_supportRadius);
	W_spiky_2d_precompute(m_supportRadius);

	// for 3d case
	W_poly6_3d_precompute(m_supportRadius);
	W_spiky_3d_precompute(m_supportRadius);
	W_mon92_3d_precompute(m_supportRadius);
}

void SPH::initialize()
{
	updateSupportRadius( .190625f );
	//updateSupportRadius( .21f );
	//updateSupportRadius( .31f );
	//updateSupportRadius( 0.3f );

	m_idealGasConstant = 0.1f;

	// rest density is computed down below (prototype particle)
	//m_restDensity = 998.29;
	//m_restDensity = 1000.0f;
	//m_restDensity = 1000.0f;
	//m_restDensity = 1000.0f;
	//m_restDensity = 100.0f;

	m_particles.clear();

	// granular
	m_cricitalDensity = m_restDensity;
	m_frictionCoefficient = 0.8f;


	// solver parameters ---
	m_particleMass = 3.8125f; // 0.02f for water
	//m_particleMass = 24.6f;
	m_timeStep = 0.005f;
	m_damping = 0.01f;

	// switches for different solvertypes

	m_friction = false;
	m_classicPressure = false;
	m_pciPressure = !m_classicPressure;
	m_unilateralIncompressibility = false;

	m_boundary = false;
	m_gravity = false;
	m_deformtest = false;



	// compute pci delta and stress delta
	{

		// TODO: find out how many particles we are supposed to put into the neighbourhood and where to put them


		// completely random: number of particles per unit length in each dimension
		//int res = 251;
		int res = 50;
		Real s = m_supportRadius;


		// PCI - pressure delta
		// prototype neighbourhood
		std::vector<Vector> prototypeNeighbourhood;
		float prototypeMassDensity = 0.0f;
		/*
		do
		{
			int newRes = res - 1;
			// rebuild neighbourhood
			prototypeNeighbourhood.clear();
			for( int k=0;k<newRes;++k )
				for( int j=0;j<newRes;++j )
					for( int i=0;i<newRes;++i )
					{
						Real u = (Real)i/(Real)newRes;
						Real v = (Real)j/(Real)newRes;
						Real w = (Real)k/(Real)newRes;
						Vector pn = Vector(2.0f*u*s-s, 2.0f*v*s-s, 2.0f*w*s-s);
						prototypeNeighbourhood.push_back(pn);
					}

			prototypeMassDensity = m_particleMass*W_poly6_2d( 0.0f );
			// evaluate massDensity of prototype particle
			for( std::vector<Vector>::iterator it = prototypeNeighbourhood.begin(); it != prototypeNeighbourhood.end();++it )
				prototypeMassDensity += m_particleMass*W_poly6_2d( (p0 - *it).getLength() );
			
			// adjust res
			if( (prototypeMassDensity < m_restDensity)||(res<6) )
				// newres goes beneath restDensity
				break;
			else
				--res;
		}while( true );
		// rebuild neighbourhood using res which was found
		prototypeNeighbourhood.clear();
		for( int k=0;k<res;++k )
			for( int j=0;j<res;++j )
				for( int i=0;i<res;++i )
				{
					Real u = (Real)i/(Real)res;
					Real v = (Real)j/(Real)res;
					Real w = (Real)k/(Real)res;
					Vector pn = Vector(2.0f*u*s-s, 2.0f*v*s-s, 2.0f*w*s-s);
					prototypeNeighbourhood.push_back(pn);
				}

		// now find scaling of neighbourhood particles such that massDensity of our prototype particle (which is centered at origin)
		// is close to restDensity
		float scaling = 1.0f;
		do
		{
			float newScaling = scaling += 0.001f;
			prototypeMassDensity = m_particleMass*W_poly6_2d( 0.0f );
			// evaluate massDensity of prototype particle
			for( std::vector<Vector>::iterator it = prototypeNeighbourhood.begin(); it != prototypeNeighbourhood.end();++it )
				prototypeMassDensity += m_particleMass*W_poly6_2d( (p0 - *it*scaling).getLength() );
			
			// adjust scaling
			if( prototypeMassDensity < m_restDensity )
				break;
			else
				scaling += 0.001f;
			std::cout<<prototypeMassDensity<<std::endl;
		}while( prototypeMassDensity > m_restDensity );
		// rebuild neighbourhood using scale which was found
		prototypeNeighbourhood.clear();
		for( int k=0;k<res;++k )
			for( int j=0;j<res;++j )
				for( int i=0;i<res;++i )
				{
					Real u = (Real)i/(Real)res;
					Real v = (Real)j/(Real)res;
					Real w = (Real)k/(Real)res;
					Vector pn = Vector(2.0f*u*s-s, 2.0f*v*s-s, 2.0f*w*s-s);
					prototypeNeighbourhood.push_back(pn*scaling);
				}



		*/

		// build prototype particle neighbourhood
		res = 8;
		//float scale = 3.2f;
		//float scale = 3.2f;
		//float scale = 1.2f;
		float scale = 2.963f;
		//float scale = 1.0f;
		prototypeNeighbourhood.clear();

		/*
		for( int j=0;j<res;++j )
			for( int i=0;i<res;++i )
			{
				Real u = (Real)i/(Real)res;
				Real v = (Real)j/(Real)res;
				Vector pn = Vector(2.0f*u*s-s, 2.0f*v*s-s, 0.0f)*scale;

				prototypeNeighbourhood.push_back(pn);
			}
		*/
		for( int k=0;k<res;++k )
			for( int j=0;j<res;++j )
				for( int i=0;i<res;++i )
				{
					Real u = (Real)i/(Real)res;
					Real v = (Real)j/(Real)res;
					Real w = (Real)k/(Real)res;
					Vector pn = Vector(2.0f*u*s-s, 2.0f*v*s-s, 2.0f*w*s-s)*scale;

					prototypeNeighbourhood.push_back(pn);
				}


		// select prototype particle (we the one which is closest to the center and therefore should have most neighbours)
		int pi = -1;
		float mind = FLT_MAX;
		int index = 0;
		for( std::vector<Vector>::iterator it = prototypeNeighbourhood.begin(); it != prototypeNeighbourhood.end();++it, ++index )
		{
			Vector &p = *it;
			float d = (p-Vector(0.0f, 0.0f, 0.0f)).getLength();
			if( d < mind )
			{
				pi = index;
				mind = d;
			}
		}

		// pi should not be -1 at this point
		// compute prototypeMassDensity
		prototypeMassDensity = 0.0f;
		Vector prototypeParticle = prototypeNeighbourhood[pi];
		for( std::vector<Vector>::iterator it = prototypeNeighbourhood.begin(); it != prototypeNeighbourhood.end();++it, ++index )
		{
			Vector &p = *it;
			float d = (p-prototypeParticle).getLength();
			prototypeMassDensity += m_particleMass*W_poly6_3d( d );
		}

		m_restDensity = prototypeMassDensity;



		// compute pci delta
		Vector s1v = Vector(0.0f, 0.0f, 0.0f);
		Real s2 = 0.0f;
		for( std::vector<Vector>::iterator it = prototypeNeighbourhood.begin(); it != prototypeNeighbourhood.end();++it )
		{
			Vector &pn = *it;
			float distance = (prototypeParticle - pn).getLength();
			Vector gradW = W_spiky_derivative_3d( distance )*(prototypeParticle - pn).normalized();
			//Vector gradW = W_poly6_derivative_3d( distance )*(prototypeParticle - pn).normalized();

			// pci - pressure delta
			s1v += gradW;
			s2 += math::dot(gradW,gradW);
		}
		Real beta = (m_timeStep*m_timeStep*m_particleMass*m_particleMass)*(2.0f/(m_restDensity*m_restDensity));
		m_pciDelta = (-1.0f)/(beta*(-math::dot(s1v, s1v ) - s2 ));
		//m_pciDelta = -0.01f;
		int debug=0;
		/*
		// compute stress delta
		for( int k=0;k<res;++k )
			for( int j=0;j<res;++j )
				for( int i=0;i<res;++i )
				{
					Real u = (Real)i/(Real)res;
					Real v = (Real)j/(Real)res;
					Real w = (Real)k/(Real)res;
					Vector pn = Vector(2.0f*u*s-s, 2.0f*v*s-s, 2.0f*w*s-s);
					Real distance = (p0 - pn).getLength();

					Vector gradWV = -W_spiky_derivative_2d( distance )*(p0 - pn).normalized();

					// pci - pressure delta
					s1v += gradWV;
					s2 += math::dot(gradWV,gradWV);
				}

		*/

		/*
		m_pciStressDeltaInverse = (2.0f*m_particleMass*m_particleMass*m_timeStep)/(m_restDensity*m_restDensity)*outerproduct_gradW;
		m_pciStressDeltaInverse.invert();
		std::cout << "m_pciStressDeltaInverse: " << m_pciStressDeltaInverse.m[0][0] << " " << m_pciStressDeltaInverse.m[0][1] << std::endl;
		std::cout << "                         " << m_pciStressDeltaInverse.m[1][0] << " " << m_pciStressDeltaInverse.m[1][1] << std::endl;
		*/

		/*
		res = 10;
		Tensor testDing = Tensor::Zero();
		for( int k=0;k<res;++k )
			for( int j=0;j<res;++j )
				for( int i=0;i<res;++i )
				{
					Real u = (Real)i/(Real)res;
					Real v = (Real)j/(Real)res;
					Real w = (Real)k/(Real)res;
					Vector pn = Vector(2.0f*u*s-s, 2.0f*v*s-s, 2.0f*w*s-s);
					Real distance = (p0 - pn).getLength();		

					//Vector gradW = gradW_spiky_2d( distance, p0 - pn );
					Vector gradW = -W_spiky_derivative_2d( distance  )*(p0 - pn).normalized();
					Vector gradW2 = gradW_poly6_2d( distance  )*(p0 - pn).normalized();

					// granular - corrective stress coefficient
					testDing += (1.0f/m_restDensity)*math::outerProduct( math::Vec2f(gradW2.x, gradW2.y), math::Vec2f(gradW2.x, gradW2.y) );
				}
		Tensor D_inverse = 2.0f*m_timeStep*m_particleMass*m_particleMass*(1.0f/(m_restDensity*m_restDensity))*testDing*1.0f;
		D_inverse.invert();
		//D_inverse = Tensor::Zero();
		m_pciStressDeltaInverse = D_inverse;
		*/

	}



	// initial fluid
	//Real spacing = 0.15f;
	Real spacing = 0.135f;
	//Real spacing = test_diff;
	Vector offset( 0.0f, 2.0f, 0.0f );
	if(1)
	{
		int n = 10;
		for( int i=0;i<n;++i )
			for( int j=0;j<n;++j )
			{
				Particle p;

				initializeParticle(p, Vector( i * spacing, j * spacing, 0.0f ) + offset);

				m_particles.push_back(p);
			}
	}

	// stress debug setup
	if(0)
	{
		Real d = test_diff;
		//d = 0.18f;
		d = 0.123f;

		//Real d =0.05f;
		Particle p;
		// center row
		initializeParticle(p, Vector( 0.0f, 0.0f, 0.0f )); // debug particle 1
		m_particles.push_back(p);
		initializeParticle(p, Vector( d, 0.0f, 0.0f ));// debug particle 2
		m_particles.push_back(p);


		//initializeParticle(p, Vector( -d, 0.0f, 0.0f )); // debug particle 1
		//m_particles.push_back(p);


		/*
		initializeParticle(p, Vector( 0.5f*d, 0.75f*d, 0.0f ));// debug particle 2
		m_particles.push_back(p);
		initializeParticle(p, Vector( 0.5f*d, -0.75f*d, 0.0f ));// debug particle 2
		m_particles.push_back(p);
		*/

		
		//initializeParticle(p, Vector( 0.0f, 0.5f*d, 0.0f )); // debug particle 1
		//m_particles.push_back(p);
		//initializeParticle(p, Vector( d, 0.5f*d, 0.0f ));// debug particle 2
		//m_particles.push_back(p);



		///*
		initializeParticle(p, Vector( -d, 0.0f, 0.0f ));m_particles.push_back(p);
		//initializeParticle(p, Vector( -d*2.0f, 0.0f, 0.0f ));m_particles.push_back(p);
		initializeParticle(p, Vector( d*2.0f, 0.0f, 0.0f ));m_particles.push_back(p);
		//initializeParticle(p, Vector( d*3.0f, 0.0f, 0.0f ));m_particles.push_back(p);

		// row +1
		initializeParticle(p, Vector( -d, d, 0.0f ));m_particles.push_back(p);
		initializeParticle(p, Vector( 0.0f, d, 0.0f ));m_particles.push_back(p);
		initializeParticle(p, Vector( d, d, 0.0f ));m_particles.push_back(p);
		initializeParticle(p, Vector( d*2.0f, d, 0.0f ));m_particles.push_back(p);

		// row -1
		initializeParticle(p, Vector( -d, -d, 0.0f ));m_particles.push_back(p);
		initializeParticle(p, Vector( 0.0f, -d, 0.0f ));m_particles.push_back(p);
		initializeParticle(p, Vector( d, -d, 0.0f ));m_particles.push_back(p);
		initializeParticle(p, Vector( d*2.0f, -d, 0.0f ));m_particles.push_back(p);

		// row 2
		//initializeParticle(p, Vector( 0.0f, d*2.0f, 0.0f ));m_particles.push_back(p);
		//initializeParticle(p, Vector( d, d*2.0f, 0.0f ));m_particles.push_back(p);

		// row -2
		//initializeParticle(p, Vector( 0.0f, -d*2.0f, 0.0f ));m_particles.push_back(p);
		//initializeParticle(p, Vector( d, -d*2.0f, 0.0f ));m_particles.push_back(p);
		//*/
	}

	// debug
	m_particles[0].trajectory = new Trajectory();

	// some wall 
	if(m_boundary)
	{
		//math::Matrix44f xform = math::Matrix44f::RotationMatrixZ( math::degToRad(-45.0f) );
		math::Matrix44f xform = math::Matrix44f::Identity();
		spacing *= 0.5f;
		int n = 80;
		for( int i=0;i<n;++i )
			for( int j=0;j<2;++j )
			{
				Particle p;

				initializeParticle(p, math::transform( Vector( -4.5f + i * spacing, 0.5f - j * spacing, 0.0f ), xform ) );
				p.states = Particle::STATE_BOUNDARY;

				m_particles.push_back(p);
			}
	}




	//debug1 = &(m_particles[81].pciPressureForce.x);
}


void SPH::initializeParticle( Particle &p, const Vector &position )
{
	p.id = numParticles();
	p.position = position;
	p.positionPrev = p.position;
	p.velocity = Vector(0.0f, 0.0f, 0.0f);
	p.velocityPrev = p.velocity;

	p.color = Vector( 0.54f, 0.85f, 1.0f );
	p.mass = m_particleMass;
	p.stressTensor = Tensor::Zero();
	p.states = Particle::STATE_NONE;

	int maxNumNeihbours = 30;
	//p.neighbourDebug.resize( maxNumNeihbours );

	p.rho_err = 0.0f;

	//p.pciTrajectory = new Trajectory();
	p.dd = 0.0f;
}

size_t SPH::numParticles()const
{
	return m_particles.size();
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

SPH::Real W_poly6_3d_h2;
SPH::Real W_poly6_3d_coeff;
SPH::Real W_poly6_3d_derivative_coeff;

void SPH::W_poly6_3d_precompute( Real supportRadius )
{
	W_poly6_3d_h2 = supportRadius*supportRadius;
	W_poly6_3d_coeff = 315.0f/( 64.0f*MATH_PIf*powf(supportRadius, 9.0f) );
	W_poly6_3d_derivative_coeff = -945.0/( 32.0*MATH_PIf*powf(supportRadius, 3.0f) );
}
SPH::Real SPH::W_poly6_3d( Real distance )
{
	Real t = distance*distance;
	if( t > W_poly6_3d_h2 )
		return 0.0f;
	if( t < 1.192092896e-07f )
		t = 1.192092896e-07f;
	t = W_poly6_3d_h2-t;
	return W_poly6_3d_coeff*t*t*t;
}

SPH::Real SPH::W_poly6_derivative_3d( Real distance )
{
	float d2 = distance*distance;
	if( d2>W_poly6_3d_h2 )
		return 0.0f;
	float t = (1.0 - (d2/W_poly6_3d_h2));
	return W_poly6_3d_derivative_coeff*(distance/W_poly6_3d_h2)*t*t;
}

SPH::Real W_poly6_2d_h;
SPH::Real W_poly6_2d_h2;
SPH::Real W_poly6_2d_coeff;
SPH::Real W_poly6_2d_grad_coeff;

void SPH::W_poly6_2d_precompute( Real supportRadius )
{
	W_poly6_2d_h = supportRadius;
	W_poly6_2d_h2 = supportRadius*supportRadius;

	W_poly6_2d_coeff = 4.0f/( MATH_PIf*supportRadius*supportRadius );
	W_poly6_2d_grad_coeff = -24.0f/(MATH_PIf*supportRadius*supportRadius);
}
SPH::Real SPH::W_poly6_2d( Real distance )
{
	if( distance >= W_poly6_2d_h )
		return 0.0f;

	float t =  1.0f - (distance*distance)/W_poly6_2d_h2;
	return W_poly6_2d_coeff*t*t*t;
}

float SPH::gradW_poly6_2d( Real distance )
{
	if( distance >= W_poly6_2d_h )
		return 0.0f;
	float t = 1.0f - (distance*distance)/W_poly6_2d_h2;
	return W_poly6_2d_grad_coeff * (distance/W_poly6_2d_h2)*t*t;
}


// spiky =====================================================

SPH::Real W_spiky_3d_h;
SPH::Real W_spiky_3d_h2;
SPH::Real W_spiky_3d_coeff;
SPH::Real W_spiky_3d_derivative_coeff;

void SPH::W_spiky_3d_precompute( Real supportRadius )
{
	W_spiky_3d_h = supportRadius;
	W_spiky_3d_h2 = supportRadius*supportRadius;

	W_spiky_3d_derivative_coeff = -(45.0f / (MATH_PIf*powf(supportRadius, 3.0f)))*(1.0f/supportRadius);
}

SPH::Real SPH::W_spiky_derivative_3d( Real distance )
{
	if (distance > W_spiky_3d_h)
		return 0.0f;
	float t = 1.0f - distance/W_spiky_3d_h;
	return W_spiky_3d_derivative_coeff*t*t;
}



SPH::Real W_spiky_2d_h;
SPH::Real W_spiky_2d_coeff;
SPH::Real W_spiky_2d_derivative_coeff;

void SPH::W_spiky_2d_precompute( Real supportRadius )
{
	W_spiky_2d_h = supportRadius;
	W_spiky_2d_coeff = 10.0f/( MATH_PIf*W_spiky_2d_h*W_spiky_2d_h );
	W_spiky_2d_derivative_coeff = -30.0f/( MATH_PIf*W_spiky_2d_h*W_spiky_2d_h )*(1.0f/W_spiky_2d_h);
}
SPH::Real SPH::W_spiky_2d( Real distance )
{
	if( distance >= W_spiky_2d_h )
		return 0.0f;

	float t = 1.0f-(distance/W_spiky_2d_h);
	return W_spiky_2d_coeff*t*t*t;

}
SPH::Real SPH::W_spiky_derivative_2d( Real distance )
{
	if( distance > W_spiky_2d_h )
		return 0.0f;

	float t = 1.0f-(distance/W_spiky_2d_h);
	return W_spiky_2d_derivative_coeff*t*t;
}

// mon92 =====================================================
SPH::Real W_mon92_3d_h;
SPH::Real W_mon92_3d_coeff;
SPH::Real W_mon92_3d_derivative_coeff1;
SPH::Real W_mon92_3d_derivative_coeff2;

void SPH::W_mon92_3d_precompute( SPH::Real supportRadius )
{
	W_mon92_3d_h = supportRadius;
	float h = W_mon92_3d_h;
	W_mon92_3d_coeff = 1.0f/(MATH_PIf*h*h*h);
	W_mon92_3d_derivative_coeff1 = 3.0f/(MATH_PIf*h*h*h*h*h);
	W_mon92_3d_derivative_coeff2 = -3.0f/(4.0f*MATH_PIf*h*h*h*h);
}

SPH::Real SPH::W_mon92_3d( SPH::Real distance )
{
	float h = W_mon92_3d_h;
	float d = 2.0f*distance;
	float q = d/h;
	if( d<h )
	{
		return W_mon92_3d_coeff*(1.0f - (3.0f/2.0f)*q*q + (3.0f/4.0f)*q*q*q);
	}else
	if( d<2.0f*h )
	{
		float t = (2.0-q);
		return W_mon92_3d_coeff*(1.0/4.0)*t*t*t;
	}
	return 0.0f;
}

SPH::Real SPH::W_mon92_derivative_3d( SPH::Real distance )
{
	float h = W_mon92_3d_h;
	float d = 2.0f*distance;

	if( d<h )
	{
		return W_mon92_3d_derivative_coeff1*( (3.0f*d*d)/(4.0f*h) - d );
	}else
	if( d<2.0f*h )
	{
		float t = 2.0f-(d/h);
		return W_mon92_3d_derivative_coeff2*t*t;
	}
	return 0.0f;
}


// viscosity =====================================================
/*
Real W_viscosity_h;
Real W_viscosity_h2;
Real W_viscosity_2h3;
Real W_viscosity_coeff;
Real W_viscosity_grad_coeff;


void W_viscosity_precompute( Real supportRadius )
{
	W_viscosity_h = supportRadius;
	W_viscosity_h2 = supportRadius*supportRadius;
	W_viscosity_2h3 = 2.0f*supportRadius*supportRadius*supportRadius;
	W_viscosity_coeff = 15.0f/(  MATH_PIf*W_viscosity_2h3  );
}

Real W_viscosity( Real distance )
{

	return 0.0f;
}

Vector gradW_viscosity( Real distance, Vector dir )
{
	return Vector();
}*/










// INTEGRATORS =======================


// assumes that previous position is tracked outside
void SPH::integrate_verlet( const Vector &p, const Vector &v, const Vector &pOld, const Vector &a, const Vector &i, Real dt, Real damping, Vector &pOut, Vector &vOut )
{
	// Position = Position + (1.0f - Damping) * (Position - PositionOld) + dt * dt * a;
	Vector nextOldPos = p;
	//pOut = p + (1.0f - damping) * (p - pOld) + dt*dt*a;

	pOut = (2.0f-damping)*p - (1.0f-damping)*pOld + dt*dt*a;

	// Velocity = (Position - PositionOld) / dt;
	vOut = (pOut - nextOldPos) / dt;
}

void SPH::integrate_leapfrog( const Vector &p, const Vector &v, const Vector &pOld, const Vector &a, const Vector &i, Real dt, Real damping, Vector &pOut, Vector &vOut )
{
	//vOut = v + a*dt*(1.0f-damping);
	//pOut = p + vOut*dt*(1.0f-damping);
	Vector v_ = v;
	if( m_currentTimeStep == 0 )
		// move velocity half step backwards
		v_ = v-0.5f*m_timeStep*a;

	vOut = v_ + a*dt*(1.0f-damping);
	pOut = p + vOut*dt*(1.0f-damping);
}


void SPH::integrate_explicit_euler( const Vector &p, const Vector &v, const Vector &pOld, const Vector &a, const Vector &i, Real dt, Real damping, Vector &pOut, Vector &vOut )
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
		Vector acceleration = p.forces*(1.0f/p.mass);
		p.velocity = p.velocity + acceleration*m_timeStep;
		p.position = p.position + p.velocity*m_timeStep;
	}
	*/

}
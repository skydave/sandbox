
#include "SPH.h"

float test_diff = 0.01f;


// constructor
SPH::Particle::Particle() : trajectory(0)
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
		p.positionPrev = oldPos;

		p.position += impuls*m_timeStep;
		p.positionPrev = p.positionPrev + impuls*m_timeStep;

		p.velocityPrev = p.velocityPrev;
		/*

		Vector v0 = p.velocity;

		// adding impulse for verlet means just adding it to position. we need to adjust positionPrev to keep velocty intact
		if( impuls.getLength() > 0.0f )
		{
			Vector d = p.position - p.positionPrev;
			//p.position = p.temp1;
			//p.positionPrev = p.position - d;
			p.position = p.temp1;
			p.positionPrev = p.position - d;

		}else
		{
		}
		*/


		Vector v1 = (p.position - p.positionPrev) / m_timeStep;

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
		p.color = Vector( 0.54f, 0.85f, 1.0f );
		if( p.trajectory )
			p.trajectory->m_steps.push_back( p );
		if( p.id == 81 )
			p.color = Vector( 1.0f, 0.0f, 0.0f );

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
		p.massDensity = p.mass*W_poly6_2d(0.0f);
		for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
			p.massDensity += it2->p->mass*W_poly6_2d(it2->distance);

		// TODO: add weight function
		//p.massDensity += wallWeightFunction( it2->distance );

		// compute pressure at particle
		p.pressure = m_idealGasConstant*(p.massDensity-m_restDensity);
	}


	// compute forces =====================================================
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

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
		/*
		if( p.id == 0 )
			p.forces += -Vector( 0.0f, -0.075, 0.0f )*1.0f;
		if( p.id == 1 )
			p.forces += -Vector( 0.0f, 0.075, 0.0f )*1.0f;
		*/
		// TODO: external forces (user interaction, dynamic objects etc.)

		/*
		// standard SPH pressure force 
		Vector f_pressure( 0.0f, 0.0f, 0.0f );
		for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
		{
			Real distance = it2->distance;
			Particle &n = *(it2->p);
			Vector gradW = gradW_spiky( distance, p.position - n.position );
			f_pressure +=  -(p.pressure + n.pressure)*0.5f*(n.mass/n.massDensity)*gradW;
		}
		p.forces += f_pressure;
		*/
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

		p.temp2 = Vector( 0.0f, 0.0f, 0.0f );
	}
	int minIterations = 3;
	int iteration = 0;
	Real densityFluctuationThreshold = 0.03f*m_restDensity;
	Real maxDensityFluctuation = FLT_MIN;
	Real maxStressTensorComponentDifference = FLT_MIN;
	//while( (maxDensityFluctuation<densityFluctuationThreshold)&&(iteration++ < minIterations) )
	//while( iteration++ < 50 )
	while( iteration++ < 500 )
	{
		maxDensityFluctuation = FLT_MIN;
		maxStressTensorComponentDifference = FLT_MIN;

		//std::cout << "dsg " << m_particles[81].pciPressureForce.x << " " << m_particles[81].pciPressureForce.y << " " << m_particles[81].pciPressureForce.z << std::endl;

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
			//std::cout << "f_pressure " << p.pciPressureForce.x << " " << p.pciPressureForce.y << " " << p.pciPressureForce.z << std::endl;
			f += p.pciPressureForce;
			if( m_friction )
				f += p.pciFrictionForce;
			Vector acceleration = f*(1.0f/p.mass);
			p.temp2 = f;

			Vector impuls = p.impulses;

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
			int c = 0;
			for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2, ++c )
			{
				Particle::Neighbour &n = *it2;
				n.predictedDistance = (p.predictedPosition - n.p->predictedPosition).getLength();
				if(c < p.neighbourDebug.size() )
					n.nd = &p.neighbourDebug[c];
			}
		}


		// compute predicted massDensity and pressure ===================================================
		for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
		{
			Particle &p = *it;
			// predict density
			p.predictedMassDensity = p.mass*W_poly6_2d(0.0f);
			for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
				p.predictedMassDensity += it2->p->mass*W_poly6_2d(it2->predictedDistance);

			if( (p.predictedMassDensity > m_cricitalDensity) || !m_unilateralIncompressibility )
			{
				// predict density variation
				Real predictedDensityVariation = p.predictedMassDensity - m_restDensity;
				maxDensityFluctuation = std::max(predictedDensityVariation, maxDensityFluctuation);

				// update pressure
				p.pressure += m_pciDelta*predictedDensityVariation;
			}else
			{
				// TODO: low stiffness discrete particle forces

			}
		}



		//
		if( m_friction )
		{
			for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
			{
				Particle &p = *it;




				Tensor outer_product_predicted = Tensor::Zero();
				Tensor outer_product_predicted_clean = Tensor::Zero();
				Tensor outer_product_0 = Tensor::Zero();
				//  predict strain rate
				int c = 0;
				for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2, ++c )
				{
					Particle &n = *(it2->p);
					Real distance = it2->distance;

					Vector gradW = gradW_spiky_2d( distance, p.predictedPosition - n.predictedPosition );
					//Vector gradW = gradW_poly6_2d( distance, p.predictedPosition - n.predictedPosition );

					//strain_rate_dissipation += (n.mass/n.massDensity)*math::outerProduct( gradW, (n.predictedVelocity - p.velocityPrev) );
					//strain_rate_dissipation += (n.mass/n.massDensity)*math::outerProduct( math::Vec2f(gradW.x, gradW.y), math::Vec2f((n.predictedVelocity - p.velocityPrev).x, (n.predictedVelocity - p.velocityPrev).y) );
					//strain_rate_dissipation += (n.mass/n.massDensity)*math::outerProduct( math::Vec2f(gradW.x, gradW.y), math::Vec2f(n.predictedVelocity.x, n.predictedVelocity.y) );
					outer_product_predicted += (n.mass/n.massDensity)*math::outerProduct( math::Vec2f(gradW.x, gradW.y), math::Vec2f(n.predictedVelocity.x, n.predictedVelocity.y) );
					//outer_product_predicted_clean += math::outerProduct( math::Vec2f(gradW.x, gradW.y), math::Vec2f(n.predictedVelocity.x, n.predictedVelocity.y) );
					outer_product_predicted_clean += math::outerProduct( math::Vec2f(gradW.x, gradW.y), math::Vec2f(n.predictedVelocity.x, n.predictedVelocity.y) );
					//outer_product_0 += (n.mass/n.massDensity)*math::outerProduct( math::Vec2f(gradW.x, gradW.y), math::Vec2f(n.velocity.x, n.velocity.y) );
					outer_product_0 += (n.mass/n.massDensity)*math::outerProduct( math::Vec2f(gradW.x, gradW.y), math::Vec2f(n.predictedVelocity.x, n.predictedVelocity.y) );

					if( c < p.neighbourDebug.size() )
						p.neighbourDebug[c].gradW = gradW;
				}

				//Tensor strainRate_0 = 0.5f*(outer_product_0+outer_product_0.transposed());
				Tensor strainRate_0 = outer_product_0;
				//strainRate_0._11 = 0.0f;
				//strainRate_0._12 = 0.0f;
				//strainRate_0._12 = 0.0f;
				//strainRate_0._22 = 0.0f;
				//p.strainRate = outer_product_0;
				p.strainRate = outer_product_0;
				Tensor strain_rate_dissipation = outer_product_predicted - outer_product_0;
				//p.strainRate = 0.5f*(outer_product+outer_product.transposed());
				//std::cout << "strain rate: " << strain_rate.m[0][0] << " " << strain_rate.m[0][1] << std::endl;
				//std::cout << "             " << strain_rate.m[1][0] << " " << strain_rate.m[1][1] << std::endl;


				/*
				// test
				Tensor testDing = Tensor::Zero();
				for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
				{
					Particle &n = *(it2->p);
					Real distance = it2->distance;

					Vector gradW = gradW_spiky( distance, p.predictedPosition - n.predictedPosition );
					testDing += (n.mass/n.massDensity)*math::outerProduct( math::Vec2f(gradW.x, gradW.y), math::Vec2f(gradW.x, gradW.y) );
				}
				Tensor D_inverse = -2.0f*m_timeStep*p.mass*p.mass*(1.0f/(p.massDensity*p.massDensity))*testDing;
				D_inverse.invert();

				//Tensor ds = D_inverse*(-1.0f*strainRate_0);
				*/
				Tensor ds = (-1.0f*strainRate_0)*m_pciStressDeltaInverse;
				//ds._11 = 0.0f;
				//Tensor s_friction = p.stressTensor + ds*(1.0f/m_timeStep);
				Tensor s_friction = p.stressTensor  + ds*1.0f;

				//s_friction = s_friction - (1.0f/3.0f)*s_friction.trace()*Tensor::Identity();

				// yield conditionon s_friction
				Real t = m_frictionCoefficient*p.pressure;
				for( int j=0;j<2;++j )
					for( int i=0;i<2;++i )
					{
						// yield constrain
						//if( fabsf(s_friction.m[j][i]) < t )
						//	s_friction.m[j][i] = math::sign(s_friction.m[j][i]) * t;
					}

				// cohesion on  ...

				p.stressTensor = s_friction;
				//p.stressTensor._11 = 0.0f;
				//p.stressTensor._12 = 0.0f;
				//p.stressTensor._21 = 0.0f;
				

				/*
				// compute corrective dissipative stress
				Tensor corrective_dissipative_stress = (strain_rate_dissipation);

				// update dissipative stress
				p.stressTensor += corrective_dissipative_stress;

				// make stress Tensor traceless
				//Tensor frictionStress;

				// traceless deviatoric part
				//frictionStress = p.stressTensor - (1.0f/3.0f)*p.stressTensor.trace()*Tensor::Identity();

				// test yield and cohesion
				Real t = m_frictionCoefficient*p.pressure;
				for( int j=0;j<3;++j )
					for( int i=0;i<3;++i )
					{
						// yield constrain
						if( fabsf(frictionStress.m[j][i]) < t )
							frictionStress.m[j][i] = math::sign(frictionStress.m[j][i]) * t;
					}

				//p.stressTensor = frictionStress;
				*/
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
			Vector f_pressure( 0.0f, 0.0f, 0.0f );

			// corrective friction force (only used when m_friction is true)
			Vector f_friction( 0.0f, 0.0f, 0.0f );

			for( Particle::Neighbours::iterator it2 = p.neighbours.begin(); it2 != p.neighbours.end();++it2 )
			{
				Particle &n = *(it2->p);
				Real distance = it2->distance;

				Vector gradW = gradW_spiky_2d( distance, p.predictedPosition - n.predictedPosition );
				//Vector gradW = gradW_poly6_2d( distance, p.predictedPosition - n.predictedPosition );

				// pressure force ---
				if(m_pressure)
					f_pressure += n.mass * ( p.pressure/(p.predictedMassDensity*p.predictedMassDensity) + n.pressure/(n.predictedMassDensity*n.predictedMassDensity) ) * gradW;

				// friction force ---
				if( m_friction )
				{
					//f_friction += n.mass * math::transform( gradW, p.stressTensor/(p.predictedMassDensity*p.predictedMassDensity) + n.stressTensor/(n.predictedMassDensity*n.predictedMassDensity) );
					math::Vec2f tmp = n.mass * math::transform( math::Vec2f(gradW.x, gradW.y), p.stressTensor/(p.predictedMassDensity*p.predictedMassDensity) + n.stressTensor/(n.predictedMassDensity*n.predictedMassDensity) );
					f_friction += Vector( tmp.x, tmp.y, 0.0f );
				}

			}




			p.pciPressureForce = p.mass*f_pressure;
			p.pciFrictionForce = p.mass*f_friction*1.0f;

			//std::cout << "f_pressure " << p.pciPressureForce.x << " " << p.pciPressureForce.y << " " << p.pciPressureForce.z << std::endl;
		}
	};
	// for each particle
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;


		if( p.states.testFlag( Particle::STATE_BOUNDARY ) )
			continue;


		// add forces which we got fom pci-scheme
		p.forces += p.pciPressureForce;
		if( m_friction )
			p.forces += p.pciFrictionForce;



		if(m_boundary)
		{
			// predict velocity/position
			Vector f = p.forces;
			Vector acceleration = f*(1.0f/p.mass);

			Vector impuls = p.impulses;

			// verlet
			integrate_verlet( p.position, p.velocity, p.predictedPositionPrev, acceleration, impuls, m_timeStep, m_damping, p.predictedPosition, p.predictedVelocity );



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


	// xdebug
	Real maxpressure = 0.0f;
	for( ParticleContainer::iterator it = m_particles.begin(); it != m_particles.end();++it )
	{
		Particle &p = *it;

		//maxDensityFluctuation = std::max( maxDensityFluctuation, p.massDensity - m_restDensity );
		maxpressure = std::max( maxpressure, p.pressure );

		if( p.id == 81 )
		{
			//std::cout << "friction force " << p.pciFrictionForce.x << " " << p.pciFrictionForce.y << " " << p.pciFrictionForce.z << std::endl;
			//std::cout << "friction force " << p.forces.x << " " << p.forces.y << " " << p.forces.z << std::endl;
			//std::cout << "friction force " << p.temp2.x << " " << p.temp2.y << " " << p.temp2.z << std::endl;
			/*
			std::cout << "stress tensor" << p.stressTensor.m[0][0] << " " << p.stressTensor.m[0][1] << " " << p.stressTensor.m[0][2] << std::endl;
			std::cout << "             " << p.stressTensor.m[1][0] << " " << p.stressTensor.m[1][1] << " " << p.stressTensor.m[1][2] << std::endl;
			std::cout << "             " << p.stressTensor.m[2][0] << " " << p.stressTensor.m[2][1] << " " << p.stressTensor.m[2][2] << std::endl;
			*/
		}
	}
	//std::cout << "maxDensityFluctuation " << maxDensityFluctuation << std::endl;
	//std::cout << "maxpressure " << maxpressure << std::endl;



	// 
	timeIntegration();

	m_currentTimeStep++;
}


void SPH::updateSupportRadius( Real newSupportRadius )
{
	m_supportRadius = newSupportRadius;
	m_supportRadiusSquared = m_supportRadius*m_supportRadius;

	// update weighting kernels
	W_poly6_2d_precompute(m_supportRadius);
	W_spiky_2d_precompute(m_supportRadius);
}

void SPH::initialize()
{
	updateSupportRadius( .190625f );
	//updateSupportRadius( .31f );
	//updateSupportRadius( .21f );

	m_idealGasConstant = 0.1f;
	//m_restDensity = 998.29;
	//m_restDensity = 1000.0f;
	m_restDensity = 1000.0f;

	m_particles.clear();

	// granular
	m_cricitalDensity = 1000.0f;
	m_frictionCoefficient = 1.0f;


	// solver parameters ---
	m_particleMass = 3.8125f; // 0.02f for water
	//m_particleMass = 3.0f; // testing
	m_timeStep = 0.005f;
	//m_damping = 0.01f;
	m_damping = 0.01f; // testing

	// switches for different solvertypes

	m_friction = true;
	m_pressure = false;
	m_unilateralIncompressibility = false;
	m_boundary = false;


	m_gravity = false;
	m_deformtest = true;



	// compute pci delta and stress delta
	{

		// TODO: find out how many particles we are supposed to put into the neighbourhood and where to put them

		// prototype particle
		Vector p0 = Vector(0.0f, 0.0f, 0.0f);

		// completely random: number of particles per unit length in each dimension
		int res = 5;
		Real s = m_supportRadius;


		// PCI - pressure delta
		Real beta = (m_particleMass*m_timeStep)/m_restDensity;
		beta = beta*beta;
		Vector sumGrad = 0.0f;
		Real sumGradDots = 0.0f;

		// granular - corrective stress coefficient
		Tensor outerproduct_gradW = Tensor::Zero();

		for( int k=0;k<res;++k )
			for( int j=0;j<res;++j )
				for( int i=0;i<res;++i )
				{
					Real u = (Real)i/(Real)res;
					Real v = (Real)j/(Real)res;
					Real w = (Real)k/(Real)res;
					Vector pn = Vector(2.0f*u*s-s, 2.0f*v*s-s, 2.0f*w*s-s);
					Real distance = (p0 - pn).getLength();		

					Vector gradW = gradW_spiky_2d( distance, p0 - pn );

					// pci - pressure delta
					sumGrad += gradW;
					sumGradDots += math::dot( gradW, gradW );

					// granular - corrective stress coefficient
					//outerproduct_gradW += (1.0f/m_restDensity)*math::outerProduct( gradW, gradW );
					//outerproduct_gradW += (1.0f/m_restDensity)*math::outerProduct( math::Vec2f(gradW.x, gradW.y), math::Vec2f(gradW.x, gradW.y) );
				}

		m_pciDelta = (-1.0f)/(beta*(math::dot(sumGrad, sumGrad) - sumGradDots ));

		/*
		m_pciStressDeltaInverse = (2.0f*m_particleMass*m_particleMass*m_timeStep)/(m_restDensity*m_restDensity)*outerproduct_gradW;
		m_pciStressDeltaInverse.invert();
		std::cout << "m_pciStressDeltaInverse: " << m_pciStressDeltaInverse.m[0][0] << " " << m_pciStressDeltaInverse.m[0][1] << std::endl;
		std::cout << "                         " << m_pciStressDeltaInverse.m[1][0] << " " << m_pciStressDeltaInverse.m[1][1] << std::endl;
		*/


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
					Vector gradW = gradW_poly6_2d( distance, p0 - pn );

					// granular - corrective stress coefficient
					testDing += (1.0f/m_restDensity)*math::outerProduct( math::Vec2f(gradW.x, gradW.y), math::Vec2f(gradW.x, gradW.y) );
				}
		Tensor D_inverse = -2.0f*m_timeStep*m_particleMass*m_particleMass*(1.0f/(m_restDensity*m_restDensity))*testDing;
		D_inverse.invert();
		m_pciStressDeltaInverse = D_inverse;
	}



	// initial fluid
	Real spacing = 0.19f;
	if(0)
	{
		int n = 10;
		for( int i=0;i<n;++i )
			for( int j=0;j<n;++j )
			{
				Particle p;

				initializeParticle(p, Vector( -1.5f + i * spacing, 1.0f + j * spacing, 0.0f ));

				m_particles.push_back(p);
			}
	}

	// stress debug setup
	if(1)
	{
		Real d = test_diff;

		//Real d =0.05f;
		Particle p;
		// center row
		initializeParticle(p, Vector( 0.0f, 0.0f, 0.0f )); // debug particle 1
		m_particles.push_back(p);
		initializeParticle(p, Vector( d, 0.0f, 0.0f ));// debug particle 2
		m_particles.push_back(p);


		///*
		initializeParticle(p, Vector( 0.5f*d, 0.75f*d, 0.0f ));// debug particle 2
		m_particles.push_back(p);
		//initializeParticle(p, Vector( 0.5f*d, -0.75f*d, 0.0f ));// debug particle 2
		//m_particles.push_back(p);
		//*/

		
		//initializeParticle(p, Vector( 0.0f, 0.5f*d, 0.0f )); // debug particle 1
		//m_particles.push_back(p);
		//initializeParticle(p, Vector( d, 0.5f*d, 0.0f ));// debug particle 2
		//m_particles.push_back(p);




		/*
		initializeParticle(p, Vector( -d, 0.0f, 0.0f ));m_particles.push_back(p);
		initializeParticle(p, Vector( -d*2.0f, 0.0f, 0.0f ));m_particles.push_back(p);
		initializeParticle(p, Vector( d*2.0f, 0.0f, 0.0f ));m_particles.push_back(p);
		initializeParticle(p, Vector( d*3.0f, 0.0f, 0.0f ));m_particles.push_back(p);

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
		initializeParticle(p, Vector( 0.0f, d*2.0f, 0.0f ));m_particles.push_back(p);
		initializeParticle(p, Vector( d, d*2.0f, 0.0f ));m_particles.push_back(p);

		// row -2
		initializeParticle(p, Vector( 0.0f, -d*2.0f, 0.0f ));m_particles.push_back(p);
		initializeParticle(p, Vector( d, -d*2.0f, 0.0f ));m_particles.push_back(p);
		*/
	}

	// debug
	//m_particles[90].trajectory = new Trajectory();

	// some wall 
	if(m_boundary)
	{
		//math::Matrix44f xform = math::Matrix44f::RotationMatrixZ( math::degToRad(-45.0f) );
		math::Matrix44f xform = math::Matrix44f::Identity();
		spacing *= 0.5f;
		int n = 50;
		for( int i=0;i<n;++i )
			for( int j=0;j<2;++j )
			{
				Particle p;

				initializeParticle(p, math::transform( Vector( -2.5f + i * spacing, 0.5f - j * spacing, 0.0f ), xform ) );
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

void SPH::W_poly6_3d_precompute( Real supportRadius )
{
	W_poly6_3d_h2 = supportRadius*supportRadius;
	W_poly6_3d_coeff = 315.0f/( 64.0f*MATH_PIf*powf(supportRadius, 9.0f) );
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

SPH::Real W_poly6_2d_h;
SPH::Real W_poly6_2d_h2;
SPH::Real W_poly6_2d_coeff;
SPH::Real W_poly6_2d_grad_coeff;

void SPH::W_poly6_2d_precompute( Real supportRadius )
{
	W_poly6_2d_h = supportRadius;
	W_poly6_2d_h2 = supportRadius*supportRadius;
	W_poly6_2d_coeff = 4.0f/( MATH_PIf*powf(supportRadius, 8.0f) );
	W_poly6_2d_grad_coeff = -24.0f/(MATH_PIf*powf(supportRadius, 8.0f));
	//W_poly6_3d_precompute(supportRadius);
}
SPH::Real SPH::W_poly6_2d( Real distance )
{

	if( distance > W_poly6_2d_h )
		return 0.0f;
	Real r2 = distance*distance;
	Real t = (W_poly6_2d_h*W_poly6_2d_h)-r2;
	return W_poly6_2d_coeff*t*t*t;

	//return W_poly6_3d( distance );
}

SPH::Vector SPH::gradW_poly6_2d( Real distance, Vector dir )
{
	Real t = W_poly6_2d_h2 - distance*distance;
	return W_poly6_2d_grad_coeff*t*t*dir;
}


// spiky =====================================================
SPH::Real W_spiky_h;
SPH::Real W_spiky_coeff;
SPH::Real W_spiky_grad_coeff;


void SPH::W_spiky_3d_precompute( Real supportRadius )
{
	W_spiky_h = supportRadius;
	W_spiky_coeff = 15.0f/( MATH_PIf*powf(supportRadius, 6.0f) );
	W_spiky_grad_coeff = -45.0f / (MATH_PIf*powf(supportRadius, 6.0f));
}
SPH::Real SPH::W_spiky_3d( Real distance )
{
	if( distance > W_spiky_h )
		return 0.0f;
	Real t = W_spiky_h - distance;
	return W_spiky_coeff*t*t*t;
}
SPH::Vector SPH::gradW_spiky_3d( Real distance, Vector dir )
{
	Real t = (W_spiky_h - distance );
	return W_spiky_grad_coeff*dir*(1.0f/distance)*t*t;
}


SPH::Real W_spiky_2d_h;
SPH::Real W_spiky_2d_coeff;
SPH::Real W_spiky_2d_grad_coeff;

void SPH::W_spiky_2d_precompute( Real supportRadius )
{
	//W_spiky_3d_precompute(supportRadius);
	W_spiky_2d_h = supportRadius;
	W_spiky_2d_coeff = 10.0f/(MATH_PIf*powf(supportRadius, 5.0f));
	W_spiky_2d_grad_coeff = -30.0f/(MATH_PIf*supportRadius*supportRadius*supportRadius*supportRadius);
}
SPH::Real SPH::W_spiky_2d( Real distance )
{
	//return W_spiky_3d(distance);
	if( distance > W_spiky_2d_h )
		return 0.0f;
	Real t=W_spiky_2d_h-distance;
	return W_spiky_2d_coeff*( t*t*t );
}
SPH::Vector SPH::gradW_spiky_2d( Real distance, Vector dir )
{
	//return gradW_spiky_3d(distance, dir);
	if( distance > W_spiky_2d_h )
		return 0.0f;
	Real q = distance/W_spiky_2d_h;
	Real t = 1.0f - q;
	return W_spiky_2d_grad_coeff*((t*t)/q)*dir;
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
	vOut = v + a*dt*(1.0f-damping);
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
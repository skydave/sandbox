varying vec4 pw;  // position in worldspace
varying vec2 uv;
varying vec3 uvw;

uniform vec3 wsLightPos;
uniform mat4 voxelToWorld;
uniform mat4 worldToVoxel;
uniform float totalCrossSection;
int lightSamples = 100;

#define PI 3.141593
#define TRANSMITTANCE_THRESHOLD 0.01

// this function is implemented by the datasource glsl module which has been linked
float queryDensity( vec3 vsPos );
bool intersectData( in vec3 rayPos, in vec3 rayDir, inout float t0, inout float t1 );

float fastexp( in float x )
{
	float x2 = x*x;
	float x3 = x2*x;
	return 1;
}

bool traversal( in vec4 wsIsectIn, in vec4 wsIsectOut,
				in vec3 vsIsectIn, in vec3 vsRayDir,
				in float vsStep, in int numSteps,
				in bool earlyOut,
				out int firstNonEmptyStep,
				out int stepsPerformed,
				out vec4 a0, out vec4 a1
				)
{
	float wsDistanceToTravel = distance( wsIsectIn.xyz, wsIsectOut.xyz );
	vec3 wsRayDir = ( wsIsectOut.xyz - wsIsectIn.xyz ) / wsDistanceToTravel;

	float wsStep = wsDistanceToTravel / float(numSteps);

	a0 = vec4(0.0);
	a1 = vec4(0.0);
	vec3 vsCurrentPos = vsIsectIn;

	//firstNonEmptyStep = -1;
	float transmittance = 1.0;
	float PIoverSteps = (PI / float(numSteps));
	vec3 vsDeltaRayDir = vsStep * vsRayDir;
	int i = 0;

	int tmp = -1;

	for( ; i < numSteps; ++i )
	{
		if ( earlyOut && transmittance < TRANSMITTANCE_THRESHOLD )
		{
			// abort traversal
			break;
		}

		float density = queryDensity( vsCurrentPos );

		//instead of 
		//if ( density > 0.0 && firstNonEmptyStep == -1 ) firstNonEmptyStep = i;
		tmp += int(transmittance); // TODO: CHECK OPTIMIZATION


		float factor = PIoverSteps * ( float(i) + 0.5 );

		// Chebyshev polynomials -> http://en.wikipedia.org/wiki/Chebyshev_polynomials
		// cos( int(k) * Theta ) = Tk( cos( Theta ) )
		// T0(x) = 1
		// T1(x) = x
		// Tk+1(x) = 2xTk(x) - Tk-1(x) --> Tk(x) = 2xTk-1(x) - Tk-2(x)

		float cosTheta = cos( factor );
		float cosTheta2 = 2.0 * cosTheta;
		float T0  = 1.0;
		float T1  = cosTheta;
		float T2  = cosTheta2 * T1 - T0;
		float T3  = cosTheta2 * T2 - T1;
		float T4  = cosTheta2 * T3 - T2;
		float T5  = cosTheta2 * T4 - T3;

		a0.x += transmittance * T0;
		a0.y += transmittance * T1;
		a0.z += transmittance * T2;
		a0.w += transmittance * T3;

		a1.x += transmittance * T4;
		a1.y += transmittance * T5;

		// the last 2 components are the t0, t1 distances of the lightray/box intersetcion
		//a1.z += transmittance * cos( factor * 14.0 );
		//a1.w += transmittance * cos( factor * 15.0 );
		transmittance *= exp( -density * wsStep * totalCrossSection );

		vsCurrentPos += vsDeltaRayDir;
	}

	a0.x *= 1.0 / sqrt(2.0);
	float coeff = sqrt(2.0/float(numSteps));
	a0 *= coeff;
	a1 *= coeff;

	stepsPerformed = i;
	firstNonEmptyStep = tmp;

	return stepsPerformed == numSteps;
}

void main()
{
	//discard;

	vec3 vsLightPos = (worldToVoxel * vec4(wsLightPos,1.0)).xyz;
	vec3 vsRayOrigin = vsLightPos;
	vec3 vsRayDir = normalize( uvw - vsLightPos );

	float t0, t1;

	bool isect = intersectData( vsRayOrigin, vsRayDir, t0, t1 );

	if ( isect )
    {

		vec3 vsIsectIn = vsRayOrigin + t0 * vsRayDir;
		vec3 vsIsectOut = vsRayOrigin + t1 * vsRayDir;
		vec4 wsIsectIn = voxelToWorld * vec4(vsIsectIn, 1.0);
		vec4 wsIsectOut = voxelToWorld * vec4(vsIsectOut, 1.0);

		float vsStep = ( t1 - t0 ) / float(lightSamples - 1);

		vec4 a0, a1;
		int stepsPerformed, firstNonEmptyStep;

		bool fullTraversal = traversal( wsIsectIn, wsIsectOut,
					vsIsectIn, vsRayDir,
					vsStep, lightSamples,
					true,
					firstNonEmptyStep,
					stepsPerformed, a0, a1 );
/*
		if(!fullTraversal)
		{
			vsIsectIn = vsRayOrigin + (t0 + float(firstNonEmptyStep - 1) * vsStep ) * vsRayDir;
			wsIsectIn = voxelToWorld * vec4(vsIsectIn, 1.0);

			vsIsectOut = vsRayOrigin + ( t0 + float(stepsPerformed) * vsStep ) * vsRayDir;
			wsIsectOut = voxelToWorld * vec4(vsIsectOut, 1.0);

			vsStep = distance( vsIsectIn, vsIsectOut ) / float(lightSamples - 1);
			traversal( wsIsectIn, wsIsectOut,
					   vsIsectIn, vsRayDir,
					   vsStep, lightSamples,
					   false,
					   firstNonEmptyStep,
					   stepsPerformed,
					   a0, a1
					   );
		}
*/


		// if a0 and a1 are 0 there is no transmittance along the ray
		//if ( a0 == vec4(0.0) && a1.xy == vec2(0.0) ) discard; // TODO: CHECK OPTIMIZATION

		vec4 wsRayOrigin = voxelToWorld * vec4(vsRayOrigin, 1.0);

		// the last 2 components are the t0, t1 distances of the lightray/box intersetcion
		float wsT0 = distance( wsIsectIn.xyz, wsRayOrigin.xyz ) * sign(t0); // t0 in world space
		float wsT1 = distance( wsIsectOut.xyz, wsRayOrigin.xyz ) * sign(t1); // t1 in world space

		a1.z = wsT0;
		a1.w = wsT1;

		gl_FragData[0] = a0;
		gl_FragData[1] = a1;
	}else
	{
		gl_FragData[0] = vec4(1.0,1.0,1.0, 1.0);
		gl_FragData[1] = vec4(1.0,1.0,1.0, 1.0);
	}
}

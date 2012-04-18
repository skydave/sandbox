
#extension GL_EXT_texture_array : enable

varying vec4 pw;  // position in worldspace
varying vec2 uv;

uniform sampler2D volumeFront;
uniform sampler2D volumeBack;
uniform sampler2DArray coeffTex;
uniform sampler2D noiseTex;

uniform mat4 voxelToWorld;

uniform vec3 wsLightPos;
uniform vec3 lightColor;
uniform mat4 worldToLightProj;
int numLightSamples = 100; // has to be in sync with dctcompute


uniform vec3 totalCrossSection;
uniform vec3 scatteringCrossSection;
// temp (uniforms)
vec3 emissionCrossSection = vec3( 0.0, 0.0, 0.0 );

#define PI 3.141593


// this function is implemented by the datasource glsl module which has been linked
float queryDensity( vec3 vsPos );


float visibility( vec4 wsSample )
{
    vec4 shadowCoord = worldToLightProj * wsSample;
    shadowCoord.xyz /= shadowCoord.w;
    shadowCoord.xy = ( shadowCoord.xy + vec2(1.0) ) * 0.5; // [-1,1] to [0,1]

	// TODO: CHECK OPTIMIZED by using clamp to border
	if ( shadowCoord.x < 0.0 ||
		 shadowCoord.y < 0.0 ||
		 shadowCoord.x > 1.0 ||
		 shadowCoord.y > 1.0 )
		 // out of bound of shadowmap
		return 0.0;


	vec4 a0 = texture2DArray( coeffTex, vec3(shadowCoord.xy, 0.0f) );
	vec4 a1 = texture2DArray( coeffTex, vec3(shadowCoord.xy, 1.0f) );

	// spot light
	float x = distance( wsSample.xyz, wsLightPos.xyz );

	float minDepth = a0.x;
	float maxDepth = a0.y;

	if ( x < minDepth ) return 1.0;
	if ( x > maxDepth ) return 0.0;


	float wsDistanceToTravel = maxDepth - minDepth;
	float n = ((x-minDepth)/wsDistanceToTravel)*float(numLightSamples);
	float PIoverSteps = (PI / float(numLightSamples));
	float factor = PIoverSteps*(n+0.5f);


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


	float transmittance = (1.0 / sqrt(2.0)) * a0.z * T0; // start with first coeff

	transmittance += a0.w*T1;
	transmittance += a1.x*T2;
	transmittance += a1.y*T3;
	transmittance += a1.z*T4;
	transmittance += a1.w*T5;

	transmittance *=  sqrt( 2.0f/float(numLightSamples) );


	return clamp(transmittance, 0.0, 1.0);
}



void main()
{

	// inputs
	float wsStepSize = 0.01;

	//


	vec4 front = texture2D( volumeFront, uv );
	vec4 back = texture2D( volumeBack, uv );

	// fetch noise value for current pixel
	vec2 viewportSize = vec2( 800, 600 );
	float noiseSize = 512;
	vec2 noiseTexUVScaleFactor = viewportSize / noiseSize;
	vec2 noiseOffset = uv * noiseTexUVScaleFactor;
	vec4 noise = texture2D( noiseTex, noiseOffset );

	float frontDepth = front.w;
	float backDepth = back.w;

	float totalDistance = backDepth-frontDepth;

	if( totalDistance == 0.0 )
		discard;


	//
	vec3 vsStart = front.xyz;
	vec3 vsEnd = back.xyz;

	vec3 vsCurrent = vsStart;
	vec3 vsRayDir = normalize(vsEnd - vsStart);
	float vsTotalDistance = length(vsEnd - vsStart);


	vec4 wsStart = voxelToWorld*vec4(vsStart, 1);
	vec4 wsEnd = voxelToWorld*vec4(vsEnd, 1);
	vec4 wsCurrent = wsStart;
	vec3 wsRayDir = normalize( wsEnd.xyz - wsStart.xyz );


	float wsTotalDistance = length( wsEnd - wsStart );


	int numSteps = int(wsTotalDistance / wsStepSize);

	vec3 vsRayStep = vsRayDir*(vsTotalDistance/float(numSteps));

	// offset first sample pos by some different random value each pixel - this will reduce banding artefacts
	//vsCurrent += vsRayStep * noise.r*10.0;
	vsCurrent += vsRayStep * noise.r*5.0;

	// raymarch
	vec3 transmittance = vec3(1.0);
	vec3 l = vec3(0.0, 0.0, 0.0); // total light towards the viewer

	for( int i = 0 ; i < numSteps; i++ )
	{

		float densityValue = queryDensity( vsCurrent );

		float ndl = densityValue*wsStepSize;
		vec3 opticalDepth = ndl * totalCrossSection;
		vec3 transmittanceStep = exp(-opticalDepth);

		// inscatter
		// spot light
		vec4 wsCurrent = voxelToWorld * vec4( vsCurrent, 1.0);
        vec3 wsFromLight = normalize( wsCurrent.xyz - wsLightPos.xyz );
        vec3 wsToEye = -wsRayDir;
		float phaseCoeff = 1.0;

		// for lightfallofversion
		//float ttt = 2.05;
		//lightColor * (1 - (x/ttt))
		vec3 lri =  lightColor*visibility( wsCurrent )*phaseCoeff;
		vec3 li = lri * scatteringCrossSection;

		// emission
		//vec3 le = texture( colorTex, vsCurrent ).xyz * emissionCrossSection;
		vec3 le = vec3( 0.0, 0.0, 0.0 );

		// proceed
		vsCurrent += vsRayStep;
		l += ( li + le ) * ndl * transmittance;
		transmittance *= transmittanceStep;
	}


	gl_FragData[0] = vec4(l*(1.0-transmittance), 1.0-transmittance.x);
}

//
// ssao using a blend of iq's approach and blizzards implementation for starcraft2
//
// outstanding issues:
//	performance
//		 use downsampled zbuffer, use smaller random normals texture, tweak shader code
//  artefacts when samples go out of the screen (edge artefacts)
//  artefacts when camera comes close to geometry
//

varying vec2 uv;
varying vec3 ev; // eye direction in view space

uniform sampler2D normal;
uniform sampler2D rndnorms;

// 16 random vectors
vec3 pSphere[16] = vec3[](vec3(0.53812504, 0.18565957, -0.43192),vec3(0.13790712, 0.24864247, 0.44301823),vec3(0.33715037, 0.56794053, -0.005789503),vec3(-0.6999805, -0.04511441, -0.0019965635),vec3(0.06896307, -0.15983082, -0.85477847),vec3(0.056099437, 0.006954967, -0.1843352),vec3(-0.014653638, 0.14027752, 0.0762037),vec3(0.010019933, -0.1924225, -0.034443386),vec3(-0.35775623, -0.5301969, -0.43581226),vec3(-0.3169221, 0.106360726, 0.015860917),vec3(0.010350345, -0.58698344, 0.0046293875),vec3(-0.08972908, -0.49408212, 0.3287904),vec3(0.7119986, -0.0154690035, -0.09183723),vec3(-0.053382345, 0.059675813, -0.5411899),vec3(0.035267662, -0.063188605, 0.54602677),vec3(-0.47761092, 0.2847911, -0.0271716));

void main()
{
	float rad = 1.5;	// sample radius scaling
	float maxzd = 0.7;  // defines the maximum difference of samplez and sampleeyespacepositionz which affects occlusion

	vec4 normal_depth = texture2D(normal,uv);
	float depth = normal_depth.w;

	if( depth == 1.0 )
		discard;

	vec3 n = normal_depth.xyz;

	// compute view space position of current fragment
	vec3 pc = depth*normalize(ev);

	// get random normal
	vec3 rndNorm = normalize(texture2D(rndnorms,uv*(512.0/128.0)).xyz); // * xres/normaltextureres

	// Set initial variables
	float occlusion = 0.0;
	int actualSamples = 0;

	// for each sample
	for( int i=0; i<16; i++ )
	{
		// create sample from random vector
		// we reflect the vector on a random plane (given by a random normal vector) to get more random samples
		vec3 offset = reflect(pSphere[i]*rad, rndNorm);

		// we sample hemisphere defined through point normal only to prevent self occlusion
		// if dotproduct is negative we flip the offsetvector back into hemisphere
		if( dot(offset, n) < 0.0 )
			offset = -offset;

		// compute the camera space sample position
		vec3 sc = pc + offset;

		// compute screen space position of sample
		vec2 ss = (sc.xy/-sc.z)*vec2(.75,1.0);

		// map from screen space into uv space
		vec2 suv = ss*0.5 + vec2(0.5);

		// get depth at screen space sample
		float sampleDepth = texture2D(normal,suv).w;

		// if the sample touches background we skip
		if( sampleDepth == 1.0 )
		{
			occlusion += 1.0;
			actualSamples++;
			continue;
		}

		// compute difference of length of the sample in cameraspace and the actuall depth value
		float zd = length(sc)-sampleDepth;


		// if that difference is too big we skip because we dont want objects being far in front of the current centerpixel
		// be occluding stuff which is very far behind (this would create black halos)
		if( zd > 0.7 )
		{
			continue;
		}


		// now map zd on a occlusion function (I use iq's approach)
		zd = 15.0*max( zd, 0.0 );
		occlusion += 1.0/(1.0+zd*zd);              // occlusion = 1/( 1 + 2500*max{dist,0)^2 )

		actualSamples++;
	}



	vec4 result = vec4(0.0);
	if( actualSamples > 0 )
		result = vec4(1.0 - occlusion/float(actualSamples));
	else
		result = vec4(0.0);

	gl_FragData[0] = result;
}

// data source module for Volume which retrieves data from a 3dtexture

#define sort2(a,b) { vec3 tmp=min(a,b); b=a+b-tmp; a=tmp; }

float fbm3d( vec3 p, int octaves, float lacunarity, float gain );
float perlinnoise3d( vec3 P );

uniform mat4 ellipsoidToSphere[2];
uniform int numClouds;

uniform sampler3D implicitfield;
uniform sampler3D noisetex;












float densityEllipsoid( int ellipsoid, vec3 vsPos )
{
	vec4 vsPos_sphere = ellipsoidToSphere[ellipsoid] * vec4( vsPos, 1.0 );

	float b = 1.0; // maxdistance the field contributes

	// transform ellipsoid into canoncial sphere space
	float d = length( vsPos_sphere.xyz );
	d = clamp( d, 0, b );
	//if( d < b )
	{
		/*
		float r2 = d*d;
		float r4 = r2*r2;
		float r6 = r4*r2;
		float b2 = b*b;
		float b4 = b2*b2;
		float b6 = b4*b2;
		*/

		// wyvill blending
		//float density = 1.0 - (4.0*r6)/(9.0*b6) + (17.0*r4)/(9.0*b4) - (22.0*r2)/(9.0*b2);
		float density = 0.1;
		return density;
	}
	//return 0.0;
	//return step( -b, -d )*density;
}

float fast_fbm3d( vec3 p, int octaves, float lacunarity, float gain )
{
	float tt = 10.0;
	float amp = pow(gain,float(octaves));
	float sum = 0.0;
	vec3 point = fract(p);
	//vec3 point = p;
	for(int i = 0; i<octaves; ++i)
	{
		float noise = texture3D( noisetex, point ).a*tt;
		sum += (noise * amp);
		amp *= 1.0/gain;
		point *= 1.0/lacunarity;
	}
	return sum;
}

float queryDensity( vec3 vsPos )
{
/*
	if( vsPos.y < 0.2 )
		return 1.0;
	if( (length(vsPos.xz - vec2(0.5,0.5)) < 0.1)&&(vsPos.y < 0.7) )
		return 1.0;
	return 0.0;
*/

	float density = 0.0;
	float noise = 0.0;


	float frequency = 8.0;
	int octaves = 6;
	float lacunarity = 2.0;
	float gain = 0.59;

	//if( vsPos.z < 0.5 )
	if(1)
	{
		noise = fbm3d( vsPos*frequency, octaves, lacunarity, gain )*0.12;
		//noise = perlinnoise3d( vsPos*256.0 );
	}
	else
	{
		frequency = 8.0;
		octaves = 1;
		lacunarity = 2.0;
		gain = 0.5;
		noise = fast_fbm3d(vsPos, octaves, lacunarity, gain);


		//noise = texture3D( noisetex, vsPos ).a*5.0;
	}

	vec3 p;
	p.x = vsPos.x + noise; 
	p.y = vsPos.y - noise;
	p.z = vsPos.z + noise;

	density = texture3D( implicitfield, p ).a;
	density = pow( density, 0.5 );

	// alpha cutoff
	float alphaCutoff = 0.0;

	return step(alphaCutoff, density )*density;

}

bool intersectData( in vec3 rayPos, in vec3 rayDir, inout float t0, inout float t1 )
{
	vec3 bbMin = vec3(0.0, 0.0, 0.0);
	vec3 bbMax = vec3( 1.0, 1.0, 1.0 );
	vec3 dL = vec3(1.0/rayDir.x, 1.0/rayDir.y, 1.0/rayDir.z);
	vec3 lo = (bbMin - rayPos) * dL;
	vec3 hi = (bbMax - rayPos) * dL;
	sort2(lo, hi);
	bool hit = !( lo.x>hi.y || lo.y>hi.x || lo.x>hi.z || lo.z>hi.x || lo.y>hi.z || lo.z>hi.y );
	t0 = max(max(lo.x, lo.y), lo.z);
	t1 = min(min(hi.x, hi.y), hi.z);
	return hit;
}

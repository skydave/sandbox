attribute vec3 P;
attribute vec3 N;
attribute vec2 UV;

varying vec3 pw;
varying vec3 n;
varying vec2 uv;

uniform mat4 mvpm;
uniform mat3 mvminvt;

vec3 noise2d( vec2 P );
vec3 noise2d_org( vec2 P );

vec3 turb2d( vec2 p, int numIterations );

//
// general
//
uniform float           time;

uniform float      maxVertexHeight; // max height of cloud layer in meters

// perlin noise parameters
uniform int       pn_octaves;
uniform float   pn_frequency;



float fbm_tex( in vec2 uv, in vec2 off )
{
	return turb2d( uv*pn_frequency+off, pn_octaves ).x*0.5+0.5;
}

vec3 flowTexture( in vec2 flow, in vec2 uv, in float time, in float noiseScale, in float interval, in float speed  )
{
	float halfInterval = interval*0.5;
	vec2 uv0 = uv;
	vec2 uv1 = uv + vec2( 0.764, 0.214 );

	float v = noise2d(uv*noiseScale).x;

	float ttime0 = mod(time + v, interval);
	float ttime1 = mod(time + v + halfInterval, interval);
	float t0 = ttime0/interval; // get weight (0-1)


	if( t0 > 0.5)
	{
		t0 = 1.0 - t0;
	}
	t0 = 2.0*t0;


	vec2 flowuv0 = uv0 + (ttime0-halfInterval)*speed*flow;
	vec2 flowuv1 = uv1 + (ttime1-halfInterval)*speed*flow;


	float val0 = fbm_tex( mod(flowuv0,1.0), vec2(0.0) );
	float val1 = fbm_tex( mod(flowuv1,1.0), vec2(0.0) );
	float val = val0*t0 + val1*(1.0 - t0);


	return vec3(val, 0.0, 0.0 );
}

void main()
{
	///*
	//vec2 fbmOffset = vec2(time);
	//float fbm = turb2d( UV*pn_frequency+fbmOffset, pn_octaves ).x*0.5 + 0.5;
	//*/
	vec2 flow = vec2( sin(UV.y*5.0), cos(UV.x*7.0) );
	//vec2 flow = vec2( 1.0 );
	float fbm = flowTexture( flow, UV, time, 1.0, 2.5, 0.05 ).x;

	float H = fbm*maxVertexHeight;
	vec3 P2 = P + vec3(0.0, H, 0.0);
	pw = P2;
	//n = mvminvt * N;  // normal in view space
	n = N;  // normal in world space
	uv = UV;
	gl_Position = mvpm * vec4(pw, 1.0);
}

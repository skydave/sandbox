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

void main()
{
	vec2 fbmOffset = vec2(time);
	float fbm = turb2d( UV*pn_frequency+fbmOffset, pn_octaves ).x*0.5 + 0.5;
	float H = fbm*maxVertexHeight;
	vec3 P2 = P + vec3(0.0, H, 0.0);
	pw = P2;
	//n = mvminvt * N;  // normal in view space
	n = N;  // normal in world space
	uv = UV;
	gl_Position = mvpm * vec4(pw, 1.0);
}

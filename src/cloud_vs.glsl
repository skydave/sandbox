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

void main()
{
	float maxHeight = 500.0;
	float fbm = turb2d( UV*20.0, 8 ).x*0.5 + 0.5;
	float H = fbm*maxHeight;
	vec3 P2 = P + vec3(0.0, H, 0.0);
	pw = P2;
	//n = mvminvt * N;  // normal in view space
	n = N;  // normal in world space
	uv = UV;
	gl_Position = mvpm * vec4(pw, 1.0);
}

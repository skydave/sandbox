attribute vec3 P;
attribute vec3 N;
attribute vec2 UV;

varying vec3 pw;
varying vec3 n;
varying vec2 uv;

uniform mat4 mvpm;
uniform mat3 mvminvt;

float fbm2d( vec2 p, int octaves, float lacunarity, float gain );


void main()
{
	float fbm = fbm2d( UV*20.0, 2, 2.0, 0.5 )*0.5+0.5;
	float H = fbm*500.0;
	vec3 P2 = P + vec3(0.0, H, 0.0);
	pw = P2;
	//pw = P;
	//n = mvminvt * N;  // normal in view space
	n = N;  // normal in world space
	uv = UV;
	gl_Position = mvpm * vec4(pw, 1.0);
}

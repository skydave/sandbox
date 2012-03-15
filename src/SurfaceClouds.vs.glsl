attribute vec3 P;
attribute vec3 N;
attribute vec2 UV;

varying vec3 pw;
varying vec3 n;
varying vec2 uv;

uniform mat4 mvpm;
uniform mat3 mvminvt;

uniform float      maxVertexHeight; // max height of cloud layer in meters

float fbm2d( vec2 p, int octaves, float lacunarity, float gain );


void main()
{
	pw = P;

	float fbm = fbm2d( UV*20.0, 2, 2.0, 0.5 )*0.5+0.5;
	float H = fbm*maxVertexHeight;
	pw += vec3(0.0, H, 0.0);


	//n = mvminvt * N;  // normal in view space
	n = N;  // normal in world space
	uv = UV;
	gl_Position = mvpm * vec4(pw, 1.0);
}

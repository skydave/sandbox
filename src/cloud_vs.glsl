attribute vec3 P;
attribute vec3 N;
attribute vec2 UV;

varying vec3 pw;
varying vec3 n;
varying vec2 uv;

uniform mat4 mvpm;
uniform mat3 mvminvt;

void main()
{
	pw = P;
	//n = mvminvt * N;  // normal in view space
	n = N;  // normal in world space
	uv = UV;
	gl_Position = mvpm * vec4(pw, 1.0);
}

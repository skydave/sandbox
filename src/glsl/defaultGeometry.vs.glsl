attribute vec3 P;
attribute vec3 N;
attribute vec2 UV;
attribute vec3 Cd;

varying vec3 pw;
varying vec3 n;
varying vec2 uv;
varying vec3 cd;

uniform mat4 mvpm;
uniform mat3 mvminvt;

void main()
{
	pw = P;
	n = mvminvt * N;
	uv = UV;
	cd = Cd;
	gl_Position = mvpm * vec4(pw, 1.0);
}

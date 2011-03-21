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
	n = mvminvt * N;
	uv = UV;
	gl_Position = mvpm * vec4(pw, 1.0);
}

attribute vec3 P;
attribute vec3 N;
attribute vec2 UV;
attribute vec3 Cd;

varying vec3 pv;
varying vec3 pw;
varying vec3 n;
varying vec2 uv;
varying vec3 cd;

uniform mat4 mvpm;
uniform mat4 mvm;
uniform mat3 mvminvt;

void main()
{
	pw = P;
	pv = (mvm * vec4(P,1.0)).xyz; // positino in eyespace
	n = mvminvt * N; // normal in eyespace
	uv = UV;
	cd = Cd;
	gl_Position = mvpm * vec4(P, 1.0);
}

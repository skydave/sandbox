
attribute vec3 P;
attribute vec3 Cd;

varying vec2 uv;
varying vec3 cd;

uniform mat4 mvpm;

void main()
{
	cd = Cd;
	gl_Position = mvpm * vec4(P,1.0);
}

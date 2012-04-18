attribute vec3 P;
attribute vec3 UVW;

uniform mat4 mvpm;

varying vec3 uvw;

void main()
{
	gl_Position = mvpm * vec4(P, 1.0);
	uvw = UVW;
}
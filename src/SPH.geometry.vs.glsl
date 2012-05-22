attribute vec3 P;
attribute vec3 N;

uniform mat4 mvpm;
uniform mat3 mvminvt;

varying vec3 n;

void main()
{
	n = N;
	gl_Position = mvpm * vec4(P, 1.0);
}

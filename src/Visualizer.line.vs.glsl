attribute vec3 P;
attribute vec3 Cd;

uniform mat4 mvpm;
uniform mat3 mvminvt;

varying vec3 cd;

void main()
{
	cd = Cd;
	gl_Position = mvpm * vec4(P, 1.0);
}

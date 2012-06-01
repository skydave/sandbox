attribute vec3 P;
attribute vec3 Cd;
attribute vec3 S;

uniform mat4 mvpm;
uniform mat3 mvminvt;

varying vec3 cd;

void main()
{
	//cd = vec3(1.0, 1.0, 1.0);
	cd = Cd;
	gl_Position = mvpm * vec4(P, 1.0);
	gl_PointSize = 3.0;
}

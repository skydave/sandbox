attribute vec3 P;
attribute vec3 N;
attribute vec3 offset;

uniform mat4 mvpm;
uniform mat3 mvminvt;
uniform mat4 vm;

varying vec3 n;

void main()
{
	n = N;

	//screen-aligned axes
	vec3 axis1 = vec3(  vm[0][0],
						vm[1][0],
						vm[2][0]);

	vec3 axis2 = vec3(  vm[0][1],
						vm[1][1],
						vm[2][1]);

	vec3 corner = P + (offset.x*axis1 + offset.y*axis2);

	gl_Position = mvpm * vec4(corner, 1.0);
}

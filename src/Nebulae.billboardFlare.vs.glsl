attribute vec3 P;
attribute vec3 N;
attribute vec3 offset;

uniform mat4 mvpm;
uniform mat3 mvminvt;
uniform mat4 vm;

varying vec3 n;
varying vec2 uv;

uniform float scale;

void main()
{
	n = N;

	// use position straight
	vec3 p = P;

	//screen-aligned axes
	vec3 axis1 = vec3(  vm[0][0],
						vm[1][0],
						vm[2][0]);

	vec3 axis2 = vec3(  vm[0][1],
						vm[1][1],
						vm[2][1]);

	vec3 corner = p + (offset.x*axis1 + offset.y*axis2)*scale;
	uv = vec2( 0.5, 0.5 ) + offset;


	gl_Position = mvpm * vec4(corner, 1.0);
}

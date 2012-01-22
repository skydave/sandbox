attribute vec3 P;
attribute vec3 N;
attribute vec3 offset;

uniform mat4 mvpm;
uniform mat3 mvminvt;
uniform mat4 vm;

varying vec3 n;
varying vec2 uv;
varying vec3 c;

uniform float scale = 1.0;
uniform sampler2D pos;
uniform sampler2D col;

void main()
{
	n = N;

	// use position straight
	//vec3 p = P;
	// read billboard position from particle texture:
	vec3 p = texture2D(pos, P.xy).xyz;


	//screen-aligned axes
	vec3 axis1 = vec3(  vm[0][0],
						vm[1][0],
						vm[2][0]);

	vec3 axis2 = vec3(  vm[0][1],
						vm[1][1],
						vm[2][1]);

	vec3 corner = p + (offset.x*axis1 + offset.y*axis2)*scale;
	uv = vec2( 0.5, 0.5 ) + offset;

	c = texture2D(col, P.xy).xyz;

	gl_Position = mvpm * vec4(corner, 1.0);
}

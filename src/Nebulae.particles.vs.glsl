attribute vec3 P;
attribute vec3 N;

uniform mat4 mvpm;
uniform mat3 mvminvt;

varying vec3 n;
varying vec3 c;

uniform sampler2D pos;
uniform sampler2D col;
uniform float scale;

void main()
{
	n = N;
	c = texture2D(col, P.xy).xyz;

	// read particle position from particle texture
	//gl_Position = mvpm * vec4(P, 1.0);
	gl_Position = mvpm * texture2D(pos, P.xy).xyzw;
	gl_PointSize = scale;
}

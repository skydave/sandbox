attribute vec3 P;
attribute vec3 N;

uniform mat4 mvpm;
uniform mat3 mvminvt;

varying vec3 n;

uniform sampler2D pos;
uniform float scale;

void main()
{
	n = N;
	// read particle position from particle texture
	//gl_Position = mvpm * vec4(P, 1.0);
	gl_Position = mvpm * texture2D(pos, P.xy).xyzw;
	gl_PointSize = scale;
}

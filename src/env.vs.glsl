attribute vec3 P;
attribute vec3 N;
attribute vec2 UV;

uniform mat4 mvpm;
uniform mat3 mvminvt;

varying vec3 n;
varying vec2 uv;

void main()
{
	n = N;
	uv = UV;
	// read particle position from particle texture
	gl_Position = mvpm * vec4(P, 1.0);
	//gl_PointSize = 10.0;
}

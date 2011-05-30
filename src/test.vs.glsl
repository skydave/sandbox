attribute vec3 P;
attribute vec3 UV;

uniform mat4 mvpm;
uniform mat3 mvminvt;

varying vec2 uv;

void main()
{
	uv = UV;
	// read particle position from particle texture
	gl_Position = mvpm * vec4(P, 1.0);
}

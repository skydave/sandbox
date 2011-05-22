attribute vec3 P;

uniform mat4 mvpm;
uniform mat3 mvminvt;

void main()
{
	// read particle position from particle texture
	gl_Position = mvpm * vec4(P, 1.0);
}

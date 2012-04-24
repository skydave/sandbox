
attribute vec3  P;
attribute vec2 UV;


varying vec2 uv;

uniform mat4 mvpm;

void main()
{
	uv = UV;
	gl_Position = mvpm * vec4(P,1.0);
}

uniform mat4 vminv;

varying vec3 cd;

void main()
{
	gl_FragData[0] = vec4(cd, 1.0);
}

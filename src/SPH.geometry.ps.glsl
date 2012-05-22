varying vec3 n;

void main()
{
	gl_FragData[0] = vec4(n, 1.0);
}

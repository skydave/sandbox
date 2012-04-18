

varying vec3 uvw;

void main()
{
	gl_FragData[0] = vec4(uvw, (gl_FragCoord.z / gl_FragCoord.w) );
}

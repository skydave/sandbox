varying vec3 pw; // position in eye space
varying float depth; // depth in eye space
uniform sampler2D texture;

void main()
{
	//gl_FragData[0] = texture2D( texture, gl_FragCoord.st );
	//gl_FragData[0] = vec4(depth);
	gl_FragData[0] = vec4(pw, 1.0);
	//gl_FragData[0] = vec4(gl_FragCoord.z/gl_FragCoord.w);
	//gl_FragData[0] = vec4(gl_FragCoord.x/512.0);
}

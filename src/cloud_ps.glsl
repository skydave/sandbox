varying vec4 pw;  // position in worldspace
varying vec3 n;
varying vec2 uv;

uniform int oid;
uniform sampler2D input;

vec3 noise2d( vec2 P );
vec3 noise2d_org( vec2 P );

vec3 turb2d( vec2 p, int numIterations );

void main()
{

	//gl_FragData[0] = vec4(uv.x, uv.y, 0.0, 0.0);
	gl_FragData[0] = texture2D(input, uv);
	//gl_FragData[0] = vec4(turb2d( uv*100.0, 8 ).x*0.5+0.5);
	//gl_FragData[0] = vec4(noise2d_org( uv ).x);
	//gl_FragData[0] = texture2D(input, uv);
	//gl_FragData[0] = vec4(1.0, 0.0, 0.0, 1.0);

	//gl_FragData[0] = vec4(1.0, 1.0, uv.x, uv.y);
	//gl_FragData[1] = vec4(pw.x, pw.y, pw.z, (gl_FragCoord.z / gl_FragCoord.w));
	//gl_FragData[2] = vec4(n.x, n.y, n.z, (gl_FragCoord.z / gl_FragCoord.w));


	// NOTE: it seems 4 fboattachments are not supported by FX1400 - using objectbuffer for now...
	//gl_FragData[3] = vec4(uv.x, uv.y, 0.0, 1.0);
}
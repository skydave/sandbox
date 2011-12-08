varying vec4 pw;  // position in worldspace
varying vec3 n;
varying vec2 uv;
varying vec3 cd;

uniform int oid;
uniform sampler2D diffuseMap;

void main()
{
	vec4 tex = texture2D(diffuseMap,  uv );
	//gl_FragData[0] = vec4(uv.x, uv.y, 0.0, 0.0);
	//gl_FragData[0] = texture2D(input, uv);
	//gl_FragData[0] = texture2D(input, uv);
	//gl_FragData[0] = vec4(cd, 1.0);
	//gl_FragData[0] = texture2D(diffuseMap, uv);
	gl_FragData[0] = vec4(cd, tex.x);
	//gl_FragData[0] = vec4(1.0,1.0,1.0, tex.x);
	//gl_FragData[0] = vec4(1.0, 1.0, uv.x, uv.y);
	//gl_FragData[1] = vec4(pw.x, pw.y, pw.z, (gl_FragCoord.z / gl_FragCoord.w));
	//gl_FragData[2] = vec4(n.x, n.y, n.z, (gl_FragCoord.z / gl_FragCoord.w));


	// NOTE: it seems 4 fboattachments are not supported by FX1400 - using objectbuffer for now...
	//gl_FragData[3] = vec4(uv.x, uv.y, 0.0, 1.0);
}

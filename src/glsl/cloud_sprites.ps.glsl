varying vec4 pw;  // position in worldspace
varying vec3 n;
varying vec2 uv;
varying vec3 cd;

uniform int oid;
uniform sampler2D diffuseMap;

uniform mat4 vminv;
uniform mat4 mm;
uniform mat4 vm;

vec3 getCameraPos()
{
	// vmatrix transforms from camera to world space
	// vmatrix inverse transforms from world to camera space
	// we transpose it to inverse the inverse
	mat4 t = transpose(vminv);
	return vec3( t[0].w, t[1].w, t[2].w );
}

void main()
{
	vec4 pv = (vm * mm) * pw;
	vec3 V = normalize(vec3(0.0, 0.0, 0.0) - pv.xyz);
	vec4 tex = texture2D(diffuseMap,  uv );



	//gl_FragData[0] = vec4(uv.x, uv.y, 0.0, 0.0);
	//gl_FragData[0] = texture2D(input, uv);
	//gl_FragData[0] = texture2D(input, uv);
	//gl_FragData[0] = vec4(cd, 1.0);
	//gl_FragData[0] = texture2D(diffuseMap, uv);

	// color from normals
	//float alpha = clamp(dot( normalize(n), V ), 0.0, 1.0);
	float t = clamp(dot( normalize(n), V ), 0.0, 1.0);
	float mi = 0.1;
	float alpha = step( mi, t )*((t-mi)/(1.0-mi));
	//gl_FragData[0] = vec4(alpha, alpha, alpha, 1.0);

	// lookup texture for alpha, color from vertex colors
	//gl_FragData[0] = vec4(cd, tex.x*alpha);
	gl_FragData[0] = vec4(cd, tex.x*alpha);
	//gl_FragData[0] = vec4(cd, 1.0*alpha);

	//gl_FragData[0] = vec4(1.0,1.0,1.0, tex.x);
	//gl_FragData[0] = vec4(1.0, 1.0, uv.x, uv.y);
	//gl_FragData[1] = vec4(pw.x, pw.y, pw.z, (gl_FragCoord.z / gl_FragCoord.w));
	//gl_FragData[2] = vec4(n.x, n.y, n.z, (gl_FragCoord.z / gl_FragCoord.w));


	// NOTE: it seems 4 fboattachments are not supported by FX1400 - using objectbuffer for now...
	//gl_FragData[3] = vec4(uv.x, uv.y, 0.0, 1.0);
}

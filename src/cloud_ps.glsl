varying vec4 pw;  // position in worldspace
varying vec3 n;
varying vec2 uv;

uniform int oid;
uniform sampler2D input;
uniform sampler2D parameters;

vec3 noise2d( vec2 P );
vec3 noise2d_org( vec2 P );

vec3 turb2d( vec2 p, int numIterations );

float b( float cos_theta )
{
	return texture2D(parameters, vec2(cos_theta, 0.25)).r;
}

float c( float cos_theta )
{
	return texture2D(parameters, vec2(cos_theta, 0.25)).g;
}

float kc( float cos_theta )
{
	return texture2D(parameters, vec2(cos_theta, 0.25)).b;
}

float t( float cos_theta )
{
	return texture2D(parameters, vec2(cos_theta, 0.25)).a;
}

float r( float cos_theta )
{
	return texture2D(parameters, vec2(cos_theta, 0.75)).r;
}

void main()
{

	//gl_FragData[0] = vec4(uv.x, uv.y, 0.0, 0.0);
	//gl_FragData[0] = texture2D(input, uv);
	//gl_FragData[0] = texture2D(parameters, uv);
	//gl_FragData[0] = texture2D(parameters, vec2(uv.x, 0.75));
	gl_FragData[0] = vec4(b(uv.x));
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

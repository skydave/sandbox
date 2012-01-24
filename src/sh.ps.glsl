uniform sampler2D envTex;
uniform sampler2D starsTex;
uniform samplerCube envTex2;
uniform mat4 vminv;

uniform vec3 Li[9];


varying vec3 n;
varying vec2 uv;
varying vec3 p;


float PI = 3.14159265358979323846264;


vec3 getCameraPos()
{
	// vmatrix transforms from camera to world space
	// vmatrix inverse transforms from world to camera space
	// we transpose it to inverse the inverse
	mat4 t = transpose(vminv);
	return vec3( t[0].w, t[1].w, t[2].w );
}

vec2 cubemapLookup( vec3 n )
{
	vec2 uv;

	vec3 absN = abs(n);

	// nrm is the world normal from a vertex
	// absNrm is the same normal, but with absolute values
	if (absN.x > absN.y && absN.x > absN.z)
	{
		if (n.x>0)
		{
			// right face
			vec3 p_n = vec3(1.0,0.0,0.0);
			float d = dot(p_n, p_n)/dot(n,p_n);
			uv = 1.0 - (d*n*0.5 + 0.5).zy;

			uv = uv*vec2(0.333,0.25) + vec2(0.666,0.25);
		}
		else
		{
			// left face
			vec3 p_n = vec3(-1.0,0.0,0.0);
			float d = dot(p_n, p_n)/dot(n,p_n);
			uv = (d*n*0.5 + 0.5).zy;
			uv.y = 1.0 - uv.y;

			uv = uv*vec2(0.333333,0.25) + vec2(0.0,0.25);
		}
	} else
	if (absN.y > absN.x && absN.y > absN.z)
	{
		if (n.y>0)
		{
			// top face
			vec3 p_n = vec3(0.0,1.0,0.0);
			float d = dot(p_n, p_n)/dot(n,p_n);
			uv = (d*n*0.5 + 0.5).xz;

			uv = uv*vec2(0.333,0.25) + vec2(0.333,0.0);
		}
		else
		{
			// bottom face
			vec3 p_n = vec3(0.0,-1.0,0.0);
			float d = dot(p_n, p_n)/dot(n,p_n);
			uv = (d*n*0.5 + 0.5).xz;
			uv.y = 1.0 - uv.y;

			uv = uv*vec2(0.333,0.25) + vec2(0.333,0.5);
		}
	}else
	{
		if (n.z>0)
		{
			// front face
			vec3 p_n = vec3(0.0,0.0,1.0);
			float d = dot(p_n, p_n)/dot(n,p_n);
			uv = (d*n*0.5 + 0.5).xy;
			uv.y = 1.0 - uv.y;

			uv = uv*vec2(0.333,0.25) + vec2(0.333,0.25);
		}
		else
		{
			// back face
			vec3 p_n = vec3(0.0,0.0,-1.0);
			float d = dot(p_n, p_n)/dot(n,p_n);
			uv = (d*n*0.5 + 0.5).xy;

			uv = uv*vec2(0.333333,0.25) + vec2(0.333333,0.75);			
		}
	}


	return uv;
}


/*
vec2 check( float p, float l )
{
	float l0, p1, l1, p2, l2;
	l0 = 0.0;
	p1 = -PI;
	l1 = -PI;
	p2 = -PI;
	l2 = PI;

	float lp = atan( (cos(p1)*sin(p2)*cos(l1) - sin(p1)*cos(p2)*cos(l2))/(sin(p1)*cos(p2)*sin(l2) - cos(p1)*sin(p2)*sin(l1)) );
	float pp = atan( -cos(lp-l1)/tan(p1) );
	float A = sin(pp)*sin(p) - cos(pp)*cos(p)*sin(l-l0);

	float x = atan((tan(p)*cos(pp) + sin(pp)*sin(l-lp))/cos(l-l0));
	float y = atanh(A);

	return vec2(x, y);
}
*/

vec2 cylindrical( float p, float l )
{

	float x = (l+PI)/(2.0*PI);
	float y = (p+ PI*0.5)/PI;

	return vec2(x, y);
}

vec2 cassini( float p, float l )
{

	float x = (asin(cos(p)*sin(l)) + PI*0.5)/PI;
	float y = 1.0 - (atan2( sin(p), cos(p)*cos(l) )+PI)/(2.0*PI);

	// flip vertically for opengl
	y = 1.0 - y;

	return vec2(x, y);
}

void main()
{
	vec3 nn = normalize(n); 


	// cartesian to spherical coords
	//float lambda = acos( nn.z ); // long
	//float phi = atan2( nn.y, nn.x ); // lat
	// spherical coordinates to cartesian
	//vec3 tt = vec3( cos(phi)*sin(lambda), sin(phi)*sin(lambda), cos(lambda) );


	// cartesian to lat long
	float lat = asin( nn.y );
	float long = atan2( nn.z, nn.x );


	//vec2 uvt = cassini(lat, long);
	vec2 uvt = cylindrical(lat, long);
	vec3 t = texture2D( starsTex, uvt );

	gl_FragData[0] = vec4(t, 1.0);
	//gl_FragData[0] = vec4(vec3(uvt.x), 1.0);
	//gl_FragData[0] = vec4(vec3(tt), 1.0);
	//gl_FragData[0] = vec4(vec3(lambda), 1.0);
	//gl_FragData[0] = vec4(vec3((long+PI)/(2.0*PI)), 1.0);
	//gl_FragData[0] = vec4(vec3((lat+(PI/2.0))/PI), 1.0);
	//gl_FragData[0] = vec4(vec3(nn), 1.0);
	//gl_FragData[0] = texture2D( envTex, cubemapLookup( n ) );
	//gl_FragData[0] = textureCube( envTex2, n );
}

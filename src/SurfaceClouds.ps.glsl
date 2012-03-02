varying vec4              pw;  // position in worldspace
varying vec3               n;
varying vec2              uv;

uniform vec3          sunDir;


uniform mat4           vminv;


float fbm2d( vec2 p, int octaves, float lacunarity, float gain );


vec3 getCameraPos()
{
	// vmatrix transforms from camera to world space
	// vmatrix inverse transforms from world to camera space
	// we transpose it to inverse the inverse
	mat4 t = transpose(vminv);
	return vec3( t[0].w, t[1].w, t[2].w );
}

vec4 phong(in vec3 n,in vec3 v,in vec3 l)
{
	vec3      ambient;  // ambient color
	vec3      diffuse;  // diffuse color
	vec3      specular; // specular color
	float     exponent; // exponent value
	float           ka;
	float           kd;
	float           ks;
	ambient = vec3( 0.05, 0.05, 0.05 ); ka = 1.0;
	diffuse = vec3( 0.5, 0.5, 0.5 ); kd = 1.0;
	specular = vec3( 1.0, 1.0, 1.0 ); ks = 1.0;
	exponent = 10.1;
	vec3 a = ka*ambient;
	vec3 d = kd*max(dot(n,l),0.0)*diffuse;
	vec3 s = exponent==0.0 ? vec3(0.0) : ks*pow(max(dot(reflect(-l,n),v),0.0),exponent)*specular;

	return vec4(a+d+s,1.0);
}

vec4 lambert(in vec3 n,in vec3 v,in vec3 l)
{
	vec3      ambient;  // ambient color
	vec3      diffuse;  // diffuse color
	float           ka;
	float           kd;
	ambient = vec3( 0.05, 0.05, 0.05 ); ka = 1.0;
	diffuse = vec3( 0.5, 0.5, 0.5 ); kd = 1.0;
	vec3 a = ka*ambient;
	vec3 d = kd*max(dot(n,l),0.0)*diffuse;
	return vec4(a+d,1.0);
}


void main()
{
	vec3 L = normalize(sunDir);
	vec3 E = normalize(getCameraPos() - pw.xyz);
	vec3 N = n;


	float fbm = fbm2d( uv*20.0, 8, 2.0, 0.5 );
	float delta = 0.01;
	float fbm_dx = (fbm - fbm2d( (uv+vec2(delta, 0.0))*20.0, 8, 2.0, 0.5 ))/delta;
	float fbm_dy = (fbm - fbm2d( (uv+vec2(0.0, delta))*20.0, 8, 2.0, 0.5 ))/delta;
	vec3 dfbm = vec3(fbm_dx, 0.0, fbm_dy);
	N = normalize( n - dfbm );

	// do some fake lighting to check
	//gl_FragData[0] = phong(N, E, L);
	gl_FragData[0] = lambert(N, E, L);

	//gl_FragData[0] = vec4(uv.x, uv.y, 0.0, 0.0);
	//gl_FragData[0] = vec4(1.0, 1.0, 1.0, 1.0);
}
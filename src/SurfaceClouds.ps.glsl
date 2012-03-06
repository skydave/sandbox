float PI = 3.14159265358979323846264;

varying vec4              pw;  // position in worldspace
varying vec3               n;
varying vec2              uv;


uniform sampler2D parameters;
uniform vec3          sunDir;
uniform vec3           C_sun;
uniform float             re; // effective radius in micro meter
uniform float             N0; // in cm^-3
uniform float            P_f; // forward scattering weight/share of P_theta
uniform float        theta_f; // angle which seperates forward scattering in radians


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
	vec3 s = exponent==0.0 ? vec3(0.0) : ks*pow(max(dot(-reflect(l,n),v),0.0),exponent)*specular;
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

float P( float theta )
{
	if( theta > theta_f )
		return texture2D(parameters, vec2(theta/PI, 0.75)).g/(1.0-P_f);
	else
		return 0.0f;
	//return texture2D(parameters, vec2(theta/PI, 0.75)).g;
}


void main()
{
	vec3 L = normalize(sunDir);
	vec3 E = normalize(getCameraPos() - pw.xyz);
	vec3 N = normalize(n);

	float fbm = fbm2d( uv*20.0, 2, 2.0, 0.5 )*0.5+0.5;
	float delta = 0.0075;
	float fbm_du = (fbm - (fbm2d( (uv+vec2(delta, 0.0))*20.0, 2, 2.0, 0.5 )*0.5+0.5));
	float fbm_dv = (fbm - (fbm2d( (uv+vec2(0.0, delta))*20.0, 2, 2.0, 0.5 )*0.5+0.5));

	vec3 dx = vec3( 1.0, fbm_du, 0.0 );
	vec3 dz = vec3( 0.0, fbm_dv, 1.0 );
	N = normalize( cross(dz, dx) );

	// user input ---
	float maxHeight = 500.0;

	// compute some globals ======================================
	//float H = fbm*maxHeight;
	float H = 0.5*maxHeight;
	float theta_el = acos(dot( E, L ));
	//float mu_l = max(dot( N, L ), 0.0);
	//float mu_e = max(dot( N, E ), 0.0);
	float mu_l = dot( N, L );
	float mu_e = dot( N, E );

	float H_l = H / mu_l;
	float H_e = H / mu_e;

	// single scattering contribution ============================
	float Ir_1 = (1.0-P_f)*N0*PI*re*re*P(theta_el)*(mu_l/(mu_e+mu_l))*(1.0-exp( -(1.0-P_f)*N0*PI*re*re*(H_l+H_e) ));


	vec3 result = Ir_1*C_sun*10.0;

	// do some fake lighting to check
	//gl_FragData[0] = phong(N, E, L);
	gl_FragData[0] = vec4(result, 1.0);
	//gl_FragData[0] = lambert(N, E, L);

	//gl_FragData[0] = vec4(uv.x, uv.y, 0.0, 0.0);
	//gl_FragData[0] = vec4(1.0, 1.0, 1.0, 1.0);
}
varying vec4 pw;  // position in worldspace
varying vec3 n;
varying vec2 uv;

uniform int oid;
uniform mat4 vminv;
uniform sampler2D input;

float PI = 3.14159265358979323846264;


vec3 noise2d( vec2 P );
vec3 noise2d_org( vec2 P );

vec3 turb2d( vec2 p, int numIterations );

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



//
// cloud shader
//
uniform sampler2D parameters;
uniform vec3          sunPos;
uniform float             Pf; // forward scattering weight/share of P_theta
uniform float        theta_f; // angle which seperates forward scattering in radians
uniform float             re; // effective radius in micro meter
uniform float             N0; // in cm^-3
uniform float           beta; // ?


float maxHeight = 500.0;      // max height of cloud layer in meters
float     H = maxHeight;      // height of current shaded slab

vec3         L;               // vector from sample to light
vec3         N;               // normal at sample
vec3         E;               // vector from sample to eye position
float       ml;               // L.N
float       me;               // E.N

float theta_el;               // angle between E and V

float       Hl;               // distance light ray travels through slab
float       He;               // distance eye ray travels through slab


vec4 Csun = vec4(1.0, 1.0, 1.0, 1.0);
vec4 Csky = vec4(.5, .5, 0.5, 1.0);
vec4 Cground = vec4( vec3(0.1), 1.0);


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

// takes theta which goes from 0 to pi
float P_theta( float theta )
{
	return texture2D(parameters, vec2(theta/PI, 0.75)).g;
}

float Ps( float theta )
{
	if( theta > theta_f )
		return P_theta(theta)/(1.0 - Pf);
	return 0.0;
}

float PF( float theta )
{
	if(theta > theta_f)
		return P_theta(theta)/Pf;
	return 0.0;
}

float Os()
{
	return (1.0 - Pf)*PI*re*re;
}

float Ks()
{
	return Os()*N0;
}

float Taus( float x )
{
	return exp(-Ks()*x);
}

float Ss( float x )
{
	return Ks()*exp(-Ks()*x);
}


void setN( vec3 normal )
{
	N = normal;
	ml = clamp(dot( N, L ), 0.0, 1.0);
	me = clamp(dot( N, E ), 0.0, 1.0);
	Hl = H / ml;
	He = H / me;
}

void main()
{
	//
	// prepare parameters
	//

	// compute height of slab
	float fbm = turb2d( uv*20.0, 8 ).x*0.5+0.5;
	float delta = 0.1;
	float fbm_dx = ((turb2d( uv*20.0+vec2(delta, 0.0), 8 ).x*0.5+0.5)-fbm)/delta;
	float fbm_dy = ((turb2d( uv*20.0+vec2(0.0, delta), 8 ).x*0.5+0.5)-fbm)/delta;
	vec3 dfbm = vec3(fbm_dx, 0.0, fbm_dy);
	H = fbm*maxHeight;


	//L = normalize(sunPos - pw.xyz);
	//L = normalize(vec3(10.0,1.0,0.0)); // pseudo-specular
	L = normalize(vec3(10.0,2.0,0.0));
	//L = normalize(vec3(0.0,1.0,0.0));
	E = normalize(getCameraPos() - pw.xyz);
	theta_el = cos(dot( E, L ));

	vec3 Z = normalize(n);
	vec3 localN = normalize( n - dfbm );


	//for Ir3 we use global normal (0,1,0)
	setN(Z);

	// compute Ir3
	float Tms = (b(ml) + (1.0 - b(ml))*exp(-c(ml)*H))  *  (beta/(H-(H-1.0)*beta));
	float Rms = 1.0 - Tms;
	float R2 = 0.5*r(ml)*t(ml)* ( 1.0 - (2.0*H*kc(ml)+1.0)*exp(-kc(ml)*2.0*H) );
	float R1 = 0.5 * r(ml) * (1.0 - exp(-kc(ml)*2.0*H));
	float R3 = Rms - R1 - R2;
	float Ir3 = R3 * (ml/(4.0*PI*me));

	// compute T0 (transparency)
	float T0 = Taus(Hl);

	// change normal to local normal (which has high frequency detail from fbm)
	setN(localN);

	// (re)compute Tms using local normal
	Tms = (b(ml) + (1.0 - b(ml))*exp(-c(ml)*H))  *  (beta/(H-(H-1.0)*beta));

	// compute Ir2
	float Ir2 = ((Ks()*Ps(theta_el)*ml) / (me+ml))  *  (1.0 - Taus(Hl + He));
	//float Ir2 = ((ml) / (me+ml)) * (1.0 - Taus(Hl + He));
	//float Ir2 =  (1.0 - Taus(Hl + He));
	//float Ir2 = Taus(Hl + He);

	// compute Ir1
	//float Ir1 = ((Ks()*Ps(theta_el)*ml) / (ml+me))  *  (1.0 - Taus(Hl + He));
	//float Ir1 = ((Ks()*ml*2.0) / (ml+me) )*  (1.0 - Taus(Hl + He));
	float Ir1 = ((Ks()*Ps(theta_el)*ml) / (ml+me))  *  (1.0 - Taus(Hl + He));

	// compute Ir
	float Ir = Ir1 + Ir2 + Ir3;
	//float Ir = Ir3*10.0;
	//float Ir = Ir1;
	//float Ir = Ir2;
	//float Ir = Ir3;
	//float Ir = Ir1 + Ir2;
	Ir = clamp(Ir,0.0,1.0);



	// compute final color
	//if( pw.x < 0.0 )
	{
		//gl_FragData[0] = Ir*Csun;
		//gl_FragData[0] = Rms*Csky;
		//gl_FragData[0] = Tms*Cground;
		//gl_FragData[0] = Ir*Csun + Tms*Cground;
		gl_FragData[0] = Ir*Csun + Rms*Csky + Tms*Cground;
		//gl_FragData[0].a = T0*10000000000000000000000.0;
		//gl_FragData[0].r = N.x;
		//gl_FragData[0].g = N.y;
		//gl_FragData[0].b = N.z;
		//gl_FragData[0] = vec4(1.0-T0);
		//gl_FragData[0] = vec4(0.0);
		//gl_FragData[0].a = fbm;// - T0;
		gl_FragData[0].a = 1.0-T0;
		//gl_FragData[0].a = 1.0;
		//gl_FragData[0] = Rms*Csky;
		//gl_FragData[0] = Ir*Csun;

		gl_FragData[0].r = gl_FragData[0].r*gl_FragData[0].a + 1.0*(1.0-gl_FragData[0].a);
		gl_FragData[0].g = gl_FragData[0].g*gl_FragData[0].a + 1.0*(1.0-gl_FragData[0].a);
		gl_FragData[0].b = gl_FragData[0].b*gl_FragData[0].a + 1.0*(1.0-gl_FragData[0].a);
	}//else
	{
		//gl_FragData[0] = vec4(fbm, fbm, fbm, T0);
	}





	// do some fake lighting to check
	//gl_FragData[0] = lambert(N, E, L);




	//gl_FragData[0] = vec4(uv.x, uv.y, 0.0, 0.0);
	//gl_FragData[0] = texture2D(input, uv);
	//gl_FragData[0] = texture2D(parameters, uv);
	//gl_FragData[0] = texture2D(parameters, vec2(uv.x, 0.75));
	//gl_FragData[0] = vec4(P_theta(uv.x*PI));
	//gl_FragData[0] = vec4(Pf);
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

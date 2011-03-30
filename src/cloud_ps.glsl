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
	if( theta < theta_f )
	return P_theta(theta)/(1.0 - Pf);
}

float PF( float theta )
{
	if(theta < theta_f)
		return 0.0;
	return P_theta(theta)/Pf;
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


void main()
{
	// prepare parameters
	vec3 sunDir = -vec3(.8, 1.0, .8);
	//vec3 L = normalize(sunPos - pw.xyz);
	vec3 L = normalize(-sunDir);
	vec3 N = normalize(n);
	vec3 E = normalize(getCameraPos() - pw.xyz);
	vec3 Z = vec3(0.0, 1.0, 0.0);
	float ml = dot( N, L );
	float me = dot( N, E );

	float theta_vl = cos(dot( E, L ));

	float maxHeight = 500.0;
	float H = (turb2d( uv*20.0, 8 ).x*0.5+0.5)*maxHeight;
	float Hl = H / ml;
	float He = H / me;

	vec4 Csun = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 Csky = vec4(.5, .5, 0.8, 1.0);
	vec4 Cground = vec4( 0.54, 0.46, 0.39, 1.0);


	// compute Ir3
	float Tms = (b(ml) + (1.0 - b(ml))*exp(-c(ml)*H))  *  (beta/(H-(H-1.0)*beta));
	float Rms = 1.0 - Tms;
	float R2 = 0.5*r(ml)*t(ml)* ( 1.0 - (2.0*H*kc(ml)+1.0)*exp(-kc(ml)*2.0*H) );
	float R1 = 0.5 * r(ml) * (1.0 - exp(-kc(ml)*2.0*H));
	float R3 = Rms - R1 - R2;
	// TODO: use Z for Ir3
	float Ir3 = R3 + ml/(4.0*PI*me);

	// compute Ir2
	float Ir2 = ((Ks()*Ps(theta_vl)*ml) / (me+ml))  *  (1.0 - Taus(Hl + He));

	// compute Ir1
	float Ir1 = ((Ks()*Ps(theta_vl)*ml) / (ml+me))  *  (1.0 - Taus(Hl + He));

	// compute Ir
	float Ir = Ir1 + Ir2 + Ir3;

	// compute T0
	T0 = Taus(Hl);


	// compute final color
	//gl_FragData[0] = Ir*Csun + Rms*Csky + Tms*Cground;
	gl_FragData[0] = Ir*Csun + Rms*Csky*0.01 + Tms*Cground;
	gl_FragData[0].a = T0*10000000000000000000000.0;
	//gl_FragData[0] = Rms*Csky;
	//gl_FragData[0] = Ir*Csun;





	// do some fake lighting to check
	//vec4 Cd = vec4(0.75, 0.75, 0.75, 1.0);
	//gl_FragData[0] = Cd*H;



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

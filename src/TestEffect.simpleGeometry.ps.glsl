//
// WIP
//

varying vec3 pv;  // position in worldspace
varying vec3 n;
varying vec2 uv;
varying vec3 cd;
varying vec3 pw;

uniform vec3 l;

uniform mat4 vminv;
uniform mat4 mvm;
uniform mat3 mvminvt;

uniform sampler2D depthMap0;
uniform mat4   viewToLight0;
uniform vec3      lightPos0; // light position in view space (of the camera)

vec3 ambient = vec3( 0.05, 0.05, 0.05 );
uniform float ka;
vec3 diffuse = vec3( 0.3, 0.3, 0.3 );
uniform float kd;
vec3 specular = vec3( 1.0, 1.0, 1.0 );
uniform float ks;


void main()
{
	vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 N = normalize(n);
	vec3 V = normalize(pv.xyz);
	vec3 L = normalize(lightPos0 - pv);


	// shadow test =====
	vec4 shadowCoord = viewToLight0 * vec4(pv,1.0);
	vec3 ttt0 = shadowCoord.xyz;
	shadowCoord.xyz /= shadowCoord.w;
	vec3 ttt = shadowCoord.xyz;
	shadowCoord.xy = ( shadowCoord.xy + vec2(1.0) ) * 0.5; // [-1,1] to [0,1]
	float lightDistance = length(pv - lightPos0);
	vec3 temp = texture2D( depthMap0, shadowCoord.xy ).xyz;
	float lightDepth = length( (mvm*vec4(temp,1.0)).xyz - lightPos0);

	// to reduce zfighting
	lightDepth += 0.3;


	float visibility = 1.0;
	//gl_FragData[0] = vec4(lightDistance*0.1);
	if ( -ttt0.z > -0.1 ||
		shadowCoord.x < 0.0 ||
		shadowCoord.x > 1.0 ||
		shadowCoord.y < 0.0 ||
		shadowCoord.y > 1.0 ||
		lightDistance > lightDepth)
		visibility = 0.0;

	// phong ==============
	vec3 a = ka*ambient;
	float     exponent; // exponent value

	exponent = 10.1;
	vec3 d = kd*max(dot(N,L),0.0)*diffuse;

	vec3 s = exponent==0.0 ? vec3(0.0) : ks*pow(max(dot(reflect(L,N),V),0.0),exponent)*specular;

	vec3 p = visibility*s + d;

	if( dot(N,V) > 0.0 )
		result = vec4(a, 1.0);
	else
	{
		result = vec4(a,1.0) + vec4(visibility*p + (1.0-visibility)*p*0.3, 1.0);
	}


	gl_FragData[0] = result;
}

varying vec3 pv;  // position in worldspace
varying vec3 n;
varying vec2 uv;
varying vec3 cd;
varying vec3 pw;

uniform vec3 l;


uniform mat4 vminv;
uniform mat4 mvm;
uniform mat3 mvminvt;
uniform sampler2D texture;

uniform sampler2D depthMap0;
uniform mat4   viewToLight0;
//uniform mat4   testMat;
uniform vec3      lightPos0; // light position in view space (of the camera)

vec3 ambient = vec3( 0.05, 0.05, 0.05 );
float ka = 2.0;
vec3 diffuse = vec3( 0.5, 0.5, 0.5 );
float kd = 1.0;
vec3 specular = vec3( 1.0, 1.0, 1.0 );
float ks = 1.0;

vec4 phong(in vec3 n,in vec3 v,in vec3 l)
{
	float     exponent; // exponent value

	exponent = 10.1;
	vec3 a = ka*ambient;
	vec3 d = kd*max(dot(n,l),0.0)*diffuse;
	vec3 s = exponent==0.0 ? vec3(0.0) : ks*pow(max(dot(reflect(l,n),v),0.0),exponent)*specular;

	return vec4(a+d+s,1.0);
}

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
	//shadowCoord.y = 1.0 - shadowCoord.y;
	//float lightDistance = length(pv - lightPos0);
	//float lightDistance = length(pw - vec3(0.0,2.0,5.0));
	//float lightDistance = length(mvm*vec4(pw,1.0) - mvm*vec4(0.0,2.0,5.0, 1.0));
	//float lightDistance = length(mvm*vec4(pw,1.0) - vec4(lightPos0,1.0));
	float lightDistance = length(pv - lightPos0);
	vec3 temp = texture2D( depthMap0, shadowCoord.xy ).xyz;
	float lightDepth = length( (mvm*vec4(temp,1.0)).xyz - lightPos0);
	lightDepth += 0.3;
	//float lightDepth = -texture2D( depthMap0, shadowCoord.xy ).x;
	//vec3 test = texture2D( texture, shadowCoord.xy ).xyz;
	float visibility = 1.0;


	//gl_FragData[0] = vec4(lightDistance*0.1);
	if ( -ttt0.z > -0.1 ||
		shadowCoord.x < 0.0 ||
		shadowCoord.x > 1.0 ||
		shadowCoord.y < 0.0 ||
		shadowCoord.y > 1.0 ||
		lightDistance > lightDepth)
		visibility = 0.0;

	//result = visibility*vec4(lightDepth, lightDepth, lightDepth,1.0)*0.1;
	//result = visibility*vec4(abs(lightDepth-lightDistance));
	//result = vec4(visibility);
	//result = vec4(ttt0.z);

	
	//if( lightDistance > lightDepth )
	//	visibility = 0.0;

	//result = vec4(visibility);
	//result = vec4(lightDistance*0.1);
	//result = vec4(lightDepth*1.0);


	//diffuse = texture2D( texture, uv ).xyz;
	//visibility = 1.0;
	result = visibility*phong(N, V, L);

	//result = phong(N, V, L);
	gl_FragData[0] = result;
}

varying vec4 pw;  // position in worldspace
varying vec3 n;
varying vec2 uv;
varying vec3 ed; // eye direction in worldspace
varying vec3 ep; // eye position (on image plane) in worldspace

uniform mat4 vminv;

float PI = 3.14159265358979323846264;




uniform sampler2D transmittanceSampler;
uniform float innerRadius;
uniform float outerRadius;
uniform float      height;




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





void main()
{
	// get height and viewangle from position
	//float height = (length(ep) - innerRadius)/(outerRadius-innerRadius);
	float cosViewAngle = dot(normalize(ep), normalize(ed));

	
	// hemisphere
	float u = uv.x;
	float v = uv.y;
	vec3 p = vec3(u*2.0-1.0, 0.0, v*2.0-1.0);

	float r = 1.0;
	float d = r*r - p.x*p.x - p.z*p.z;
	if(d>0.0)
	{
		p.y = sqrt(d);
		float cosViewAngle = dot(vec3(0.0, 1.0, 0.0), normalize(p));
		vec4 result = texture2D( transmittanceSampler, vec2( cosViewAngle, height ) );
		gl_FragData[0] = result;
	}else
	{
		gl_FragData[0] = vec4(0.0, 0.0, 0.0, 0.0);
	}
	

	//gl_FragData[0] = vec4(uv.x, uv.y, 0.0, 0.0);
	//gl_FragData[0] = texture2D( transmittanceSampler, uv );
	//gl_FragData[0] = result*0.5 + texture2D( transmittanceSampler, uv )*0.5;
	//gl_FragData[0] = vec4( 0.0, 0.0, 0.0, 1.0 );
	//if( (cosViewAngle+1.0)*0.5 > 1.0 )
	//	gl_FragData[0] = vec4( 1.0, 0.0, 0.0, 1.0 );
	//gl_FragData[0] = result;
	//gl_FragData[0] = texture2D( transmittanceSampler, vec2( cosViewAngle*0.5 + 1.0, height ) );
	//gl_FragData[0] = texture2D( transmittanceSampler, vec2( cosViewAngle*0.5 + 1.0, height ) );
	//gl_FragData[0] = vec4( cosViewAngle*0.5 + 1.0, cosViewAngle*0.5 + 1.0, cosViewAngle*0.5 + 1.0, 1.0 );
	//gl_FragData[0] = vec4( cosViewAngle*0.5, cosViewAngle*0.5 + 1.0, cosViewAngle*0.5 + 1.0, 1.0 );
	//gl_FragData[0] = vec4( cosViewAngle*0.5 + 1.0, cosViewAngle*0.5 + 1.0, cosViewAngle*0.5 + 1.0, 1.0 );
}
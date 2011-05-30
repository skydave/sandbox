varying vec2 uv;

vec3 noise2d( vec2 P );
vec3 turb2d( vec2 p, int numIterations );

uniform float time;

void main()
{
	// comes from flow texture
	vec2 vel = vec2( 1.0, 1.0 );

	//distort uvs
	vec2 flowuv = uv + time*vel;

	float val = turb2d( flowuv*10.0, 8 ).x*0.5+0.5;
	//float val = noise2d( uv*10.0 );
	gl_FragData[0] = vec4(val, val, val, 1.0);
}

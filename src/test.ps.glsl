varying vec2 uv;

vec3 noise2d( vec2 P );
vec3 turb2d( vec2 p, int numIterations );

uniform float time;
uniform sampler2D tex;

void main()
{
	float interval = 0.5;
	float speed = 0.1;
	// comes from flow texture
	//vec2 vel = vec2( 1.0, 2.0 );
	vec2 vel = vec2( sin(uv.y*5), cos(uv.x*7) );

	uv = uv*2.0;

	vec2 uv0 = uv;
	vec2 uv1 = uv + vec2( 0.764, 0.214 );

	float v = noise2d(uv*1.25);

	time = time + v;

	float ttime0 = mod(time, interval);
	float ttime1 = mod(time+interval*0.5, interval);
	float t0 = ttime0/interval; // get weight (0-1)


	if( t0 > 0.5)
	{
		t0 = 1.0 - t0;
	}
	t0 = 2.0*t0;
	
	float t1 = 1.0 - t0;

	vec2 flowuv0 = uv0 + (ttime0-interval*0.5)*speed*vel;
	vec2 flowuv1 = uv1 + (ttime1-interval*0.5)*speed*vel;

	vec3 val0 = texture2D(tex,flowuv0);
	vec3 val1 = texture2D(tex,flowuv1);
	vec3 val = val0*t0 + val1*t1;
	//vec3 val = vec3(v);
	gl_FragData[0] = vec4(val, 1.0);
}

varying vec2 uv;

vec3 noise2d( vec2 P );
vec3 turb2d( vec2 p, int numIterations );

uniform float time;
uniform sampler2D tex;

void main()
{
	float interval = 5.0;
	// comes from flow texture
	vec2 vel = vec2( 1.0, 2.0 );

	float ttime = mod(time, interval);
	float t = ttime/interval; // get weight (0-1)

	if( t > 0.5)
		t = 1.0 - t;
	t = 2.0*t;
	
	float t2 = 1.0 - t;

	vec2 flowuv = uv + (ttime-interval*0.5)*0.1*vel;

	float val = texture2D(tex,flowuv).x;
	val = val*t;
	gl_FragData[0] = vec4(val, val, val, 1.0);
}

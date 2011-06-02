varying vec2 uv;

vec3 noise2d( vec2 P );
vec3 turb2d( vec2 p, int numIterations );

uniform float time;
uniform sampler2D tex;

//vec3 flowTexture( in sampler2D tex, in sampler2D flow, in sampler2D noise, in float interval, int float speed,  )

void main()
{

	float interval = 0.5;
	float speed = 0.4;
	// comes from flow texture
	//vec2 vel = vec2( 1.0, 2.0 );
	vec2 vel = vec2( sin(uv.y*5.0), cos(uv.x*7.0) );


	vec2 uv0 = uv*2.0;
	vec2 uv1 = uv*2.0 + vec2( 0.764, 0.214 );

	float v = noise2d(uv*1.25).x;

	float ttime0 = mod(time + v, interval);
	float ttime1 = mod(time + v + interval*0.5, interval);
	float t0 = ttime0/interval; // get weight (0-1)


	if( t0 > 0.5)
	{
		t0 = 1.0 - t0;
	}
	t0 = 2.0*t0;
	
	float t1 = 1.0 - t0;

	vec2 flowuv0 = uv0 + (ttime0-interval*0.5)*speed*vel;
	vec2 flowuv1 = uv1 + (ttime1-interval*0.5)*speed*vel;

	vec3 val0 = texture2D(tex,flowuv0).xyz;
	vec3 val1 = texture2D(tex,flowuv1).xyz;
	vec3 val = val0*t0 + val1*t1;
	//vec3 val = vec3(v);
	gl_FragData[0] = vec4(val, 1.0);

/*
	float HalfCycle = 0.5;
	float ttime = mod(time, HalfCycle*2.0 );
	float FlowMapOffset0 = ttime;
	float FlowMapOffset1 = ttime + HalfCycle;
	float TexScale = 2.0;


	vec2 vel = vec2( sin(uv.y*5.0), cos(uv.x*7.0) );
	float cycleOffset = noise2d(uv*1.25).x;
	
	float phase0 = cycleOffset * .5 + FlowMapOffset0;
	float phase1 = cycleOffset * .5 + FlowMapOffset1;

	// Sample texture
	vec3 texT0 = texture2D(tex, ( uv * TexScale ) + vel * phase0 ).xyz;
	vec3 texT1 = texture2D(tex, ( uv * TexScale ) + vel * phase1 ).xyz;

	float flowLerp = ( abs( HalfCycle - FlowMapOffset0 ) / HalfCycle );
	vec3 color = mix( texT0, texT1, flowLerp );
	//vec3 color = texT1;


	gl_FragData[0] = vec4(color, 1.0);
*/
}

varying vec2 uv;

uniform sampler2D inputPositions;

float perlinnoise2d( vec2 uv );
float perlinnoise3d( vec3 P );
float fbm3d( vec3 p, int octaves, float lacunarity, float gain );

uniform float frequency;
uniform float lacunarity;
uniform float gain;
uniform int octaves;

void main()
{
	// read particle position from particle texture
	vec4 p = texture2D(inputPositions, uv).xyzw;

	//modify positions
	vec3 disp;


	disp.x = fbm3d( p.xyz*frequency, octaves, lacunarity, gain );
	disp.y = fbm3d( p.xyz*frequency+vec3(100), octaves, lacunarity, gain );
	disp.z = fbm3d( p.xyz*frequency+vec3(200), octaves, lacunarity, gain );

	/*
	disp.x = fbm3d( p.xyz*frequency, octaves, lacunarity, 0.5 );
	disp.y = fbm3d( p.xyz*frequency+vec3(100), octaves, lacunarity, 0.5 );
	disp.z = fbm3d( p.xyz*frequency+vec3(200), octaves, lacunarity, 0.5 );
	*/

	float scale = 0.1;
	vec3 newP = vec3(p.x+disp.x, p.y+disp.y, p.z+disp.z)*scale;

	// write result
	gl_FragData[0] = vec4(newP, 1.0);
}

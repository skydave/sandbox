varying vec2 uv;

uniform mat4 mvpm;

uniform sampler2D inputPositions;

float perlinnoise2d( vec2 uv );
float perlinnoise3d( vec3 P );
float fbm3d( vec3 p, int octaves, float lacunarity, float gain );

uniform float frequency;
uniform int octaves;

void main()
{
	// read particle position from particle texture
	vec4 p = texture2D(inputPositions, uv).xyzw;

	//todo: modify positions
	vec3 disp;
	//disp.x = perlinnoise3d( p.xyz*frequency );
	//disp.y = perlinnoise3d( p.xyz*frequency+vec3(100) );
	//disp.z = perlinnoise3d( p.xyz*frequency+vec3(200) );


	disp.x = fbm3d( p.xyz*frequency, octaves, 2.0, 0.5 );
	disp.y = fbm3d( p.xyz*frequency+vec3(100), octaves, 2.0, 0.5 );
	disp.z = fbm3d( p.xyz*frequency+vec3(200), octaves, 2.0, 0.5 );


	//gl_FragData[0] = vec4(uv.x, uv.y, perlinnoise2d(uv), 1.0);
	gl_FragData[0] = vec4(p.x+disp.x, p.y+disp.y, p.z+disp.z, 1.0);

	// write result
	//gl_FragData[0] = vec4(p.x, p.y, p.z, 1.0);
}

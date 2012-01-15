varying vec2 uv;

uniform mat4 mvpm;

uniform sampler2D inputPositions;

float perlinnoise2d( vec2 uv );
float perlinnoise3d( vec3 P );

uniform float frequency = 1.0;

void main()
{
	// read particle position from particle texture
	vec4 p = texture2D(inputPositions, uv).xyzw;

	//todo: modify positions
	vec3 disp;
	disp.x = perlinnoise3d( p.xyz*frequency );
	disp.y = perlinnoise3d( p.xyz*frequency+vec3(100) );
	disp.z = perlinnoise3d( p.xyz*frequency+vec3(200) );

	//gl_FragData[0] = vec4(uv.x, uv.y, perlinnoise2d(uv), 1.0);
	gl_FragData[0] = vec4(p.x+disp.x, p.y+disp.y, p.z+disp.z, 1.0);

	// write result
	//gl_FragData[0] = vec4(p.x, p.y, p.z, 1.0);
}

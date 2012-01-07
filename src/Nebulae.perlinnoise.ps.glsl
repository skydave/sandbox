varying vec2 uv;

uniform mat4 mvpm;

uniform sampler2D inputPositions;


void main()
{
	// read particle position from particle texture
	vec4 p = texture2D(inputPositions, uv).xyzw;
	gl_FragData[0] = vec4(p.x, p.y, p.z, 1.0);
}

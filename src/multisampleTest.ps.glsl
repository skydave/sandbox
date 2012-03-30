varying vec2 uv;

uniform sampler2DMS texture;

void main()
{
	//gl_FragColor = vec4(uv.x, uv.y, 0.0, 1.0);
	gl_FragColor = texelFetch(texture, ivec2(int(uv.x*512.0), int(uv.y*512.0)), 0 );
}

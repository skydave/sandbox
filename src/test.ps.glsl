varying vec2 uv;

void main()
{
	gl_FragData[0] = vec4(uv.x, uv.y, 0.0, 1.0);
}

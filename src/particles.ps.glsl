

void main()
{
	vec2 uv = gl_PointCoord.st;
	gl_FragData[0] = vec4(uv.x, uv.y, 0.0, 1.0);
}

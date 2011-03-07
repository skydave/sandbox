varying vec2 uv;
uniform sampler2D normal_depth;

uniform float focalLength;
uniform float fallOffStart;
uniform float fallOffEnd;

void main(void)
{
	//float focalLength = 10.0;
	//float start = 1.0;
	//float end = 9.0;

	float depth = texture2D( normal_depth, uv ).w;
	float coc = smoothstep( fallOffStart, fallOffEnd, abs(depth - focalLength) );
	gl_FragColor.r = 0.5;
	gl_FragColor.w = coc;
}

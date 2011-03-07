varying vec2 uv;
uniform sampler2D input;
uniform sampler2D coc;

uniform float pixelStep; // 1.0/yres

void main(void)
{
	vec3 sum = vec3(0.0);


	float coc0 = texture2D(coc, vec2(uv.x - 4.0*pixelStep, uv.y)).w + 0.05;
	float coc1 = texture2D(coc, vec2(uv.x - 3.0*pixelStep, uv.y)).w + 0.09;
	float coc2 = texture2D(coc, vec2(uv.x - 2.0*pixelStep, uv.y)).w + 0.12;
	float coc3 = texture2D(coc, vec2(uv.x - pixelStep, uv.y)).w + 0.15;
	float centerCOC = texture2D(coc, vec2(uv.x, uv.y)).w;
	float coc4 = centerCOC + 0.16;
	float coc5 = texture2D(coc, vec2(uv.x + pixelStep, uv.y)).w + 0.15;
	float coc6 = texture2D(coc, vec2(uv.x + 2.0*pixelStep, uv.y)).w + 0.12;
	float coc7 = texture2D(coc, vec2(uv.x + 3.0*pixelStep, uv.y)).w + 0.09;
	float coc8 = texture2D(coc, vec2(uv.x + 4.0*pixelStep, uv.y)).w + 0.05;

	float cocSum = 0.0;
	cocSum += coc0;
	cocSum += coc1;
	cocSum += coc2;
	cocSum += coc3;
	cocSum += coc4;
	cocSum += coc5;
	cocSum += coc6;
	cocSum += coc7;
	cocSum += coc8;

	coc0 /= cocSum;
	coc1 /= cocSum;
	coc2 /= cocSum;
	coc3 /= cocSum;
	coc4 /= cocSum;
	coc5 /= cocSum;
	coc6 /= cocSum;
	coc7 /= cocSum;
	coc8 /= cocSum;


	// blur in y (vertical)
	// take nine samples, with the distance blurSize between them
	sum += texture2D(input, vec2(uv.x - 4.0*pixelStep, uv.y)).xyz * coc0;
	sum += texture2D(input, vec2(uv.x - 3.0*pixelStep, uv.y)).xyz * coc1;
	sum += texture2D(input, vec2(uv.x - 2.0*pixelStep, uv.y)).xyz * coc2;
	sum += texture2D(input, vec2(uv.x - pixelStep, uv.y)).xyz * coc3;
	sum += texture2D(input, vec2(uv.x, uv.y)).xyz * coc4;
	sum += texture2D(input, vec2(uv.x + pixelStep, uv.y)).xyz * coc5;
	sum += texture2D(input, vec2(uv.x + 2.0*pixelStep, uv.y)).xyz * coc6;
	sum += texture2D(input, vec2(uv.x + 3.0*pixelStep, uv.y)).xyz * coc7;
	sum += texture2D(input, vec2(uv.x + 4.0*pixelStep, uv.y)).xyz * coc8;

	gl_FragColor.xyz = sum;
	gl_FragColor.w = centerCOC;
}

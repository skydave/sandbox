// http://en.wikipedia.org/wiki/ASC_CDL
varying vec2 uv;
uniform sampler2D texture;

float slope = 0.1;

void main()
{
	vec4 input = texture2D( texture, uv );
	vec4 output;
	// R
	output.x = input.x*slope;
	// G
	output.y = input.y*slope;
	// B
	output.z = input.z*slope;
	gl_FragData[0] = output;
}

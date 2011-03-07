
varying vec2 uv;
uniform sampler2D input;



void main()
{
	gl_FragData[0] = texture2D(input, uv);
}


varying vec3 cd;

uniform float alpha;

void main()
{
	gl_FragColor = vec4( cd*alpha, 1.0 );
}

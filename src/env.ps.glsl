uniform samplerCube envTex;
uniform mat4 vminv;


varying vec3 n;
varying vec2 uv;
varying vec3 p;




void main()
{
	gl_FragData[0] = textureCube( envTex, n );
}

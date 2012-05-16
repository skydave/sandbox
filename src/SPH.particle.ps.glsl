uniform sampler2D tex;
uniform mat4 vminv;

uniform float alpha;

varying vec3 cd;

void main()
{
	vec2 uv = gl_PointCoord.st;
	//gl_FragData[0] = vec4(c.x, c.y, c.z, texture2D(tex, uv).r*alpha);
	gl_FragData[0] = vec4(cd, 1.0);
}

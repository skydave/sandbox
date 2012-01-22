uniform sampler2D tex;
uniform mat4 vminv;

varying vec3 n;
varying vec3 p;
varying vec2 uv;

uniform float alpha;

vec3 getCameraPos()
{
	// vmatrix transforms from camera to world space
	// vmatrix inverse transforms from world to camera space
	// we transpose it to inverse the inverse
	mat4 t = transpose(vminv);
	return vec3( t[0].w, t[1].w, t[2].w );
}



void main()
{
	//vec3 E = normalize(getCameraPos() - p.xyz);
	//gl_FragData[0] = vec4(uv.x, uv.y, 0.0, 1.0);
	float texAlpha = texture2D(tex, uv).x;
	vec3 c = vec3(1.0, 1.0, 1.0);
	gl_FragData[0] = vec4(c, texAlpha*alpha);
	//gl_FragData[0] = vec4(1.0, 1.0, 1.0, 1.0);
	//gl_FragData[0] = vec4(1.0, 1.0, 1.0, 1.0);
	//gl_FragData[0] = lambert( n, E, vec3(1.0, 0.0,0.0)  );

}

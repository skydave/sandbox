uniform sampler2D tex;
uniform mat4 vminv;

varying vec3 n;
varying vec3 p;
varying vec3 c;

uniform float alpha;

vec3 getCameraPos()
{
	// vmatrix transforms from camera to world space
	// vmatrix inverse transforms from world to camera space
	// we transpose it to inverse the inverse
	mat4 t = transpose(vminv);
	return vec3( t[0].w, t[1].w, t[2].w );
}


vec4 lambert(in vec3 n,in vec3 v,in vec3 l)
{
	vec3 ambient; // ambient color
	vec3 diffuse; // diffuse color
	float ka;
	float kd;
	ambient = vec3( 0.05, 0.05, 0.05 ); ka = 1.0;
	diffuse = vec3( 0.5, 0.5, 0.5 ); kd = 1.0;
	vec3 a = ka*ambient;
	vec3 d = kd*max(dot(n,l),0.0)*diffuse;
	return vec4(a+d,1.0);
}


void main()
{
	vec3 E = normalize(getCameraPos() - p.xyz);
	vec2 uv = gl_PointCoord.st;
	//gl_FragData[0] = vec4(uv.x, uv.y, 0.0, 1.0);
	//gl_FragData[0] = vec4(1.0, 1.0, 1.0, color.r);
	//gl_FragData[0] = vec4(c.x*alpha, c.y*alpha, c.z*alpha, texture2D(tex, uv).r*alpha);
	//gl_FragData[0] = vec4(c.x, c.y, c.z, alpha);
	gl_FragData[0] = vec4(c.x, c.y, c.z, texture2D(tex, uv).r*alpha);
	//gl_FragData[0] = vec4(1.0, 1.0, 1.0, 1.0);
	//gl_FragData[0] = lambert( n, E, vec3(1.0, 0.0,0.0)  );
	
}

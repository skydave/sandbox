uniform sampler2D envTex;
uniform mat4 vminv;

uniform vec3 Li[9];


varying vec3 n;
varying vec2 uv;
varying vec3 p;


vec3 getCameraPos()
{
	// vmatrix transforms from camera to world space
	// vmatrix inverse transforms from world to camera space
	// we transpose it to inverse the inverse
	mat4 t = transpose(vminv);
	return vec3( t[0].w, t[1].w, t[2].w );
}

vec2 cubemapLookup( vec3 n )
{
	vec2 uv;

	vec3 absN = abs(n);

	// nrm is the world normal from a vertex
	// absNrm is the same normal, but with absolute values
	if (absN.x > absN.y && absN.x > absN.z)
	{
		if (n.x>0)
		{
			// right face
			vec3 p_n = vec3(1.0,0.0,0.0);
			float d = dot(p_n, p_n)/dot(n,p_n);
			uv = 1.0 - (d*n*0.5 + 0.5).zy;

			uv = uv*vec2(0.333,0.25) + vec2(0.666,0.25);
		}
		else
		{
			// left face
			vec3 p_n = vec3(-1.0,0.0,0.0);
			float d = dot(p_n, p_n)/dot(n,p_n);
			uv = (d*n*0.5 + 0.5).zy;
			uv.y = 1.0 - uv.y;

			uv = uv*vec2(0.333333,0.25) + vec2(0.0,0.25);
		}
	} else
	if (absN.y > absN.x && absN.y > absN.z)
	{
		if (n.y>0)
		{
			// top face
			vec3 p_n = vec3(0.0,1.0,0.0);
			float d = dot(p_n, p_n)/dot(n,p_n);
			uv = (d*n*0.5 + 0.5).xz;

			uv = uv*vec2(0.333,0.25) + vec2(0.333,0.0);
		}
		else
		{
			// bottom face
			vec3 p_n = vec3(0.0,-1.0,0.0);
			float d = dot(p_n, p_n)/dot(n,p_n);
			uv = (d*n*0.5 + 0.5).xz;
			uv.y = 1.0 - uv.y;

			uv = uv*vec2(0.333,0.25) + vec2(0.333,0.5);
		}
	}else
	{
		if (n.z>0)
		{
			// front face
			vec3 p_n = vec3(0.0,0.0,1.0);
			float d = dot(p_n, p_n)/dot(n,p_n);
			uv = (d*n*0.5 + 0.5).xy;
			uv.y = 1.0 - uv.y;

			uv = uv*vec2(0.333,0.25) + vec2(0.333,0.25);
		}
		else
		{
			// back face
			vec3 p_n = vec3(0.0,0.0,-1.0);
			float d = dot(p_n, p_n)/dot(n,p_n);
			uv = (d*n*0.5 + 0.5).xy;

			uv = uv*vec2(0.333333,0.25) + vec2(0.333333,0.75);			
		}
	}


	return uv;
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


vec3 SH_unprojectV3( vec3 n )
{
	float c0 = 0.282095;
	float c1 = 0.488603;
	float c2 = 0.315392;
	float c3 = 0.546274;
	float c4 = 1.092548;

	vec3 result;
	result.x = Li[0].x*c0 + Li[1].x*c1*n.x + Li[2].x*c1*n.z + Li[3].x*c1*n.y + Li[4].x*c4*n.x*n.z + Li[5].x*c4*n.y*n.z + Li[6].x*c2*(3.0*n.z*n.z - 1.0) + Li[7].x*c4*n.x*n.y + Li[8].x*c3*(n.x*n.x - n.y*n.y);
	result.y = Li[0].y*c0 + Li[1].y*c1*n.x + Li[2].y*c1*n.z + Li[3].y*c1*n.y + Li[4].y*c4*n.x*n.z + Li[5].y*c4*n.y*n.z + Li[6].y*c2*(3.0*n.z*n.z - 1.0) + Li[7].y*c4*n.x*n.y + Li[8].y*c3*(n.x*n.x - n.y*n.y);
	result.z = Li[0].z*c0 + Li[1].z*c1*n.x + Li[2].z*c1*n.z + Li[3].z*c1*n.y + Li[4].z*c4*n.x*n.z + Li[5].z*c4*n.y*n.z + Li[6].z*c2*(3.0*n.z*n.z - 1.0) + Li[7].z*c4*n.x*n.y + Li[8].z*c3*(n.x*n.x - n.y*n.y);
	return result;
}

float SH_unproject( vec3 n )
{
	float c0 = 0.282095;
	float c1 = 0.488603;
	float c2 = 0.315392;
	float c3 = 0.546274;
	float c4 = 1.092548;

	return Li[0].x*c0 + Li[1].x*c1*n.x + Li[2].x*c1*n.z + Li[3].x*c1*n.y + Li[4].x*c4*n.x*n.z + Li[5].x*c4*n.y*n.z + Li[6].x*c2*(3.0*n.z*n.z - 1.0) + Li[7].x*c4*n.x*n.y + Li[8].x*c3*(n.x*n.x - n.y*n.y);
}

void main()
{
	//float val = SH_unproject( n );
	//gl_FragData[0] = vec4(val, val, val, 1.0);
	vec3 val = SH_unprojectV3(n);
	gl_FragData[0] = vec4(val, 1.0);
	//gl_FragData[0] = texture2D( envTex, cubemapLookup( n ) );
}

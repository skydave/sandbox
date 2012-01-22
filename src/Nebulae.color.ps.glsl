varying vec2 uv;
uniform sampler2D inputPositions;

vec3 light( vec3 pos, vec3 color, vec3 p )
{
	float distance = length(p - pos)*25.0;
	float attenuation = 1.0/(distance*distance);
	return color*attenuation;
}

vec3 light2( vec3 pos, vec3 color, float radius, vec3 p )
{
	float distance = clamp(length(p - pos), 0.0, radius);
	float attenuation = 1.0 - (distance/radius)*(distance/radius);
	return color*attenuation;
}

void main()
{
	// read particle position from particle texture
	vec3 p = texture2D(inputPositions, uv).xyz;

	//compute color

	// light0
	//vec3 light0 = light( vec3(-0.3, 0.0, 0.0), vec3(.9, 0.7, 0.2), p );
	vec3 light0 = light2( vec3(-0.3, 0.0, 0.0), vec3(.9, 0.7, 0.2), 0.4, p );
	//vec3 light0 = vec3(0.0);

	// light1
	//vec3 light1 = light( vec3(0.3, 0.0, 0.0), vec3(.1, 0.5, 0.9), p );
	vec3 light1 = light2( vec3(0.3, 0.0, 0.0), vec3(.1, 0.5, 0.9), 0.4, p );
	//vec3 light1 = vec3(0.0);

	vec3 color = light0 + light1;

	gl_FragData[0] = vec4(color, 1.0);
}

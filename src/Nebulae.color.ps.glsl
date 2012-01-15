varying vec2 uv;
uniform sampler2D inputPositions;

void main()
{
	// read particle position from particle texture
	vec3 p = texture2D(inputPositions, uv).xyz;

	//compute color

	vec3 lightPos = vec3(0.0, 0.0, 0.0);

	float distance = length(p - lightPos)*5;
	float attenuation = 1.0/(distance*distance);

	//vec3 color = vec3(1.0, 0.0, 0.0)*exp(-distance);
	//vec3 color = vec3(exp(-distance*3.0));
	vec3 color = vec3(.9, 0.7, 0.2)*attenuation;
	//vec3 color = vec3(1.0, 1.0, 1.0)*attenuation;
	//vec3 color = vec3(p.x, p.x, p.x);

	gl_FragData[0] = vec4(color, 1.0);
}

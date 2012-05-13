
float PI = 3.14159265358979323846264; 

uniform sampler2D texture;

varying vec3 n;



vec2 cylindrical( float p, float l )
{

	float x = (l+PI)/(2.0*PI);
	float y = (p+ PI*0.5)/PI;

	return vec2(x, y);
}


void main()
{
	vec3 nn = normalize(n); 

	// cartesian to lat long
	float lat = asin( nn.y );
	float long = atan2( nn.z, nn.x );

	vec2 uvt = cylindrical(lat, long);
	vec3 t = texture2D( texture, uvt );

	gl_FragData[0] = vec4(t*0.3, 1.0);
}

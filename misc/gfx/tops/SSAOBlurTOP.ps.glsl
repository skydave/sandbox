//
// super simple gaussian blur (convolution)
//
// todo: performance improvement: split into 2x 1d blur for faster computation
//		 flexibility: compute filter kernel in app and have radius as shader parameter
//		 ssao: check for normals to avoid bleeding of seperate objects into each other
//

#define RAD 2 // Filter kernel radius
#define N  49 // Number of elements in the filter kernel

varying vec2 uv;
uniform sampler2D input;


//float kernel[9] = float[]( 1.0, 2.0, 1.0, 2.0, 4.0, 2.0, 1.0, 2.0, 1.0 );
float kernel[25] = float[]( 1, 4, 7, 4, 1, 4, 16, 26, 16, 4, 7, 26, 41, 26, 7, 4, 16, 26, 16, 4, 1, 4, 7, 4, 1 );





void main()
{
	vec4 result = vec4(0.0);
	
	int i, j, k;
	k = 0;
	for (i = -RAD; i <= RAD; i++)
		for (j = -RAD; j <= RAD; j++)
		{
			//result += texture2D(input, uv + vec2(i, j)*(1.0/512.0)) * (kernel[k]/16.0);
			result += texture2D(input, uv + vec2(i, j)*(1.0/512.0)) * (kernel[k]/273.0);
			k++;
		}
		
	gl_FragData[0] = result;
}

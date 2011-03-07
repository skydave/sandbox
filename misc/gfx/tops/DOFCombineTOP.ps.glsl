varying vec2 uv;
uniform sampler2D original;
uniform sampler2D normal_depth;
uniform sampler2D blurMed;
uniform sampler2D blurLarge_coc;
uniform sampler2D blurredCoC;
uniform sampler2D blurredDepth;

uniform vec2 pixelStep; // 1.0/xres 1.0/yres

uniform float focalLength;
uniform float fallOffStart;
uniform float fallOffEnd;

vec3 getSmallBlur( sampler2D tex, vec2 uvCoord )
{
	vec3 sum;
	float weight = 4.0 / 17.0;

	sum	= vec3(0.0);

	sum += weight * texture2D(tex, uvCoord+vec2(pixelStep.x, 0.0)).xyz;
	sum += weight * texture2D(tex, uvCoord+vec2(0.0, pixelStep.y)).xyz;
	sum += weight * texture2D(tex, uvCoord+vec2(-pixelStep.x, 0.0)).xyz;
	sum += weight * texture2D(tex, uvCoord+vec2(0.0, -pixelStep.y)).xyz;

	return sum;
}

float computeCoC( float depth )
{
	//float focalLength = 10.0;
	//float start = 1.0;
	//float end = 9.0;

	return smoothstep( fallOffStart, fallOffEnd, abs(depth - focalLength) );
}


void main(void)
{
	//gl_FragColor = texture2D(original, uv);
	//gl_FragColor = vec4(texture2D(normal_depth, uv).w);
	//vec3 blurSmall = getSmallBlur( original, uv );
	//gl_FragColor = vec4( blurSmall, 1.0 );
	//gl_FragColor = texture2D(blurMed, uv);
	//gl_FragColor = texture2D(blurLarge_coc, uv);
	//gl_FragColor = vec4(texture2D(blurLarge_coc, uv).w);
	//gl_FragColor = vec4(texture2D(blurredCoC, uv).w);
	//gl_FragColor = vec4(texture2D(blurredDepth, uv).w);

	vec3 org = texture2D(original, uv).xzy;
	float depth = texture2D(normal_depth, uv).w;
	vec3 blurSmall = getSmallBlur( original, uv );
	vec3 blurMed = texture2D(blurMed, uv).xyz;
	vec3 blurLarge = texture2D(blurLarge_coc, uv).xyz;
	float blurredDepth = texture2D(blurredDepth, uv).w;



	// if object is sharp but downscaled depth is behind, then stay sharp
	// means: if average depth of surrounding pixels is behind current pixel then the pixels tend to be behind and have no influence
	// otherwise they are on top and therefore have influence on pixels behind
	float coc = 0.0;
	if( blurredDepth > depth )
		coc = computeCoC(depth);
	else
		coc = texture2D(blurredCoC, uv).w;


	//float d0 = 0.5;
	//float d1 = 0.25;
	//float d2 = 0.25;
	float d0 = 0.5;
	float d1 = 0.25;
	float d2 = 0.25;
	vec4 weights = clamp( coc * vec4(-1.0/d0, -1.0/d1, -1.0/d2, 1.0/d2) + vec4(1.0, (1.0-d2)/d1, 1.0/d2, (d2-1.0)/d2 ), 0.0, 1.0 );
	weights.yz = min(weights.yz, 1.0 - weights.xy );



	// blend together all the different blurs depending on their weights
	vec3 outColor = weights.y * blurSmall + weights.z * blurMed + weights.w * blurLarge;
	// compute alpha channel?
	//float outAlpha = dot(weights.yzw, vec3(	16.0/17.0, 1.0, 1.0 ));
	//gl_FragColor = vec4(outColor, outAlpha );
	//gl_FragColor = vec4(outColor, 1.0 );
	//gl_FragColor = vec4( outAlpha );

	//gl_FragColor = vec4(mix( outColor, org, 1.0 - weights.y - weights.z - weights.w ), 1.0);
	//gl_FragColor = vec4( org + outColor*outAlpha, 1.0);
	//gl_FragColor = vec4( org, 1.0);
	//gl_FragColor = vec4( outColor, 1.0);
	//gl_FragColor = vec4( outColor, 1.0);
	//gl_FragColor = vec4(mix( outColor, org, 1.0 - weights.y - weights.z - weights.w ), 1.0);
	//gl_FragColor = vec4( weights.y );
	//gl_FragColor = vec4( weights.y * blurSmall, 1.0 );
	//gl_FragColor = vec4( weights.z * blurMed, 1.0 );
	//gl_FragColor = vec4( weights.y, weights.z, weights.w, 1.0 );
	//gl_FragColor = vec4(vec3( weights.w), 1.0 );
	gl_FragColor = vec4(  org*(1.0 - weights.y - weights.z - weights.w) + weights.y * blurSmall + weights.z * blurMed + weights.w * blurLarge, 1.0 );


}

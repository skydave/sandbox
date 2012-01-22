attribute vec3 P;
attribute vec3 N;
attribute vec3 offset;
attribute float index; // index of the tile with the 4x4 cloud texture
attribute float scale; // size per billboard

uniform mat4 mvpm;
uniform mat3 mvminvt;
uniform mat4 vm;

varying vec3 n;
varying vec2 uv;


void main()
{
	n = N;

	// use position straight
	vec3 p = P;


	//screen-aligned axes
	vec3 axis1 = vec3(  vm[0][0],
						vm[1][0],
						vm[2][0]);

	vec3 axis2 = vec3(  vm[0][1],
						vm[1][1],
						vm[2][1]);

	vec3 corner = p + (offset.x*axis1 + offset.y*axis2)*scale;

	// computing uv coords is a bit more involved since we will use the index
	// to select a subsectino of the texture
	uv = vec2( 0.5, 0.5 ) + offset.xy;

	int col = mod(index, 4);
	int row = index / 4;
	uv.x = (uv.x * 0.25) + col * 0.25;
	uv.y = (uv.y * 0.25) + row * 0.25;

	gl_Position = mvpm * vec4(corner, 1.0);
}

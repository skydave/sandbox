uniform mat4 vminv;


attribute vec3 P;
attribute vec4 Cd;
attribute vec3 BI;
attribute vec2 UV;

varying vec2 uv;
varying vec3 ep; // eye position (on image plane) in worldspace
varying vec3 ed; // eye direction in worldspace

void main()
{
	vec3 r = P*vec3(1.3333,1.0,0.0)*0.5+vec3(0.0,0.0,-1.0);
	mat4 t = transpose(vminv); // we transpose so that we can use .xyz to easily access the vectors of the coordinate system defined by the matrix
	ed.x=dot(r,t[0].xyz);
	ed.y=dot(r,t[1].xyz);
	ed.z=dot(r,t[2].xyz);

	ep.x = t[0].w;
	ep.y = t[1].w;
	ep.z = t[2].w;


	gl_Position = gl_ModelViewProjectionMatrix * vec4(P,1.0);
	uv = UV;
}

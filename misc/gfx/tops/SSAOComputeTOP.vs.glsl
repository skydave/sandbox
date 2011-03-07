attribute vec3 P;
attribute vec4 Cd;
attribute vec3 BI;
attribute vec2 UV;

varying vec2 uv;
varying vec3 ev; // eye vector in viewspace

void main()
{
	ev = P*vec3(1.3333,1.0,0.0)+vec3(0.0,0.0,-1.0);
	//mat4 t = transpose(vminv);
    //ed.x=dot(r,t[0].xyz);
    //ed.y=dot(r,t[1].xyz);
    //ed.z=dot(r,t[2].xyz);


	gl_Position = vec4(P,1.0);
	uv = UV;
}

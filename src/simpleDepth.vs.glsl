
attribute vec3 P;

varying float depth; // depth in eye space
varying vec3 pw; // position in world space

uniform mat4 mvpm;
uniform mat4 mvm;

void main()
{
	pw = P;
	depth = (mvm * vec4(P,1.0)).z;
	//depth = P.z;
	gl_Position = mvpm * vec4(P,1.0);

}

// data source module for Volume which retrieves data from a 3dtexture

#define sort2(a,b) { vec3 tmp=min(a,b); b=a+b-tmp; a=tmp; }

uniform sampler3D density;


float queryDensity( vec3 vsPos )
{
	return texture3D( density, vsPos ).a;
}

bool intersectData( in vec3 rayPos, in vec3 rayDir, inout float t0, inout float t1 )
{
	vec3 bbMin = vec3(0.0, 0.0, 0.0);
	vec3 bbMax = vec3( 1.0, 1.0, 1.0 );
	vec3 dL = vec3(1.0/rayDir.x, 1.0/rayDir.y, 1.0/rayDir.z);
	vec3 lo = (bbMin - rayPos) * dL;
	vec3 hi = (bbMax - rayPos) * dL;
	sort2(lo, hi);
	bool hit = !( lo.x>hi.y || lo.y>hi.x || lo.x>hi.z || lo.z>hi.x || lo.y>hi.z || lo.z>hi.y );
	t0 = max(max(lo.x, lo.y), lo.z);
	t1 = min(min(hi.x, hi.y), hi.z);
	return hit;
}
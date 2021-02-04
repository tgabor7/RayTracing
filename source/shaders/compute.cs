#version 430

#define inf 1.0 / 0.0
#define EPSILON 1e-8
#define PI 3.14159265

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img_output;
layout(std430, binding = 1) buffer modelLayout {
	float _vertices[];
};
struct Sphere
{
	float px;
	float py;
	float pz;
	
	float colorx;
	float colory;
	float colorz;
	
	float emissionx;
	float emissiony;
	float emissionz;
	
	float specularx;
	float speculary;
	float specularz;
	
	float smoothness;
	float radius;
};
layout(shared, binding = 2) buffer spherebuffer {
	Sphere _spheres[];
};
uniform float size;
uniform mat4 inverseProjection;
uniform mat4 invview;
uniform int width;
uniform int height;
uniform vec4 light = vec4(.1,1,.5,1);
uniform float seed;
uniform sampler2D floor_texture;
uniform sampler2D skybox_texture;
uniform mat4 transformation;
uniform vec2 randpx;
uniform int number_of_spheres;

struct Box {
	vec3 min;
	vec3 max;
};
struct Ray
{
    vec3 origin;
    vec3 energy;
    vec3 direction;
};
struct RayHit
{
    vec3 position;
    vec3 normal;
    vec3 albedo;
    vec3 specular;
	float transparency;
    float distance;
	float smoothness;
	vec3 emission;
};


float rseed = dot(gl_GlobalInvocationID.xy, vec2(12.9898f, 78.233f));
float rand()
{
    float result = fract(sin(seed / 100.0f * rseed) * 43758.5453f);
	rseed++;
    return result;
}
float sdot(vec3 x, vec3 y, float f = 1.0f)
{
    return clamp(dot(x, y) * f,0.0,1.0);
}
mat3 getTangentSpace(vec3 normal)
{
	vec3 helper = vec3(1, 0, 0);
    if (abs(normal.x) > 0.99f)
        helper = vec3(0, 0, 1);
    // Generate vectors
    vec3 tangent = normalize(cross(normal, helper));
    vec3 binormal = normalize(cross(normal, tangent));
    return mat3(tangent, binormal, normal);
}
vec2 s = vec2(gl_GlobalInvocationID.xy)*(float(seed)+1.0);
vec2 rand2n() {
    s+=vec2(-1,1);
	return vec2(fract(sin(dot(s.xy ,vec2(12.9898,78.233))) * 43758.5453),
		fract(cos(dot(s.xy ,vec2(4.898,7.23))) * 23421.631));
};
vec3 ortho(vec3 v) {
    return abs(v.x) > abs(v.z) ? vec3(-v.y, v.x, 0.0)  : vec3(0.0, -v.z, v.y);
}
vec3 sampleHemisphere(vec3 dir, float alpha)
{
	dir = normalize(dir);
	vec3 o1 = normalize(ortho(dir));
	vec3 o2 = normalize(cross(dir, o1));
	vec2 r = rand2n();
	r.x=r.x*2.*PI;
	r.y=pow(r.y,1.0/(alpha+1.0));
	float oneminus = sqrt(1.0-r.y*r.y);
	return cos(r.x)*oneminus*o1+sin(r.x)*oneminus*o2+r.y*dir;
}
RayHit createRayHit()
{
    RayHit ray;
    ray.distance = inf;
    ray.albedo = vec3(0.0);
    ray.normal = vec3(0.0);
    ray.position = vec3(0.0);

    return ray;
}

Ray cameraRay(vec2 uv)
{
    Ray r;
    vec3 origin = (invview * vec4(0.0, 0.0, 0.0, 1.0)).xyz;

    vec3 direction = (inverseProjection * vec4(uv, 0.0, 1.0)).xyz;

    direction = (invview * vec4(direction, 0.0)).xyz;
    direction = normalize(direction);

    r.direction = direction;
    r.origin = origin;
    r.energy = vec3(1, 1, 1);

    return r;
}
bool intersectBox(Box b, Ray ray)
{
	float t[10];
	t[1] = (b.min.x - ray.origin.x)/ray.direction.x;
	t[2] = (b.max.x - ray.origin.x)/ray.direction.x;
	t[3] = (b.min.y - ray.origin.y)/ray.direction.y;
	t[4] = (b.max.y - ray.origin.y)/ray.direction.y;
	t[5] = (b.min.z - ray.origin.z)/ray.direction.z;
	t[6] = (b.max.z - ray.origin.z)/ray.direction.z;
	t[7] = max(max(min(t[1], t[2]), min(t[3], t[4])), min(t[5], t[6]));
	t[8] = min(min(max(t[1], t[2]), max(t[3], t[4])), max(t[5], t[6]));
	t[9] = (t[8] < 0 || t[7] > t[8]) ? 0.0 : t[7];
	return bool(t[9]);
}
void intersectGround(Ray ray, inout RayHit bestHit)
{
    float t = -ray.origin.y / ray.direction.y;
    if (t > 0 && t < bestHit.distance)
    {
        bestHit.distance = t;
        bestHit.position = ray.origin + t * ray.direction;
        vec4 floor_color = texture2D(floor_texture,.5 * vec2(bestHit.position.x,bestHit.position.z)*0.5+0.5);
		bestHit.albedo = floor_color.xyz;
		//bestHit.albedo = vec3(.8);
		bestHit.normal = vec3(0.0, 1.0, 0.0);
        bestHit.smoothness = 0.2;
        bestHit.emission = vec3(0.0);
		bestHit.specular = vec3(0.03);
    }
}
float smoothnessToPhongAlpha(float s)
{
    return pow(1000.0f, s * s);
}
void intersectSphere(Sphere sphere, inout RayHit bestHit, inout Ray ray)
{
    vec3 d = ray.origin - vec3(sphere.px,sphere.py,sphere.pz);
    float p1 = -dot(ray.direction, d);
    float p2sqr = p1 * p1 - dot(d, d) + sphere.radius * sphere.radius;
    if (p2sqr < 0)
    {
        return;
    }
	
    float p2 = sqrt(p2sqr);
    float t = p1 - p2 > 0 ? p1 - p2 : p1 + p2;
	
    if (t > 0 && t < bestHit.distance)
    {
		bestHit.distance = t;
		bestHit.position = ray.origin + t * ray.direction;
		bestHit.normal = normalize(bestHit.position - vec3(sphere.px,sphere.py,sphere.pz));
		bestHit.albedo.x = sphere.colorx;
		bestHit.albedo.y = sphere.colory;
		bestHit.albedo.z = sphere.colorz;
		
		bestHit.specular.x = sphere.specularx;
		bestHit.specular.y = sphere.speculary;
		bestHit.specular.z = sphere.specularz;
		bestHit.emission.x = sphere.emissionx;
		bestHit.emission.y = sphere.emissiony;
		bestHit.emission.z = sphere.emissionz;
		bestHit.smoothness = sphere.smoothness;
		//bestHit.color = sphere.color;
		
    }
}
bool triangleHit(Ray ray, vec3 v0, vec3 v1, vec3 v2, inout float t, inout float u, inout float v)
{
	vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 pvec = cross(ray.direction, edge2);
    float det = dot(edge1, pvec);
    if (det < EPSILON)
        return false;
    float inv_det = 1.0f / det;
    vec3 tvec = ray.origin - v0;
    u = dot(tvec, pvec) * inv_det;
    if (u < 0.0 || u > 1.0f)
        return false;
    vec3 qvec = cross(tvec, edge1);
    v = dot(ray.direction, qvec) * inv_det;
    if (v < 0.0 || u + v > 1.0f)
        return false;
    t = dot(edge2, qvec) * inv_det;

	return true;
}

void intersectTriangle(Ray ray, inout RayHit bestHit, vec3 v0, vec3 v1, vec3 v2)
{
	float t, u, v;
	if (triangleHit(ray, v0, v1, v2, t, u, v))
	{
		if (t > 0 && t < bestHit.distance)
		{
			bestHit.distance = t;
			bestHit.position = ray.origin + t * ray.direction;
			bestHit.normal = normalize(cross(v1 - v0, v2 - v0));
			bestHit.albedo = vec3(1);
			bestHit.specular = vec3(1);
			bestHit.emission = vec3(.2);
			bestHit.smoothness = 2;
		}
	}
}
void traceModel(Ray ray, inout RayHit bestHit)
{
	for(int i = 0;i<size;i+=9){
		vec3 v0, v1, v2;
		v0.x = _vertices[i];
		v0.y = _vertices[i+1];
		v0.z = _vertices[i+2];
		v1.x = _vertices[i+3];
		v1.y = _vertices[i+4];
		v1.z = _vertices[i+5];
		v2.x = _vertices[i+6];
		v2.y = _vertices[i+7];
		v2.z = _vertices[i+8];
		
		v0 = (transformation * vec4(v0,1.0)).xyz;
		v1 = (transformation * vec4(v1,1.0)).xyz;
		v2 = (transformation * vec4(v2,1.0)).xyz;
		
		intersectTriangle(ray, bestHit, v0,v1,v2);
		
	}
}
RayHit trace(Ray ray)
{
    RayHit hit = createRayHit(); 
    Sphere s;
    intersectGround(ray, hit);
	Box b;
	b.min = vec3(-2,0,-2);
	b.max = vec3(2,4,2);
	//if(intersectBox(b,ray)){
		traceModel(ray, hit);
	//}
	
	for(int i = 0;i<number_of_spheres;i++){
		intersectSphere(_spheres[i], hit, ray);
	}
    return hit;
}
float energy(vec3 color)
{
    return dot(color, vec3(1.0f / 3.0f));
}
vec3 shade(inout Ray ray, RayHit hit)
{
    if (hit.distance < inf)
    {
        hit.albedo = min(1.0f - hit.specular, hit.albedo);
        float specChance = energy(hit.specular);
        float diffChance = energy(hit.albedo);
		// Roulette-select the ray's path
        float roulette = rand();
		
		Ray shadowRay;
        shadowRay.origin = hit.position + hit.normal * 0.0001;
        shadowRay.direction = normalize(light.xyz);
        RayHit shadowHit = trace(shadowRay);
        if (shadowHit.distance < inf)
        {
			ray.energy *= .8;
        }
		if (roulette < specChance)
        {
            // Specular reflection
            ray.origin = hit.position + hit.normal * 0.001f;
            float alpha = smoothnessToPhongAlpha(hit.smoothness);
            ray.direction = sampleHemisphere(reflect(ray.direction, hit.normal), alpha);
            float f = (alpha + 2) / (alpha + 1);
            ray.energy *= (1.0f / specChance) * hit.specular * sdot(hit.normal, ray.direction, f);
        }
		else if (diffChance > 0 && roulette < specChance + diffChance)
        {
            // Diffuse reflection
            ray.origin = hit.position + hit.normal * 0.001f;
            ray.direction = sampleHemisphere(hit.normal, 1.0f);
            ray.energy *= (1.0f / diffChance) * hit.albedo;
        }
		else
        {
            // Terminate ray
            ray.energy = vec3(0);
        }
		
        return hit.emission;
    }
    else
    {
        ray.energy = vec3(0);

		float theta = acos(ray.direction.y) / -PI;
        float phi = atan(-ray.direction.z,ray.direction.x) / -PI * 0.5f;
		
		
        return vec3(texture2D(skybox_texture, vec2(phi, theta)).xyz);
    }

}

bool isnull(vec3 v)
{
    return v.x == 0 && v.y == 0 && v.z == 0;
}
void main()
{
	float res = (float(width) / float(gl_NumWorkGroups.x));
    vec2 storePos = vec2(((vec2(gl_GlobalInvocationID.xy) + vec2(randpx)) * res));
    vec2 uv = vec2((vec2(storePos)) / vec2(width, height) * 2.0 - vec2(1.0));
    Ray ray = cameraRay(uv);
	
	
    vec3 color = vec3(0, 0, 0);
    for (int i = 0; i < 8; i++)
    {
		RayHit hit = trace(ray);
        vec3 energy = ray.energy;
        vec3 sha = shade(ray, hit);
		
        color += energy * sha;
		if (isnull(energy))
        {
            break;
        }
    }
	
	
	imageStore(img_output, ivec2(storePos), vec4(color, 1.0));
}
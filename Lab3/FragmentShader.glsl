#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentColor;
in vec3 fragmentNormal;

// Ouput data
out vec3 color;

struct DirectionalLight
{
	vec3 direction;

	// contains color and intensity
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
struct PointLight
{
	vec3 position;
	
	// coefficient for falloff function
	// float c0;	// constant coefficient
	// float c1;	// linear coefficient
	// float c2;	// quadratic coefficient
	// float c3;	// cubic coefficient
    vec3 coefficient;

	// contains color and intensity
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
struct SpotLight
{
	vec3 position;
	vec3 direction;

	// cosine of corn angle
	float radius_inner;
	float radius_outer;

	// contains color and intensity
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

// uniform vec3 uLight;
uniform DirectionalLight dLight;
uniform PointLight pLight;
uniform SpotLight slight;

vec3 MonoColor(vec3);
vec3 DLightColor(vec3);
vec3 PLightColor(PointLight light, vec3 normal, vec3 fpos);
vec3 SLightColor(vec3);

void main(){
	
	// color = MonoColor(vec3(1.0,1.0,1.0));
	//TODO: Assign fragmentColor as a final fragment color
	// color = fragmentColor;
	//TODO:Assign fragmentNormal as a final fragment color
	// vec3 normal = normalize(fragmentNormal);
    // color = normal;
    // TODO: Phong reflection model

    // Phong chage
    // vec3 tolight = normalize(uLight - fragmentPosition);
    // vec3 toV = -normalize(vec3(fragmentPosition));
    // vec3 h = normalize(toV + tolight);
    // vec3 normal = normalize(fragmentNormal);
    // float specular = pow(max(0.0, dot(h, normal)), 64.0);
    // float diffuse = max(0.0, dot(normal, tolight));
    // vec3 intensity = fragmentColor *diffuse + vec3(0.6, 0.6, 0.6)*specular;
    // color = floor(pow(intensity, vec3(1.0 / 2.2))*3)/3.0f;

    color = PLightColor(pLight, fragmentNormal, fragmentPosition);

    // Toon Shading
    // vec3 normal = normalize(fragmentNormal);
    // color = floor(dot(normal,normalize(vec3(1.0,2.0,2.0)))*3)/3.0*vec3(1.0,0.6,0.7); // Apply gamma correction 

    // color = floor(color*3)/3.0;

    // Gouraud shading
    // color = fragmentColor;

}

vec3 MonoColor(vec3 c)
{
    return c;
}

vec3 PLightColor(PointLight light, vec3 normal, vec3 fpos)
{
    vec3 tolight = normalize(light.position - fpos);
    vec3 toV = -normalize(vec3(fpos));
    vec3 h = normalize(toV + tolight);
    vec3 _normal = normalize(normal);
    
    float spec = pow(max(0.0, dot(h, _normal)), 64.0); // material shiness = 64.0
    float diff = max(0.0, dot(_normal, tolight));

    float d = length(light.position - fpos);
    float attenuation = 1.0f/dot(light.coefficient,vec3(1.0f,d,d*d)); // 1/(c0 + c1*d + c2*d^2)
    vec3 ambient = light.ambient *  vec3(0.0, 0.0, 0.0); // material ambient
    vec3 diffuse = light.diffuse *  diff * fragmentColor; // material diffuse
    vec3 specular = light.specular *  spec * vec3(0.6, 0.6, 0.6); // material specular

    vec3 intensity = attenuation * (ambient + diffuse + specular);
    return pow(intensity, vec3(1.0 / 2.2)); // Apply gamma correction 
 }
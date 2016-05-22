#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentColor;
in vec3 fragmentNormal;
flat in vec3 fragmentFlatPosition;
flat in vec3 fragmentFlatColor;
flat in vec3 fragmentFlatNormal;

// Ouput data
out vec3 color;

// contains color and intensity
struct Illumination
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;    
};
struct DirectionalLight
{
    vec3 direction;

    Illumination illumination;
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

	Illumination illumination;
};
struct SpotLight
{
	vec3 position;
	vec3 direction;

	// cosine of corn angle
	float radius_inner;
	float radius_outer;

    Illumination illumination;
};
struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shiness;
};

uniform DirectionalLight dLight;
uniform PointLight pLight;
uniform SpotLight sLight;
uniform Material material;
uniform int shaderType;

vec3 MonoColor(vec3);
vec3 DLightColor(DirectionalLight light, vec3 normal, vec3 fpos);
vec3 PLightColor(PointLight light, vec3 normal, vec3 fpos);
vec3 SLightColor(SpotLight light, vec3 normal, vec3 fpos);
vec3 BlinnPhong(vec3 l, vec3 p, vec3 n, Illumination i, Material m);

void main(){

    switch(shaderType)
    {
        case 0:
            color = DLightColor(dLight, fragmentNormal, fragmentPosition)
                  + PLightColor(pLight, fragmentNormal, fragmentPosition)
                  + SLightColor(sLight, fragmentNormal, fragmentPosition);
            break;
        case 1:
            color = DLightColor(dLight, fragmentFlatNormal, fragmentFlatPosition)
                  + PLightColor(pLight, fragmentFlatNormal, fragmentFlatPosition)
                  + SLightColor(sLight, fragmentFlatNormal, fragmentFlatPosition);
            break;
        case 2:
            color = DLightColor(dLight, fragmentNormal, fragmentPosition)
                  + PLightColor(pLight, fragmentNormal, fragmentPosition)
                  + SLightColor(sLight, fragmentNormal, fragmentPosition);
            color = floor(color*3)/3.0;

            vec3 normal = normalize(fragmentNormal);
            color = floor(dot(normal,normalize(vec3(1.0,2.0,2.0)))*3)/3.0*vec3(1.0,0.6,0.7); // Apply gamma correction
            break;
        default:
            color = vec3(0.4,0.5,0.6);
            break;

    }
    // Toon Shading
    // vec3 normal = normalize(fragmentNormal);
    // color = floor(dot(normal,normalize(vec3(1.0,2.0,2.0)))*3)/3.0*vec3(1.0,0.6,0.7); // Apply gamma correction 

    // color = floor(color*3)/3.0;
}

vec3 MonoColor(vec3 c)
{
    return c;
}

vec3 BlinnPhong(vec3 l, vec3 p, vec3 n, Illumination i, Material m)
{
    vec3 tolight = normalize(-l);
    vec3 toV = -normalize(vec3(p));
    vec3 h = normalize(toV + tolight);
    vec3 normal = normalize(n);

    float diff = max(0.0, dot(normal, tolight));
    float spec = pow(max(0.0, dot(h, normal)), m.shiness); // material shiness = 64.0

    vec3 ambient = i.ambient * m.ambient; // material ambient
    vec3 diffuse = i.diffuse * m.diffuse * diff; // material diffuse
    vec3 specular = i.specular * m.specular * spec; // material specular
    return (ambient + diffuse + specular);
}

vec3 DLightColor(DirectionalLight light, vec3 normal, vec3 fpos)
{
    vec3 intensity = BlinnPhong(light.direction, fpos, normal, light.illumination, material);
    return pow(intensity, vec3(1.0 / 2.2)); // Apply gamma correction 
 }
vec3 PLightColor(PointLight light, vec3 normal, vec3 fpos)
{
    float d = length(light.position - fpos);
    float attenuation = 1.0f/dot(light.coefficient,vec3(1.0f,d,d*d));

    vec3 intensity = attenuation * BlinnPhong(fpos - light.position, fpos, normal, light.illumination, material);
    return pow(intensity, vec3(1.0 / 2.2)); // Apply gamma correction 
 }
vec3 SLightColor(SpotLight light, vec3 normal, vec3 fpos)
{
    float theta = dot(normalize(-light.direction), normalize(-light.direction)); 
    float epsilon = light.radius_inner - light.radius_outer;
    float falloff = clamp((theta - light.radius_outer) / epsilon, 0.0, 1.0);

    vec3 intensity = falloff * BlinnPhong(light.direction, fpos, normal, light.illumination, material);
    return pow(intensity, vec3(1.0 / 2.2)); // Apply gamma correction 
}
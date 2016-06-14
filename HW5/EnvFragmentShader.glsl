#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentNormal;
in vec2 UV;

smooth in vec3 ReflectDir;
smooth in vec3 RefractDir;
in float Reflectivity;
// smooth in vec3 UVy;

// Ouput data
out vec3 color;

uniform vec3 uLight;
uniform bool DrawSkyBox;

uniform samplerCube cubemap;
uniform sampler2D myTextureSampler;

void main(){
	vec3 normal = normalize(fragmentNormal);
    // float thickness = 0.1;

	// if (DrawSkyBox){
	// 	color = vec3(1.0, 1.0, 1.0);
	// }
	// else{
	// 	color = vec3(0.0, 0.7, 0.7);
	// }
	//TODO: assign color from environmental map(cubemap) texture
	vec4 texColor = texture(cubemap, normalize(ReflectDir));
	if(DrawSkyBox){
	   color = texColor.xyz;
	}else{
        float cosine = dot(normal, -normalize(fragmentPosition));//cos theta = dot (N V)
        //Schlick's approximation
        float rtheta = clamp(Reflectivity + (1.0-Reflectivity)*pow((1-cosine),5.0),0.0,1.0);

        vec4 Kd = texture(myTextureSampler, UV);
        vec4 T = texture(cubemap, normalize(RefractDir));
        vec4 R = texture(cubemap, normalize(ReflectDir));
        color = mix(T, mix(Kd, R, 0.7), rtheta).xyz;
        // vec3 ortho = RefractDir+reflect(RefractDir, normalize(fragmentNormal));
        // vec3 para = RefractDir-reflect(RefractDir, normalize(fragmentNormal));
        
        // vec2 dUV = thickness / length(para) * ortho;
	}
}
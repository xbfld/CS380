#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentNormal;
in vec2 UV;

smooth in vec3 ReflectDir;
smooth in vec3 RefractDir;
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
        vec4 Kd = texture(myTextureSampler, UV);
        color = mix(Kd, texture(cubemap, normalize(RefractDir)), 0.85).xyz;
        // vec3 ortho = RefractDir+reflect(RefractDir, normalize(fragmentNormal));
        // vec3 para = RefractDir-reflect(RefractDir, normalize(fragmentNormal));
        
        // vec2 dUV = thickness / length(para) * ortho;
	}
}
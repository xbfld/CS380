#version 330 core

in vec3 fragmentPosition;
in vec3 fragmentNormal;
in vec2 UV;

smooth in vec3 ReflectDir;

// Ouput data
out vec3 color;

uniform vec3 uLight;
uniform bool DrawSkyBox;

uniform samplerCube cubemap;
uniform sampler2D myTextureSampler;

void main(){
	vec3 normal = normalize(fragmentNormal);

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
	   color = mix(Kd, texColor, 0.85).xyz;
	}
}
#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexNormal_modelspace;
layout(location = 3) in vec2 vertexUV;
layout(location = 4) in vec3 tangents;

// Output data ; will be interpolated for each fragment.
out vec3 fragmentPosition;
out vec3 fragmentNormal;
out vec2 UV;

smooth out vec3 ulightdir;
smooth out vec3 utovdir;
smooth out vec3 uhalf;
smooth out vec3 vlightdir;
smooth out vec3 vtovdir;
smooth out vec3 vhalf;

uniform mat4 ModelTransform;
uniform mat4 Eye;
uniform mat4 Projection;

uniform vec3 uLight;
uniform vec3 vLight;

uniform sampler2D myTextureSampler;
uniform sampler2D myBumpSampler;

vec3 tspace(vec3 p, vec3 t, vec3 b, vec3 n);

void main(){
	// Output position of the vertex, in clip space : MVP * position
	mat4 MVM = inverse(Eye) * ModelTransform;

	vec4 wPosition = MVM * vec4(vertexPosition_modelspace, 1);
    // wPosition.xyz = floor(wPosition.xyz*5.0)/5.0; // Pixelizer
	fragmentPosition = wPosition.xyz;
	gl_Position = Projection * wPosition;

	//transpose of inversed model view matrix
	mat4 invm = inverse(MVM);
	invm[0][3] = 0; invm[1][3] = 0;	invm[2][3] = 0;
	mat4 NVM = transpose(invm);
	vec4 tnormal = vec4(vertexNormal_modelspace, 0.0);
	fragmentNormal = vec3(NVM * tnormal);
	UV = vertexUV;

	// vec3 dv = texture(myBumpSampler, UV).rgb*2.0 - 1.0;;
	// float df = 0.30*dv.x + 0.59*dv.y + 0.11*dv.z;
	// gl_Position = Projection * vec4(fragmentPosition + normalize(fragmentNormal)*df,1);

	//Light properties in tanget space
	vec3 n = vec3(NVM * tnormal);
	vec3 t = normalize((NVM * vec4(tangents,0))).xyz;
	vec3 b = cross(n, t);

	vec3 vert_pos = wPosition.xyz;
	vec3 lightDir = normalize(uLight);
	vec3 halfVector = normalize(vert_pos + lightDir);

	ulightdir = normalize(tspace(lightDir,t,b,n));
	utovdir = normalize(tspace(vert_pos,t,b,n));
	uhalf = tspace(halfVector,t,b,n);

	//TODO: calculate one or more lights properties in tangent space and pass to fragment shader
	vert_pos = wPosition.xyz;
	lightDir = normalize(vLight - vert_pos);
	halfVector = normalize(vert_pos + lightDir);

	vlightdir = normalize(tspace(lightDir,t,b,n));
	vtovdir = normalize(tspace(vert_pos,t,b,n));
	vhalf = tspace(halfVector,t,b,n);
}

vec3 tspace(vec3 p, vec3 t, vec3 b, vec3 n)
{
	vec3 v;
	v.x = dot(p, t);
	v.y = dot(p, b);
	v.z = dot(p, n);
	return v;
}
#version 440
in vec3 TexCoords;
in float visibility;

out vec4 color;

uniform samplerCube skybox;

void main(){
	color = vec4(texture(skybox, TexCoords).rgb, 1.0f);
	color = vec4(0.0f, 1.0f, 0.0f, 0.5f);
}
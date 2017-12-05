#version 440

in vec3 TexCoords;
in float visibility;

out vec4 dacolor;

uniform samplerCube skybox;

void main(){
	dacolor = texture(skybox, TexCoords);
}
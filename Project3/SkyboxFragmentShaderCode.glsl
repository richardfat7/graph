#version 440

in vec3 TexCoords;
in float visibility;

out vec4 finalColor;

uniform vec3 fogColor;
uniform samplerCube skybox;

void main(){
	vec4 daColor = texture(skybox, TexCoords);
	finalColor = mix(vec4(fogColor, 1.0f), daColor, visibility);
}
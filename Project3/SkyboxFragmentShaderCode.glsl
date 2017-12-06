#version 440

in vec3 TexCoords;
in float visibility;


uniform samplerCube skybox;

uniform vec3 fog_Color;

out vec4 fogfinalColor;


void main(){
	vec4 finalColor = texture(skybox, TexCoords);
	fogfinalColor = mix (vec4(fog_Color, 1.0f), finalColor, visibility);
}
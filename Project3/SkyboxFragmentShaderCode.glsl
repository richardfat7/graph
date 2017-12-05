#version 440
<<<<<<< HEAD
=======
in vec3 TexCoords;
in float visibility;
>>>>>>> 7520b4c52c915aa344540c3fdee18e91ba8df8eb

in vec3 TexCoords;
in float visibility;

<<<<<<< HEAD
out vec4 finalColor;

uniform vec3 fogColor;
uniform samplerCube skybox;

void main(){
	vec4 daColor = texture(skybox, TexCoords);
	finalColor = mix(vec4(fogColor, 1.0f), daColor, visibility);
=======
uniform samplerCube skybox;

void main(){
	color = vec4(texture(skybox, TexCoords).rgb, 1.0f);
	color = vec4(0.0f, 1.0f, 0.0f, 0.5f);
>>>>>>> 7520b4c52c915aa344540c3fdee18e91ba8df8eb
}
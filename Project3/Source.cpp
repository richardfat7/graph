/*********************************************************
FILE : main.cpp (csci3260 2017-2018 Project)
*********************************************************/
/*********************************************************
Student Information
Student ID: 1155078385
Student Name: Ng Ka Sing
Student ID: 1155079326
Student Name: Tang Justin Wang Lap
*********************************************************/

#define _CRT_SECURE_NO_WARNINGS

#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include "Dependencies\glm\glm.hpp"
#include "Dependencies\glm\gtc\matrix_transform.hpp"
#include "Dependencies\glui\glui.h"
#include "Dependencies\irrKlang\irrKlang.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;
using namespace irrklang;
using glm::vec3;
using glm::mat4;

GLint programID, instanceProgramID, skyboxProgramID, particleProgramID;
GLUI *glui;
/*
index	vao/drawSize	texture
---------------------------------------
0	skybox			skybox
1	planet			planetA
2	spaceship		planetA_normal
3	lightbox		planetB
4	rock			planetC1
5	star			planetC2
6	alien			spaceship
7	particle		rock
8					star
9					alien
*/
GLuint texture[16], vao[10], drawSize[10];

GLfloat vehicleSpeed = 0.005f, vehicleOrbit = 0.0f;
GLfloat camera_fov = 44.5f, angle_horizontal = 3.14f, angle_vertical = 0.0f;
GLfloat ratio = 1.0f, ambient = 0.3f, diffuse1 = 1.0f, specular1 = 1.0f, diffuse2 = 1.0f, specular2 = 1.0f;
GLint curTime, prevTime, mouseX, mouseY, centerX, centerY;
bool viewControl = false, fog = false;

glm::mat4 projectionMatrix = glm::perspective(camera_fov, ratio, 0.1f, 200.0f);
glm::vec3 camera = glm::vec3(0, 0, 0);
glm::vec3 front = glm::vec3(0, 0, -1);
glm::vec3 up = glm::vec3(0, 1, 0);
glm::mat4 viewMatrix = glm::lookAt(camera, camera + front, up);
glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
glm::mat4 rotationMatrixA = glm::mat4(1.0f);
glm::mat4 rotationMatrixB = glm::mat4(1.0f);
glm::mat4 selfRotationMatrixB = glm::mat4(1.0f);
glm::mat4 rotationMatrixC = glm::mat4(1.0f);
glm::mat4 rotationMatrixD = glm::mat4(1.0f); //spaceship
glm::mat4 rotationMatrixE = glm::mat4(1.0f); //light source 1
glm::mat4 rotationMatrixF = glm::mat4(1.0f); //light source 2
glm::mat4 rotationMatrixG = glm::mat4(1.0f); //rock
glm::vec3 fogColor = vec3(1.0f, 1.0f, 0.9f);

const int numRock = 6000, numStar = 5, maxAlien = 10, maxParticle = 1000;
GLint curStar = 0, lastStarTime;
glm::mat4* modelStars = new glm::mat4[numStar];

int viewpoints = 0, fogOnOff = 0, fogCol = 0;
GLfloat alienMinX, alienMaxX, alienMinY, alienMaxY, alienMinZ, alienMaxZ;
GLfloat tempMinX = 1000000.0f, tempMaxX = -1000000.0f, tempMinY = 1000000.0f, tempMaxY = -1000000.0f, tempMinZ = 1000000.0f, tempMaxZ = -1000000.0f;

struct Alien {
	glm::vec3 pos;
	GLfloat xmin, xmax, ymin, ymax, zmin, zmax, size;
	bool life;
};
Alien alienContainer[maxAlien];
glm::mat4* modelAlien = new glm::mat4[maxAlien];
GLint newestAlien = 0;
GLint findNewestAlien() {
	for (int i = newestAlien; i < newestAlien + maxAlien; i++) {
		if (alienContainer[i % maxAlien].life == false) {
			newestAlien = i % maxAlien;
			return newestAlien;
		}
	}
	return -1;
}

struct Particle {
	glm::vec3 pos, speed;
	GLfloat xmin, xmax, ymin, ymax, zmin, zmax, distance, size;
	bool life;
	bool operator<(const Particle& that) const {
		return this->distance > that.distance;
	}
};
Particle particleContainer[maxParticle];
GLuint particleVbo, particleV, particleC;
GLint newestParticle = 0;
static GLfloat *particlePosition = new GLfloat[maxParticle * 4];
static GLubyte *particleColor = new GLubyte[maxParticle * 4];
void sortParticle() {
	std::sort(&particleContainer[0], &particleContainer[maxParticle]);
}
GLint findNewestParticle() {
	for (int i = newestParticle; i < newestParticle + maxParticle; i++) {
		if (particleContainer[i % maxParticle].life == false) {
			newestParticle = i % maxParticle;
			return newestParticle;
		}
	}
	return -1;
}

bool collision(Alien a, Particle p) {
	return (a.xmin <= p.xmax && a.xmax >= p.xmin && a.ymin <= p.ymax && a.ymax >= p.ymin && a.zmin <= p.zmax && a.zmax >= p.zmin);
}

bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType) {
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE) {
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
		cout << buffer << endl;

		delete[] buffer;
		return false;
	}
	return true;
}

bool checkShaderStatus(GLuint shaderID) {
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID) {
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

string readShaderCode(const char* fileName) {
	ifstream meInput(fileName);
	if (!meInput.good()) {
		cout << "File failed to load..." << fileName;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

void installShaders() {
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode("FragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))	return;

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	if (!checkProgramStatus(programID))	return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(programID);
}

void installInstanceShaders() {
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("InstanceVertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode("InstanceFragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))	return;

	instanceProgramID = glCreateProgram();
	glAttachShader(instanceProgramID, vertexShaderID);
	glAttachShader(instanceProgramID, fragmentShaderID);
	glLinkProgram(instanceProgramID);

	if (!checkProgramStatus(instanceProgramID))	return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(instanceProgramID);
}

void installSkyboxShaders() {
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("SkyboxVertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode("SkyboxFragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))	return;

	skyboxProgramID = glCreateProgram();
	glAttachShader(skyboxProgramID, vertexShaderID);
	glAttachShader(skyboxProgramID, fragmentShaderID);
	glLinkProgram(skyboxProgramID);

	if (!checkProgramStatus(skyboxProgramID)) return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(skyboxProgramID);
}

void installParticleShaders() {
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("ParticleVertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode("ParticleFragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))	return;

	particleProgramID = glCreateProgram();
	glAttachShader(particleProgramID, vertexShaderID);
	glAttachShader(particleProgramID, fragmentShaderID);
	glLinkProgram(particleProgramID);

	if (!checkProgramStatus(particleProgramID)) return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(particleProgramID);
}

void reshape(int w, int h) {
	int tx, ty, tw, th;
	GLUI_Master.get_viewport_area(&tx, &ty, &tw, &th);
	glViewport(tx, ty, tw, th);
	ratio = 1.0f * tw / th;
	centerX = tw / 2;
	centerY = th / 2;
}

void keyboard(unsigned char key, int x, int y) {
	//TODO: Use keyboard to do interactive events and animation
	if (key == 'q' && diffuse1 > 0) diffuse1 -= 0.05f;
	if (key == 'w') diffuse1 += 0.05f;
	if (key == 'e' && specular1 > 0) specular1 -= 0.1f;
	if (key == 'r') specular1 += 0.1f;
	if (key == 't' && diffuse2 > 0) diffuse2 -= 0.05f;
	if (key == 'y') diffuse2 += 0.05f;
	if (key == 'u' && specular2 > 0) specular2 -= 0.1f;
	if (key == 'i') specular2 += 0.1f;

	if (key == 'a') viewpoints = 1;
	if (key == 's') viewpoints = 2;
	if (key == 'd') viewpoints = 3;
	if (key == 'f') viewpoints = 0;
	if (key == 'g') viewpoints = 4;

	if (key == 'p') {
		if (viewControl == false) {
			viewControl = true;
			prevTime = glutGet(GLUT_ELAPSED_TIME);
			glutSetCursor(GLUT_CURSOR_NONE);
		}
		else {
			viewControl = false;
			glutSetCursor(GLUT_CURSOR_INHERIT);
		}
	}
	if (key == 'o') {
		GLint alienIndex = findNewestAlien();
		if (alienIndex != -1) {
			alienContainer[alienIndex].life = true;
			alienContainer[alienIndex].pos.x = rand() % 150 / 10.0f - 0.4f;
			alienContainer[alienIndex].pos.y = rand() % 60 / 10.0f + 0.7f;
			alienContainer[alienIndex].pos.z = -rand() % 300 / 10.0f - 36.6f;
			alienContainer[alienIndex].size = rand() % 10 / 10.0f + 0.3f;
			alienContainer[alienIndex].xmin = alienContainer[alienIndex].pos.x + alienMinX;
			alienContainer[alienIndex].xmax = alienContainer[alienIndex].pos.x + alienMaxX;
			alienContainer[alienIndex].ymin = alienContainer[alienIndex].pos.y + alienMinY;
			alienContainer[alienIndex].ymax = alienContainer[alienIndex].pos.y + alienMaxY;
			alienContainer[alienIndex].zmin = alienContainer[alienIndex].pos.z + alienMinZ;
			alienContainer[alienIndex].zmax = alienContainer[alienIndex].pos.z + alienMaxZ;
		}
	}
	if (key == ' ') {
		GLint particleIndex = findNewestParticle();
		if (particleIndex != -1) {
			particleContainer[particleIndex].life = true;
			particleContainer[particleIndex].pos = glm::vec3(0, 0, 0);
			particleContainer[particleIndex].speed = front;
			particleContainer[particleIndex].size = 0.5f;
		}
	}
}

void move(int key, int x, int y) {
	if (key == GLUT_KEY_UP) vehicleSpeed += 0.001f;
	if (key == GLUT_KEY_DOWN) vehicleSpeed -= 0.001f;
	if (key == GLUT_KEY_LEFT) vehicleOrbit -= 0.1f;
	if (key == GLUT_KEY_RIGHT) vehicleOrbit += 0.1f;
}

void PassiveMouse(int x, int y) {
	mouseX = x;
	mouseY = y;
}

void Mouse_Wheel_Func(int button, int state, int x, int y) {
	if (button == 3 || button == 4) {
		if (state == GLUT_UP) return;
		if (button == 3) camera_fov = max(44.1f, camera_fov - 0.001f);
		else camera_fov = min(44.9f, camera_fov + 0.001f);
	}
}

bool loadOBJ(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			if (vertex.x < tempMinX) tempMinX = vertex.x;
			if (vertex.x > tempMaxX) tempMaxX = vertex.x;
			if (vertex.y < tempMinY) tempMinY = vertex.y;
			if (vertex.y > tempMaxY) tempMaxY = vertex.y;
			if (vertex.z < tempMinZ) tempMinZ = vertex.z;
			if (vertex.z > tempMaxZ) tempMaxZ = vertex.z;
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i<vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}

	return true;
}

GLuint loadBMP_custom(const char * imagepath) {

	printf("Reading image %s\n", imagepath);

	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	unsigned char * data;

	FILE * file = fopen(imagepath, "rb");
	if (!file) { printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0; }

	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    return 0; }
	if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return 0; }

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	if (imageSize == 0) imageSize = width*height * 3;
	if (dataPos == 0) dataPos = 54;

	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);


	GLuint textureID;
	//TODO: Create one OpenGL texture and set the texture parameter 
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[]data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	return textureID;
}

GLuint loadCubemap(vector<const GLchar*>faces) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++) {
		const GLchar* imagepath = faces.at(i);
		printf("Reading image %s\n", imagepath);

		unsigned char header[54];
		unsigned int dataPos;
		unsigned int imageSize;
		unsigned int width, height;
		unsigned char * data;

		FILE * file = fopen(imagepath, "rb");
		if (!file) { printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0; }

		if (fread(header, 1, 54, file) != 54) {
			printf("Not a correct BMP file\n");
			return 0;
		}
		if (header[0] != 'B' || header[1] != 'M') {
			printf("Not a correct BMP file\n");
			return 0;
		}
		if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    return 0; }
		if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return 0; }

		dataPos = *(int*)&(header[0x0A]);
		imageSize = *(int*)&(header[0x22]);
		width = *(int*)&(header[0x12]);
		height = *(int*)&(header[0x16]);
		if (imageSize == 0) imageSize = width*height * 3;
		if (dataPos == 0) dataPos = 54;

		data = new unsigned char[imageSize];
		fread(data, 1, imageSize, file);
		fclose(file);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		delete[] data;
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return textureID;
}

void sendDataToOpenGL() {
	//TODO:
	//Load objects and bind to VAO & VBO
	//Load texture
	std::vector<glm::vec3> vertexbuffer[10];
	std::vector<glm::vec2> uvbuffer[10];
	std::vector<glm::vec3> normalbuffer[10];
	bool res;
	GLuint vertex[10], uv[10], normal[10];

	/*skybox*/
	GLfloat skyboxVertices[] = {
		-100.0f, +100.0f, -100.0f,
		-100.0f, -100.0f, -100.0f,
		+100.0f, -100.0f, -100.0f,
		+100.0f, -100.0f, -100.0f,
		+100.0f, +100.0f, -100.0f,
		-100.0f, +100.0f, -100.0f,

		-100.0f, -100.0f, +100.0f,
		-100.0f, -100.0f, -100.0f,
		-100.0f, +100.0f, -100.0f,
		-100.0f, +100.0f, -100.0f,
		-100.0f, +100.0f, +100.0f,
		-100.0f, -100.0f, +100.0f,

		+100.0f, -100.0f, -100.0f,
		+100.0f, -100.0f, +100.0f,
		+100.0f, +100.0f, +100.0f,
		+100.0f, +100.0f, +100.0f,
		+100.0f, +100.0f, -100.0f,
		+100.0f, -100.0f, -100.0f,

		-100.0f, -100.0f, +100.0f,
		-100.0f, +100.0f, +100.0f,
		+100.0f, +100.0f, +100.0f,
		+100.0f, +100.0f, +100.0f,
		+100.0f, -100.0f, +100.0f,
		-100.0f, -100.0f, +100.0f,

		-100.0f, +100.0f, -100.0f,
		+100.0f, +100.0f, -100.0f,
		+100.0f, +100.0f, +100.0f,
		+100.0f, +100.0f, +100.0f,
		-100.0f, +100.0f, +100.0f,
		-100.0f, +100.0f, -100.0f,

		-100.0f, -100.0f, -100.0f,
		-100.0f, -100.0f, +100.0f,
		+100.0f, -100.0f, -100.0f,
		+100.0f, -100.0f, -100.0f,
		-100.0f, -100.0f, +100.0f,
		+100.0f, -100.0f, +100.0f
	};
	glGenVertexArrays(1, &vao[0]);
	glBindVertexArray(vao[0]);
	glGenBuffers(1, &vertex[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vertex[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	drawSize[0] = GLint(sizeof(skyboxVertices));

	/*load planet*/
	res = loadOBJ("planet.obj", vertexbuffer[1], uvbuffer[1], normalbuffer[1]);
	glGenVertexArrays(1, &vao[1]);
	glBindVertexArray(vao[1]);
	glGenBuffers(1, &vertex[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vertex[1]);
	glBufferData(GL_ARRAY_BUFFER, vertexbuffer[1].size() * sizeof(glm::vec3), &vertexbuffer[1][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, &uv[1]);
	glBindBuffer(GL_ARRAY_BUFFER, uv[1]);
	glBufferData(GL_ARRAY_BUFFER, uvbuffer[1].size() * sizeof(glm::vec2), &uvbuffer[1][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, &normal[1]);
	glBindBuffer(GL_ARRAY_BUFFER, normal[1]);
	glBufferData(GL_ARRAY_BUFFER, normalbuffer[1].size() * sizeof(glm::vec3), &normalbuffer[1][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	drawSize[1] = GLint(vertexbuffer[1].size());

	/*load spaceship*/
	res = loadOBJ("spaceship.obj", vertexbuffer[2], uvbuffer[2], normalbuffer[2]);
	glGenVertexArrays(1, &vao[2]);
	glBindVertexArray(vao[2]);
	glGenBuffers(1, &vertex[2]);
	glBindBuffer(GL_ARRAY_BUFFER, vertex[2]);
	glBufferData(GL_ARRAY_BUFFER, vertexbuffer[2].size() * sizeof(glm::vec3), &vertexbuffer[2][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, &uv[2]);
	glBindBuffer(GL_ARRAY_BUFFER, uv[2]);
	glBufferData(GL_ARRAY_BUFFER, uvbuffer[2].size() * sizeof(glm::vec2), &uvbuffer[2][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, &normal[2]);
	glBindBuffer(GL_ARRAY_BUFFER, normal[2]);
	glBufferData(GL_ARRAY_BUFFER, normalbuffer[2].size() * sizeof(glm::vec3), &normalbuffer[2][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	drawSize[2] = GLint(vertexbuffer[2].size());

	/*light source box*/
	res = loadOBJ("lightbox.obj", vertexbuffer[3], uvbuffer[3], normalbuffer[3]);
	glGenVertexArrays(1, &vao[3]);
	glBindVertexArray(vao[3]);
	glGenBuffers(1, &vertex[3]);
	glBindBuffer(GL_ARRAY_BUFFER, vertex[3]);
	glBufferData(GL_ARRAY_BUFFER, vertexbuffer[3].size() * sizeof(glm::vec3), &vertexbuffer[3][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, &uv[3]);
	glBindBuffer(GL_ARRAY_BUFFER, uv[3]);
	glBufferData(GL_ARRAY_BUFFER, uvbuffer[3].size() * sizeof(glm::vec2), &uvbuffer[3][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, &normal[3]);
	glBindBuffer(GL_ARRAY_BUFFER, normal[3]);
	glBufferData(GL_ARRAY_BUFFER, normalbuffer[3].size() * sizeof(glm::vec3), &normalbuffer[3][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	drawSize[3] = GLint(vertexbuffer[3].size());

	/*load rock*/
	res = loadOBJ("rock.obj", vertexbuffer[4], uvbuffer[4], normalbuffer[4]);
	glGenVertexArrays(1, &vao[4]);
	glBindVertexArray(vao[4]);
	glGenBuffers(1, &vertex[4]);
	glBindBuffer(GL_ARRAY_BUFFER, vertex[4]);
	glBufferData(GL_ARRAY_BUFFER, vertexbuffer[4].size() * sizeof(glm::vec3), &vertexbuffer[4][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, &uv[4]);
	glBindBuffer(GL_ARRAY_BUFFER, uv[4]);
	glBufferData(GL_ARRAY_BUFFER, uvbuffer[4].size() * sizeof(glm::vec2), &uvbuffer[4][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, &normal[4]);
	glBindBuffer(GL_ARRAY_BUFFER, normal[4]);
	glBufferData(GL_ARRAY_BUFFER, normalbuffer[4].size() * sizeof(glm::vec3), &normalbuffer[4][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	drawSize[4] = GLint(vertexbuffer[4].size());

	GLfloat radius = 5.0f, offset = 1.0f;
	glm::mat4* modelMatrices = new glm::mat4[numRock];
	for (GLuint i = 0; i < numRock; i++) {
		glm::mat4 model;
		GLfloat angle = GLfloat(i) / GLfloat(numRock) * 6.28f;
		GLfloat displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		GLfloat x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		GLfloat y = displacement * 0.1f;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		GLfloat z = cos(angle) * radius + displacement;
		model = glm::translate(model, glm::vec3(x, y, z));
		GLfloat scale = (rand() % 100) / 2500.0f + 0.0025f;
		model = glm::scale(model, glm::vec3(scale));
		GLfloat rotAngle = GLfloat(rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));
		modelMatrices[i] = model;
	}
	GLuint rockBuffer;
	glGenBuffers(1, &rockBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, rockBuffer);
	glBufferData(GL_ARRAY_BUFFER, numRock * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);

	/*load star*/
	res = loadOBJ("star.obj", vertexbuffer[5], uvbuffer[5], normalbuffer[5]);
	glGenVertexArrays(1, &vao[5]);
	glBindVertexArray(vao[5]);
	glGenBuffers(1, &vertex[5]);
	glBindBuffer(GL_ARRAY_BUFFER, vertex[5]);
	glBufferData(GL_ARRAY_BUFFER, vertexbuffer[5].size() * sizeof(glm::vec3), &vertexbuffer[5][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, &uv[5]);
	glBindBuffer(GL_ARRAY_BUFFER, uv[5]);
	glBufferData(GL_ARRAY_BUFFER, uvbuffer[5].size() * sizeof(glm::vec2), &uvbuffer[5][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, &normal[5]);
	glBindBuffer(GL_ARRAY_BUFFER, normal[5]);
	glBufferData(GL_ARRAY_BUFFER, normalbuffer[5].size() * sizeof(glm::vec3), &normalbuffer[5][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	drawSize[5] = GLint(vertexbuffer[5].size());
	for (GLuint i = 0; i < numStar; i++) {
		modelStars[i] = glm::translate(glm::mat4(1.0f), glm::vec3(-0.8f, 5.6f + vehicleOrbit, -3.1f));
		modelStars[i] = glm::scale(modelStars[i], glm::vec3(0.3f, 0.3f, 0.3f));
	}
	lastStarTime = glutGet(GLUT_ELAPSED_TIME);

	/*load alien*/
	tempMinX = 1000000.0f, tempMaxX = -1000000.0f, tempMinY = 1000000.0f, tempMaxY = -1000000.0f, tempMinZ = 1000000.0f, tempMaxZ = -1000000.0f;
	res = loadOBJ("alien.obj", vertexbuffer[6], uvbuffer[6], normalbuffer[6]);
	alienMinX = tempMinX;
	alienMaxX = tempMaxX;
	alienMinY = tempMinY;
	alienMaxY = tempMaxY;
	alienMinZ = tempMinZ;
	alienMaxZ = tempMaxZ;
	glGenVertexArrays(1, &vao[6]);
	glBindVertexArray(vao[6]);
	glGenBuffers(1, &vertex[6]);
	glBindBuffer(GL_ARRAY_BUFFER, vertex[6]);
	glBufferData(GL_ARRAY_BUFFER, vertexbuffer[6].size() * sizeof(glm::vec3), &vertexbuffer[6][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, &uv[6]);
	glBindBuffer(GL_ARRAY_BUFFER, uv[6]);
	glBufferData(GL_ARRAY_BUFFER, uvbuffer[6].size() * sizeof(glm::vec2), &uvbuffer[6][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, &normal[6]);
	glBindBuffer(GL_ARRAY_BUFFER, normal[6]);
	glBufferData(GL_ARRAY_BUFFER, normalbuffer[6].size() * sizeof(glm::vec3), &normalbuffer[6][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	drawSize[6] = GLint(vertexbuffer[6].size());

	/*create particle*/
	glGenVertexArrays(1, &vao[7]);
	glBindVertexArray(vao[7]);
	static const GLfloat particleVertex[] = {
		-0.5f, -0.5f, +0.0f,
		+0.5f, -0.5f, +0.0f,
		-0.5f, +0.5f, +0.0f,
		+0.5f, +0.5f, +0.0f,
	};
	glGenBuffers(1, &particleVbo);
	glBindBuffer(GL_ARRAY_BUFFER, particleVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particleVertex), particleVertex, GL_STATIC_DRAW);
	glGenBuffers(1, &particleV);
	glBindBuffer(GL_ARRAY_BUFFER, particleV);
	glBufferData(GL_ARRAY_BUFFER, maxParticle * 4 * sizeof(GLfloat), 0, GL_STREAM_DRAW);
	glGenBuffers(1, &particleC);
	glBindBuffer(GL_ARRAY_BUFFER, particleC);
	glBufferData(GL_ARRAY_BUFFER, maxParticle * 4 * sizeof(GLubyte), 0, GL_STREAM_DRAW);

	/*load cube map*/
	vector<const GLchar*> universe_faces;
	universe_faces.push_back("skybox/right.bmp");
	universe_faces.push_back("skybox/left.bmp");
	universe_faces.push_back("skybox/top.bmp");
	universe_faces.push_back("skybox/bottom.bmp");
	universe_faces.push_back("skybox/back.bmp");
	universe_faces.push_back("skybox/front.bmp");
	texture[0] = loadCubemap(universe_faces);

	/*load textures*/
	texture[1] = loadBMP_custom("planetA.bmp");
	texture[2] = loadBMP_custom("planetA_normal.bmp");
	texture[3] = loadBMP_custom("planetB.bmp");
	texture[4] = loadBMP_custom("planetC1.bmp");
	texture[5] = loadBMP_custom("planetC2.bmp");
	texture[6] = loadBMP_custom("spaceship.bmp");
	texture[7] = loadBMP_custom("rock.bmp");
	texture[8] = loadBMP_custom("star.bmp");
	texture[9] = loadBMP_custom("alien.bmp");
}

void paintGL(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*background color*/
	glClearColor(0.0f, 0.2f, 0.0f, 1.0f);

	/*initialize*/
	GLint textureID;
	GLuint lightSource = 0;
	bool normalMap = false, multiTex = false;

	rotationMatrixA = glm::rotate(glm::mat4(1.0f), +0.01f, glm::vec3(1, 1, 1)) * rotationMatrixA;
	selfRotationMatrixB = glm::rotate(glm::mat4(1.0f), -0.02f, glm::vec3(1, 1, 1)) * selfRotationMatrixB;
	rotationMatrixB = glm::rotate(glm::mat4(1.0f), -0.02f, glm::vec3(1, 1, -1)) * rotationMatrixB;
	rotationMatrixC = glm::rotate(glm::mat4(1.0f), -0.005f, glm::vec3(1, -1, 1)) * rotationMatrixC;
	rotationMatrixD = glm::rotate(glm::mat4(1.0f), vehicleSpeed, glm::vec3(1, 0, 0)) * rotationMatrixD;
	rotationMatrixE = glm::rotate(glm::mat4(1.0f), +0.01f, glm::vec3(1, 0, 0)) * rotationMatrixE;
	rotationMatrixF = glm::rotate(glm::mat4(1.0f), +0.01f, glm::vec3(0, 1, 0)) * rotationMatrixF;

	if (viewControl == true) {
		curTime = glutGet(GLUT_ELAPSED_TIME);
		if (curTime - prevTime > 100) {
			angle_horizontal += GLfloat(centerX - mouseX) * 0.0001f;
			angle_vertical += GLfloat(centerY - mouseY) * 0.0001f;
			front.x = cos(angle_vertical) * sin(angle_horizontal);
			front.y = sin(angle_vertical);
			front.z = cos(angle_vertical) * cos(angle_horizontal);
			glm::vec3 right = glm::cross(front, up);
			up = glm::cross(right, front);
			glutWarpPointer(centerX, centerY);
			prevTime = curTime;
		}
	}
	else {
		if (viewpoints == 0) camera = vec3(0.0f, 0.0f, 0.0f), front = vec3(0.0f, 0.0f, -1.0f), up = vec3(0.0f, 1.0f, 0.0f);
		else if (viewpoints == 1) camera = vec3(-40.0f, 0.0f, -35.0f), front = vec3(40.0f, 0.0f, 0.0f), up = vec3(0.0f, 1.0f, 0.0f);
		else if (viewpoints == 2) camera = vec3(40.0f, 0.0f, -35.0f), front = vec3(-40.0f, 0.0f, 0.0f), up = vec3(0.0f, 1.0f, 0.0f);
		else if (viewpoints == 3) camera = vec3(0.0f, 40.0f, -35.0f), front = vec3(0.0f, -40.0f, 0.0f), up = vec3(0.0f, 0.0f, 1.0f);
		else if (viewpoints == 4) {
			modelTransformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 2.5f, -40.0f));
			modelTransformMatrix = modelTransformMatrix * rotationMatrixD;
			modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(-0.8f, 6.8f + vehicleOrbit, -1.5f));
			glm::vec4 tmp = modelTransformMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			camera = vec3(tmp);
			up = glm::normalize(camera - vec3(-10.0f, 2.5f, -40.0f));
			front = glm::cross(vec3(vehicleSpeed, 0.0f, 0.0f), up);
		}
		if (fogOnOff == 0) fog = false;
		else if (fogOnOff == 1) fog = true;
		if (fogCol == 0) fogColor = vec3(0.8f, 0.8f, 0.7f);
		else if (fogCol == 1) fogColor = vec3(0.1f, 0.4f, 0.6f);
	}
	viewMatrix = glm::lookAt(camera, camera + front, up);
	projectionMatrix = glm::perspective(camera_fov, ratio, 0.1f, 200.0f);

	/*skybox*/
	glDepthMask(GL_FALSE);
	glUseProgram(skyboxProgramID);

	GLint modelTransformMatrixUniformLocation = glGetUniformLocation(skyboxProgramID, "M");
	GLint viewMatrixUniformLocation = glGetUniformLocation(skyboxProgramID, "view");
	GLint projectionMatrixUniformLocation = glGetUniformLocation(skyboxProgramID, "projection");
	GLint fogUniformLocation = glGetUniformLocation(skyboxProgramID, "fog");
	GLint fogColorUniformLocation = glGetUniformLocation(skyboxProgramID, "fogColor");

	modelTransformMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	glm::mat4 view = glm::mat4(glm::mat3(viewMatrix));
	glUniformMatrix4fv(viewMatrixUniformLocation, 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniform1i(fogUniformLocation, fog);
	glUniform3fv(fogColorUniformLocation, 1, &fogColor[0]);

	glBindVertexArray(vao[0]);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(skyboxProgramID, "skybox"), 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture[0]);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[0]);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);


	/*objects and lights*/
	glUseProgram(programID);

	modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
	viewMatrixUniformLocation = glGetUniformLocation(programID, "viewMatrix");
	projectionMatrixUniformLocation = glGetUniformLocation(programID, "projectionMatrix");
	textureID = glGetUniformLocation(programID, "myTextureSampler");
	fogUniformLocation = glGetUniformLocation(programID, "fog");
	fogColorUniformLocation = glGetUniformLocation(programID, "fogColor");
	GLint normalMapUniformLocation = glGetUniformLocation(programID, "normalMap");
	GLint multiTexUniformLocation = glGetUniformLocation(programID, "multiTex");
	GLint lightSourceUniformLocation = glGetUniformLocation(programID, "lightSource");
	GLint ambientLightUniformLocation = glGetUniformLocation(programID, "ambientLight");
	GLint lightPosition1UniformLocation = glGetUniformLocation(programID, "lightPositionWorld1");
	GLint diffuseLightBrightness1UniformLocation = glGetUniformLocation(programID, "diffuseLightBrightness1");
	GLint specularLightBrightness1UniformLocation = glGetUniformLocation(programID, "specularLightBrightness1");
	GLint lightPosition2UniformLocation = glGetUniformLocation(programID, "lightPositionWorld2");
	GLint diffuseLightBrightness2UniformLocation = glGetUniformLocation(programID, "diffuseLightBrightness2");
	GLint specularLightBrightness2UniformLocation = glGetUniformLocation(programID, "specularLightBrightness2");
	GLint eyePositionUniformLocation = glGetUniformLocation(programID, "eyePositionWorld");

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(programID, "skybox"), 0);
	glUniformMatrix4fv(viewMatrixUniformLocation, 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniform1i(normalMapUniformLocation, normalMap);
	glUniform1i(multiTexUniformLocation, multiTex);
	glUniform1i(lightSourceUniformLocation, lightSource);
	glUniform1i(fogUniformLocation, fog);
	glUniform3fv(fogColorUniformLocation, 1, &fogColor[0]);
	glUniform3fv(eyePositionUniformLocation, 1, &camera[0]);

	/*planet A*/
	glBindVertexArray(vao[1]);
	modelTransformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 2.5f, -40.0f));
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
	modelTransformMatrix = modelTransformMatrix * rotationMatrixA;
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	normalMap = true;
	glUniform1i(normalMapUniformLocation, normalMap);
	textureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glUniform1i(textureID, 1);
	textureID = glGetUniformLocation(programID, "myTextureSampler2");
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glUniform1i(textureID, 2);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[1]);
	normalMap = false;
	glUniform1i(normalMapUniformLocation, normalMap);

	/*planet B*/
	glBindVertexArray(vao[1]);
	modelTransformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 2.5f, -40.0f));
	modelTransformMatrix = modelTransformMatrix * rotationMatrixB;
	modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(-1.8f, -3.4f, -7.9f));
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.2f, 0.2f, 0.2f));
	modelTransformMatrix = modelTransformMatrix * selfRotationMatrixB;
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	textureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glUniform1i(textureID, 3);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[1]);

	/*planet C*/
	glBindVertexArray(vao[1]);
	modelTransformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(+8.0f, -2.7f, -29.0f));
	modelTransformMatrix = modelTransformMatrix * rotationMatrixC;
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	multiTex = true;
	glUniform1i(multiTexUniformLocation, multiTex);
	textureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, texture[4]);
	glUniform1i(textureID, 4);
	textureID = glGetUniformLocation(programID, "myTextureSampler2");
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, texture[5]);
	glUniform1i(textureID, 5);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[1]);
	multiTex = false;
	glUniform1i(multiTexUniformLocation, multiTex);

	/*spaceship*/
	glBindVertexArray(vao[2]);
	modelTransformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 2.5f, -40.0f));
	modelTransformMatrix = modelTransformMatrix * rotationMatrixD;
	modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(12.7f, 6.4f + vehicleOrbit, 7.8f));
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(3.9f, 3.9f, 3.9f));
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	textureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, texture[6]);
	glUniform1i(textureID, 6);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[2]);

	/*ambient light*/
	glm::vec4 ambientLight(ambient, ambient, ambient, 1.0f);
	glUniform4fv(ambientLightUniformLocation, 1, &ambientLight[0]);

	/*light source box 1*/
	glBindVertexArray(vao[3]);
	modelTransformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -40.0f));
	modelTransformMatrix = modelTransformMatrix * rotationMatrixE;
	modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(0.0f, 15.0f, 0.0f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, +0.5f, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	lightSource = 1;
	glUniform1i(lightSourceUniformLocation, lightSource);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[3]);
	lightSource = 0;
	glUniform1i(lightSourceUniformLocation, lightSource);
	/*light source 1*/
	glm::vec4 lightPositionTemp1 = modelTransformMatrix * glm::vec4(0.0f, 15.0f, 0.0f, 0.0f);
	glm::vec3 lightPosition1 = glm::vec3(lightPositionTemp1[0], lightPositionTemp1[1], lightPositionTemp1[2]);
	glUniform3fv(lightPosition1UniformLocation, 1, &lightPosition1[0]);
	glm::vec4 diffuseLightBrightness1(diffuse1, diffuse1, diffuse1, 1.0f);
	glUniform4fv(diffuseLightBrightness1UniformLocation, 1, &diffuseLightBrightness1[0]);
	glm::vec4 specularLightBrightness1(specular1, specular1, specular1, 1.0f);
	glUniform4fv(specularLightBrightness1UniformLocation, 1, &specularLightBrightness1[0]);

	/*light source box 2*/
	glBindVertexArray(vao[3]);
	modelTransformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -40.0f));
	modelTransformMatrix = modelTransformMatrix * rotationMatrixF;
	modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(30.0f, 0.0f, 0.0f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, +0.5f, glm::vec3(1, 0, 0));
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	lightSource = 2;
	glUniform1i(lightSourceUniformLocation, lightSource);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[3]);
	lightSource = 0;
	glUniform1i(lightSourceUniformLocation, lightSource);
	/*light source 2*/
	glm::vec4 lightPositionTemp2 = modelTransformMatrix * glm::vec4(30.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 lightPosition2 = glm::vec3(lightPositionTemp2[0], lightPositionTemp2[1], lightPositionTemp2[2]);
	glUniform3fv(lightPosition2UniformLocation, 1, &lightPosition2[0]);
	glm::vec4 diffuseLightBrightness2(diffuse2, diffuse2, diffuse2, 1.0f);
	glUniform4fv(diffuseLightBrightness2UniformLocation, 1, &diffuseLightBrightness2[0]);
	glm::vec4 specularLightBrightness2(specular2, specular2, specular2, 1.0f);
	glUniform4fv(specularLightBrightness2UniformLocation, 1, &specularLightBrightness2[0]);


	glUseProgram(instanceProgramID);

	modelTransformMatrixUniformLocation = glGetUniformLocation(instanceProgramID, "modelTransformMatrix");
	viewMatrixUniformLocation = glGetUniformLocation(instanceProgramID, "viewMatrix");
	projectionMatrixUniformLocation = glGetUniformLocation(instanceProgramID, "projectionMatrix");
	ambientLightUniformLocation = glGetUniformLocation(instanceProgramID, "ambientLight");
	lightPosition1UniformLocation = glGetUniformLocation(instanceProgramID, "lightPositionWorld1");
	diffuseLightBrightness1UniformLocation = glGetUniformLocation(instanceProgramID, "diffuseLightBrightness1");
	specularLightBrightness1UniformLocation = glGetUniformLocation(instanceProgramID, "specularLightBrightness1");
	lightPosition2UniformLocation = glGetUniformLocation(instanceProgramID, "lightPositionWorld2");
	diffuseLightBrightness2UniformLocation = glGetUniformLocation(instanceProgramID, "diffuseLightBrightness2");
	specularLightBrightness2UniformLocation = glGetUniformLocation(instanceProgramID, "specularLightBrightness2");
	fogUniformLocation = glGetUniformLocation(instanceProgramID, "fog");
	fogColorUniformLocation = glGetUniformLocation(instanceProgramID, "fogColor");

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(instanceProgramID, "skybox"), 0);
	glUniformMatrix4fv(viewMatrixUniformLocation, 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniform1i(fogUniformLocation, fog);
	glUniform4fv(ambientLightUniformLocation, 1, &ambientLight[0]);
	glUniform3fv(lightPosition1UniformLocation, 1, &lightPosition1[0]);
	glUniform4fv(diffuseLightBrightness1UniformLocation, 1, &diffuseLightBrightness1[0]);
	glUniform4fv(specularLightBrightness1UniformLocation, 1, &specularLightBrightness1[0]);
	glUniform3fv(lightPosition2UniformLocation, 1, &lightPosition2[0]);
	glUniform4fv(diffuseLightBrightness2UniformLocation, 1, &diffuseLightBrightness2[0]);
	glUniform4fv(specularLightBrightness2UniformLocation, 1, &specularLightBrightness2[0]);
	glUniform3fv(fogColorUniformLocation, 1, &fogColor[0]);

	rotationMatrixG = glm::rotate(glm::mat4(1.0f), +0.0005f, glm::vec3(0, 1, 0)) * rotationMatrixG;

	/*rock*/
	glBindVertexArray(vao[4]);
	modelTransformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(8.0f, -3.2f, -29.7f));
	modelTransformMatrix = modelTransformMatrix * rotationMatrixG;
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	textureID = glGetUniformLocation(instanceProgramID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, texture[7]);
	glUniform1i(textureID, 7);
	glDrawArraysInstanced(GL_TRIANGLES, 0, drawSize[4], numRock);

	/*star*/
	glBindVertexArray(vao[5]);
	modelTransformMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 2.5f, -40.0f));
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	GLint curStarTime = glutGet(GLUT_ELAPSED_TIME);
	if (curStarTime - lastStarTime > 1000) {
		modelStars[curStar] = rotationMatrixD;
		modelStars[curStar] = glm::translate(modelStars[curStar], glm::vec3(-0.8f, 5.6f + vehicleOrbit, -3.1f));
		modelStars[curStar] = glm::scale(modelStars[curStar], glm::vec3(0.3f, 0.3f, 0.3f));
		curStar = (curStar + 1) % numStar;
		lastStarTime = curStarTime;
	}
	GLuint starBuffer;
	glGenBuffers(1, &starBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, starBuffer);
	glBufferData(GL_ARRAY_BUFFER, numStar * sizeof(glm::mat4), &modelStars[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	textureID = glGetUniformLocation(instanceProgramID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, texture[8]);
	glUniform1i(textureID, 8);
	glDrawArraysInstanced(GL_TRIANGLES, 0, drawSize[5], numStar);

	/*alien*/
	glBindVertexArray(vao[6]);
	modelTransformMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	for (int i = 0; i < maxAlien; i++) {
		if (alienContainer[i].life == true) {
			modelAlien[i] = glm::translate(glm::mat4(1.0f), alienContainer[i].pos);
			modelAlien[i] = glm::scale(modelAlien[i], glm::vec3(alienContainer[i].size));
		}
		else modelAlien[i] = glm::mat4(0.0f);
	}
	GLuint alienBuffer;
	glGenBuffers(1, &alienBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, alienBuffer);
	glBufferData(GL_ARRAY_BUFFER, maxAlien * sizeof(glm::mat4), &modelAlien[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	textureID = glGetUniformLocation(programID, "myTextureSampler");
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, texture[9]);
	glUniform1i(textureID, 9);
	glDrawArraysInstanced(GL_TRIANGLES, 0, drawSize[6], maxAlien);


	glUseProgram(particleProgramID);

	viewMatrixUniformLocation = glGetUniformLocation(particleProgramID, "viewMatrix");
	projectionMatrixUniformLocation = glGetUniformLocation(particleProgramID, "projectionMatrix");
	modelTransformMatrixUniformLocation = glGetUniformLocation(particleProgramID, "modelTransformMatrix");

	glUniformMatrix4fv(viewMatrixUniformLocation, 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

	/*particle*/
	glBindVertexArray(vao[7]);
	modelTransformMatrix = glm::mat4(1.0f);
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);
	GLint numParticle = 0;
	for (int i = 0; i < maxParticle; i++) {
		if (abs(particleContainer[i].pos.x) > 100.0f || abs(particleContainer[i].pos.y) > 100.0f || abs(particleContainer[i].pos.z) > 100.0f) particleContainer[i].life = false;
		if (particleContainer[i].life == true) {
			particleContainer[i].xmin = particleContainer[i].pos.x + min(0.0f, particleContainer[i].speed.x) - 0.5f * particleContainer[i].size;
			particleContainer[i].xmax = particleContainer[i].pos.x + max(0.0f, particleContainer[i].speed.x) + 0.5f * particleContainer[i].size;
			particleContainer[i].ymin = particleContainer[i].pos.y + min(0.0f, particleContainer[i].speed.y) - 0.5f * particleContainer[i].size;
			particleContainer[i].ymax = particleContainer[i].pos.y + max(0.0f, particleContainer[i].speed.y) + 0.5f * particleContainer[i].size;
			particleContainer[i].zmin = particleContainer[i].pos.z + min(0.0f, particleContainer[i].speed.z);
			particleContainer[i].zmax = particleContainer[i].pos.z + max(0.0f, particleContainer[i].speed.z);
			particleContainer[i].pos += particleContainer[i].speed;
			particleContainer[i].distance = glm::length(particleContainer[i].pos - camera);
			particlePosition[numParticle * 4] = particleContainer[i].pos.x;
			particlePosition[numParticle * 4 + 1] = particleContainer[i].pos.y;
			particlePosition[numParticle * 4 + 2] = particleContainer[i].pos.z;
			particlePosition[numParticle * 4 + 3] = particleContainer[i].size;
			particleColor[numParticle * 4] = 251;
			particleColor[numParticle * 4 + 1] = 43;
			particleColor[numParticle * 4 + 2] = 17;
			particleColor[numParticle * 4 + 3] = 255;
			numParticle++;
		}
	}
	sortParticle();
	glBindBuffer(GL_ARRAY_BUFFER, particleV);
	glBufferData(GL_ARRAY_BUFFER, maxParticle * 4 * sizeof(GLfloat), 0, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numParticle * 4 * sizeof(GLfloat), particlePosition);
	glBindBuffer(GL_ARRAY_BUFFER, particleC);
	glBufferData(GL_ARRAY_BUFFER, maxParticle * 4 * sizeof(GLubyte), 0, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, numParticle * 4 * sizeof(GLubyte), particleColor);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, particleVbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, particleV);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, particleC);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, numParticle);

	for (int i = 0; i < maxAlien; i++) {
		for (int j = 0; j < maxParticle; j++) {
			if (alienContainer[i].life == true && particleContainer[j].life == true && collision(alienContainer[i], particleContainer[j]) == true) {
				alienContainer[i].life = false;
				particleContainer[j].life = false;
			}
		}
	}

	glui->sync_live();
	glFlush();
	glutPostRedisplay();
}

void initializedGL(void) {
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	installShaders();
	installInstanceShaders();
	installSkyboxShaders();
	installParticleShaders();
	sendDataToOpenGL();

	for (int i = 0; i < maxParticle; i++) particleContainer[i].life = false;
	for (int i = 0; i < maxAlien; i++) alienContainer[i].life = false;
}

int main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);

	int mainWindowID = glutCreateWindow("Project");

	ISoundEngine* engine = createIrrKlangDevice();

	GLUI_Master.set_glutKeyboardFunc(keyboard);
	GLUI_Master.set_glutSpecialFunc(move);
	GLUI_Master.set_glutMouseFunc(Mouse_Wheel_Func);
	GLUI_Master.set_glutReshapeFunc(reshape);

	initializedGL();
	glutDisplayFunc(paintGL);
	glutReshapeFunc(reshape);

	glutKeyboardFunc(keyboard);
	glutSpecialFunc(move);
	glutPassiveMotionFunc(PassiveMouse);
	glutMouseFunc(Mouse_Wheel_Func);

	glui = GLUI_Master.create_glui_subwindow(mainWindowID, GLUI_SUBWINDOW_RIGHT);
	glui->set_main_gfx_window(mainWindowID);
	glui->add_separator();
	GLUI_StaticText *infoText = glui->add_statictext("i-Navigation@CUHK");
	infoText->set_alignment(GLUI_ALIGN_CENTER);
	glui->add_separator();

	GLUI_Spinner *VS_Spinner = glui->add_spinner("Vehicle_S:", GLUI_SPINNER_FLOAT, &vehicleSpeed);
	VS_Spinner->set_float_limits(0.0001f, 10.0f);
	VS_Spinner->set_speed(0.005f);

	GLUI_Rollout *RC_Rollout = new GLUI_Rollout(glui, "Render Control", true);
	GLUI_Panel *V_Panel = new GLUI_Panel(RC_Rollout, "Viewport");
	GLUI_RadioGroup *V_RG = glui->add_radiogroup_to_panel(V_Panel, &viewpoints);
	glui->add_radiobutton_to_group(V_RG, "Front");
	glui->add_radiobutton_to_group(V_RG, "Left");
	glui->add_radiobutton_to_group(V_RG, "Right");
	glui->add_radiobutton_to_group(V_RG, "Top");
	glui->add_radiobutton_to_group(V_RG, "Vehicle");
	GLUI_Panel *FogSwitch_Panel = new GLUI_Panel(RC_Rollout, "Fog On/Off");
	glui->add_checkbox_to_panel(FogSwitch_Panel, "Fog", &fogOnOff);
	GLUI_Panel *FogColor_Panel = new GLUI_Panel(RC_Rollout, "Fog Color");
	GLUI_RadioGroup *F_RG = glui->add_radiogroup_to_panel(FogColor_Panel, &fogCol);
	glui->add_radiobutton_to_group(F_RG, "Ivory");
	glui->add_radiobutton_to_group(F_RG, "Ocean Blue");

	engine->play2D("background.wav", true);
	glutMainLoop();

	engine->drop();
	return 0;
}
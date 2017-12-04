/*********************************************************
FILE : main.cpp (csci3260 2017-2018 Assignment 2)
*********************************************************/
/*********************************************************
Student Information
Student ID: 1155077843
Student Name: Lo Chun Hei
Reference for particle system: http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
*********************************************************/
#define _CRT_SECURE_NO_WARNINGS
#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include "Dependencies\glm\glm.hpp"
#include "Dependencies\glm\gtc\matrix_transform.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;
using glm::vec3;
using glm::mat4;

GLuint programID, programID2, SkyboxprogramID;
size_t drawSize[5];
// Could define the Vao&Vbo and interaction parameter here
GLuint vaoID0, vaoID1, vaoID2, vaoID3, vaoID4, vaoSkybox, Texture[10];
GLuint oldtime = 0;
GLfloat xangle = 3.14f, yangle = 0.0f;
int d_num = 0, s_num = 0, viewcon = -1, rotz_press_num = 0, roty_press_num = 0, rotz = -1;
int xpos, ypos, xcen, ycen;
float xx = 1.0, lx = 0.0, ly = 0.0, lz = 0.0, carx = 0.0f, carz = 0.0f, carangle = 0.0f, a=0.0f,b=0.0f,c=0.0f;
float ddd = 0.0f, red = 0.0f, yel = 0.0f, gre = 0.0f, rc =0.0f, spe = 0.0f;
int justenter = 0;
glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));

/*#####################################Particle#########################################*/
GLuint billbdvbo, ppVao;
GLuint partposvbo;
GLuint partcolvbo;

struct Particle {
	glm::vec3 pos, speed;
	unsigned char r, g, b, a; // Color
	float size, angle, weight;
	float life;
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};

const int maxnopar = 3000;
Particle ParticlesContainer[maxnopar];
static GLfloat* g_par_position_size_data = new GLfloat[maxnopar * 4];
static GLubyte* g_par_color_data = new GLubyte[maxnopar * 4];

int LastUsedParticle = 0;

// Finds a Particle in ParticlesContainer which isn't used yet.
// (i.e. life < 0);
int FindUnusedParticle() {

	for (int i = LastUsedParticle; i<maxnopar; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = i;
			return i;
		}
	}

	for (int i = 0; i<LastUsedParticle; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = i;
			return i;
		}
	}

	return 0; // All particles are taken, override the first one
}
/*#######################################################Particle############################################*/

bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE)
	{
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

bool checkShaderStatus(GLuint shaderID)
{
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID)
{
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

string readShaderCode(const char* fileName)
{
	ifstream meInput(fileName);
	if (!meInput.good())
	{
		cout << "File failed to load..." << fileName;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

void installShaders()
{
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
	

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	if (!checkProgramStatus(programID))
		return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(programID);
}

void installParticleShaders()
{

	GLuint vertexShaderID2 = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID2 = glCreateShader(GL_FRAGMENT_SHADER);


	const GLchar* adapter[1];
	string temp = readShaderCode("VertexShaderCode2.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID2, 1, adapter, 0);
	temp = readShaderCode("FragmentShaderCode2.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID2, 1, adapter, 0);

	glCompileShader(vertexShaderID2);
	glCompileShader(fragmentShaderID2);

	if (!checkShaderStatus(vertexShaderID2) || !checkShaderStatus(fragmentShaderID2))
		return;

	programID2 = glCreateProgram();
	glAttachShader(programID2, vertexShaderID2);
	glAttachShader(programID2, fragmentShaderID2);
	glLinkProgram(programID2);


	if (!checkProgramStatus(programID2))
		return;

	glDeleteShader(vertexShaderID2);
	glDeleteShader(fragmentShaderID2);

	glUseProgram(programID2);
}

void installSkyboxShaders()
{
	GLuint SkyboxvertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint SkyboxfragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("SkyboxVertexShaderCode2.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(SkyboxvertexShaderID, 1, adapter, 0);
	temp = readShaderCode("SkyboxFragmentShaderCode2.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(SkyboxfragmentShaderID, 1, adapter, 0);


	glCompileShader(SkyboxvertexShaderID);
	glCompileShader(SkyboxfragmentShaderID);


	if (!checkShaderStatus(SkyboxvertexShaderID) || !checkShaderStatus(SkyboxfragmentShaderID))
		return;

	SkyboxprogramID = glCreateProgram();
	glAttachShader(SkyboxprogramID, SkyboxprogramID);
	glAttachShader(SkyboxprogramID, SkyboxfragmentShaderID);
	glLinkProgram(SkyboxprogramID);

	if (!checkProgramStatus(SkyboxprogramID))
		return;

	glDeleteShader(SkyboxvertexShaderID);
	glDeleteShader(SkyboxfragmentShaderID);

	glUseProgram(SkyboxprogramID);
}



void keyboard(unsigned char key, int x, int y)
{
	//TODO: Use keyboard to do interactive events and animation
	if (key == 'q')ddd += 0.1f;
	if (key == 'w')ddd -= 0.1f;
	if (key == 'z')spe += 0.1f;
	if (key == 'x')spe -= 0.1f;
	if (key == ' ') {
		viewcon = viewcon *= -1;
		justenter = 1;
	}
	if (key == 's')rotz *= -1;
	if (key == 'r')red += 0.5f;
	if (key == 't')red -= 0.5f;
	if (key == 'y')yel += 0.5f;
	if (key == 'u')yel -= 0.5f;
	if (key == 'i')gre += 0.5f;
	if (key == 'o')gre -= 0.5f;
}

void move(int key, int x, int y)
{
	//TODO: Use arrow keys to do interactive events and animation
	float carangle = roty_press_num*0.1f;
	if (key == GLUT_KEY_DOWN) {
		carx = carx + 0.3f * cos(carangle);
		carz = carz - 0.3f * sin(carangle);
	}
	if (key == GLUT_KEY_UP) {
		carx = carx - 0.3f * cos(carangle);
		carz = carz + 0.3f * sin(carangle);
	};
	if (key == GLUT_KEY_LEFT)roty_press_num++;
	if (key == GLUT_KEY_RIGHT)roty_press_num--;
}

void PassiveMouse(int x, int y)
{
	//TODO: Use Mouse to do interactive events and animation
	xpos = x;
	ypos = y;
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
	if (imageSize == 0)    imageSize = width*height * 3;
	if (dataPos == 0)      dataPos = 54;

	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);


	GLuint textureID;
	//TODO: Create one OpenGL texture and set the texture parameter 

	//create openGL texture
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[]data;
	//set texture parameter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);


	return textureID;
}

GLuint loadCubemap(vector<const GLchar*> faces) {
	unsigned int width, height;
	const GLchar* imagepath;
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	for (GLuint i = 0; i < faces.size(); i++) {
		imagepath = faces.at(i);
		printf("Reading image %s\n", imagepath);

		unsigned char header[54];
		unsigned int dataPos;
		unsigned int imageSize;
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

void sendDataToOpenGL()
{
	//TODO:
	//Load objects and bind to VAO & VBO
	//Load texture

	/*skybox*/
	GLfloat skyboxVertices[] = {
		-1.0f, +1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		+1.0f, -1.0f, -1.0f,
		+1.0f, -1.0f, -1.0f,
		+1.0f, +1.0f, -1.0f,
		-1.0f, +1.0f, -1.0f,

		-1.0f, -1.0f, +1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, +1.0f, -1.0f,
		-1.0f, +1.0f, -1.0f,
		-1.0f, +1.0f, +1.0f,
		-1.0f, -1.0f, +1.0f,

		+1.0f, -1.0f, -1.0f,
		+1.0f, -1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, -1.0f,
		+1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f, +1.0f,
		-1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		+1.0f, -1.0f, +1.0f,
		-1.0f, -1.0f, +1.0f,

		-1.0f, +1.0f, -1.0f,
		+1.0f, +1.0f, -1.0f,
		+1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		-1.0f, +1.0f, +1.0f,
		-1.0f, +1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, +1.0f,
		+1.0f, -1.0f, -1.0f,
		+1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, +1.0f,
		+1.0f, -1.0f, +1.0f
	};

	GLuint vboSkybox;
	glGenVertexArrays(1, &vaoSkybox);
	glGenBuffers(1, &vboSkybox);
	glBindVertexArray(vaoSkybox);
	glBindBuffer(GL_ARRAY_BUFFER, vboSkybox);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	//drawSize[0] = GLint(sizeof(skyboxVertices));

	std::vector<glm::vec3> vertices[5];
	std::vector<glm::vec2> uvs[5];
	std::vector<glm::vec3> normals[5];
	GLuint vboID0, vboID1, vboID2, vboID3, vboID4;

	bool res = loadOBJ("obj/planet.obj", vertices[0], uvs[0], normals[0]);
	//GLuint vaoID1; (throw out!!)
	glGenVertexArrays(1, &vaoID0);
	glBindVertexArray(vaoID0);  //first VAO
								
	glGenBuffers(1, &vboID0);
	glBindBuffer(GL_ARRAY_BUFFER, vboID0);
	glBufferData(GL_ARRAY_BUFFER, vertices[0].size() * sizeof(glm::vec3), &vertices[0][0], GL_STATIC_DRAW);
	//vertex position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//uv
	//vbo2 uvbuffer
	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs[0].size() * sizeof(glm::vec2), &uvs[0][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	//vbo3 normalcoord
	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals[0].size() * sizeof(glm::vec3), &normals[0][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	drawSize[1] = vertices[0].size();
	Texture[1] = loadBMP_custom("oldtexture/lamp_texture.bmp");


	res = loadOBJ("obj/Car_porsche.obj", vertices[1], uvs[1], normals[1]);
	//res = loadOBJ("plane.obj", vertices[1], uvs[1], normals[1]);
	//GLuint vaoID1; (throw out!!)
	glGenVertexArrays(1, &vaoID1);
	glBindVertexArray(vaoID1);  //first VAO
								//vbo1
	glGenBuffers(1, &vboID1);
	glBindBuffer(GL_ARRAY_BUFFER, vboID1);
	glBufferData(GL_ARRAY_BUFFER, vertices[1].size() * sizeof(glm::vec3), &vertices[1][0], GL_STATIC_DRAW);
	//vertex position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//uv
	//vbo2 uvbuffer
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs[1].size() * sizeof(glm::vec2), &uvs[1][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	//vbo3 normalcoord
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals[1].size() * sizeof(glm::vec3), &normals[1][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//remember the drawSize for paintGL function
	drawSize[2] = vertices[1].size();
	//load texture and remember texture ID for paintGL function
	Texture[2] = loadBMP_custom("oldtexture/car_texture.bmp");


	res = loadOBJ("obj/B-2_Spirit.obj", vertices[2], uvs[2], normals[2]);
	//GLuint vaoID1; (throw out!!)
	glGenVertexArrays(1, &vaoID2);
	glBindVertexArray(vaoID2);  //first VAO
								
	glGenBuffers(1, &vboID2);
	glBindBuffer(GL_ARRAY_BUFFER, vboID2);
	glBufferData(GL_ARRAY_BUFFER, vertices[2].size() * sizeof(glm::vec3), &vertices[2][0], GL_STATIC_DRAW);
	//vertex position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//uv
	//vbo2 uvbuffer
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs[2].size() * sizeof(glm::vec2), &uvs[2][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	//vbo3 normalcoord
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals[2].size() * sizeof(glm::vec3), &normals[2][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//remember the drawSize for paintGL function
	drawSize[3] = vertices[2].size();
	//load texture and remember texture ID for paintGL function
	Texture[3] = loadBMP_custom("oldtexture/moon.bmp");


	res = loadOBJ("obj/Arc170.obj", vertices[3], uvs[3], normals[3]);
	//GLuint vaoID1; (throw out!!)
	glGenVertexArrays(1, &vaoID3);
	glBindVertexArray(vaoID3);  //first VAO
								
	glGenBuffers(1, &vboID3);
	glBindBuffer(GL_ARRAY_BUFFER,vboID3);
	glBufferData(GL_ARRAY_BUFFER, vertices[3].size() * sizeof(glm::vec3), &vertices[3][0], GL_STATIC_DRAW);
	//vertex position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//uv
	//vbo2 uvbuffer
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs[3].size() * sizeof(glm::vec2), &uvs[3][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	//vbo3 normalcoord
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals[3].size() * sizeof(glm::vec3), &normals[3][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//remember the drawSize for paintGL function
	drawSize[4] = vertices[3].size();
	//load texture and remember texture ID for paintGL function
	Texture[4] = loadBMP_custom("oldtexture/plane_texture.bmp");

	res = loadOBJ("obj/starfy.obj", vertices[4], uvs[4], normals[4]);
	//GLuint vaoID1; (throw out!!)
	glGenVertexArrays(1, &vaoID4);
	glBindVertexArray(vaoID4);  //first VAO

	glGenBuffers(1, &vboID4);
	glBindBuffer(GL_ARRAY_BUFFER, vboID4);
	glBufferData(GL_ARRAY_BUFFER, vertices[4].size() * sizeof(glm::vec3), &vertices[4][0], GL_STATIC_DRAW);
	//vertex position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//uv
	//vbo2 uvbuffer
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs[4].size() * sizeof(glm::vec2), &uvs[4][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	//vbo3 normalcoord
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals[4].size() * sizeof(glm::vec3), &normals[4][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//remember the drawSize for paintGL function
	drawSize[5] = vertices[4].size();
	//load texture and remember texture ID for paintGL function
	Texture[5] = loadBMP_custom("oldtexture/slamp_texture.bmp");

/*#####################################Particle##############################################*/
	glGenVertexArrays(1, &ppVao);
	glBindVertexArray(ppVao);
	static const GLfloat g_vertex_buffer_data[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
	};
	glGenBuffers(1, &billbdvbo);
	glBindBuffer(GL_ARRAY_BUFFER, billbdvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// The VBO containing the positions and sizes of the particles
	glGenBuffers(1, &partposvbo);
	glBindBuffer(GL_ARRAY_BUFFER, partposvbo);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, maxnopar * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	glGenBuffers(1, &partcolvbo);
	glBindBuffer(GL_ARRAY_BUFFER, partcolvbo);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, maxnopar * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	
	vector<const GLchar*> Skybox_faces;
	Skybox_faces.push_back("skybox/right.bmp");
	Skybox_faces.push_back("skybox/left.bmp");
	Skybox_faces.push_back("skybox/top.bmp");
	Skybox_faces.push_back("skybox/bottom.bmp");
	Skybox_faces.push_back("skybox/back.bmp");
	Skybox_faces.push_back("skybox/front.bmp");
	Texture[0] = loadCubemap(Skybox_faces);

}

void SortParticles() {
	std::sort(&ParticlesContainer[0], &ParticlesContainer[maxnopar]);
}

void paintGL(void)
{
/*#######################################################Particle###############################################*/

	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	GLint newtime, deltatime;
	newtime = glutGet(GLUT_ELAPSED_TIME);
	deltatime = newtime - oldtime;
	double delta = deltatime / 1000.0;
	//mouse rotate camera (dx and dy takes value from passive mouse function)
	if (viewcon == 1) {
		xcen = vp[2] / 2;
		ycen = vp[3] / 2;
		if (justenter == 1) {
			justenter = 0;
			glutWarpPointer(xcen, ycen);
			oldtime = glutGet(GLUT_ELAPSED_TIME);
		}
		else {
			xangle = xangle + 0.00005f * deltatime * GLfloat((xcen - xpos));
			yangle = yangle + 0.00005f * deltatime * GLfloat((ycen - ypos));
			glm::vec3 direction = glm::vec3(cos(yangle) * sin(xangle), sin(yangle), cos(yangle) * cos(xangle));
			glm::vec3 right = glm::vec3(sin(xangle - 3.14f / 2.0f), 0, cos(xangle - 3.14f / 2.0f));
			glm::vec3 up = glm::cross(right, direction);
			viewMatrix = glm::lookAt(glm::vec3(lx, ly, lz), direction, up);
			glutWarpPointer(xcen, ycen);
		}

	}
	oldtime = newtime;
	if (rotz == 1)rotz_press_num++;

	//TODO:
	//Set lighting information, such as position and color of lighting source
	//Set transformation matrix
	//Bind different textures
	glClearColor(0.15f, 0.1f, 0.1f, 1.0f); //specify the background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glUseProgram(SkyboxprogramID);

	GLuint Skb_ModelUniformLocation = glGetUniformLocation(SkyboxprogramID, "M");
	glm::mat4 Skb_ModelMatrix = glm::mat4(1.0f);
	//remove any translation component of the view matrix
	glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
	glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenWidth, 0.1f, 100.0f);

	glUniformMatrix4fv(Skb_ModelUniformLocation, 1, GL_FALSE, &Skb_ModelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(SkyboxprogramID, "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(SkyboxprogramID, "projection"), 1, GL_FALSE, &projection[0][0]);
	
	//skybox cube
	glBindVertexArray(vaoSkybox);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(SkyboxprogramID, "skybox"), 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Texture[0]);

	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);


	glUseProgram(programID);

	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
	glm::mat4 rotationMatrix = glm::mat4(1.0f);
	glm::mat4 Matrix = glm::mat4(1.0f);
	glm::mat4 projectionMatrix = glm::perspective(1.0f, (float)vp[2] / vp[3], 5.0f, 100.0f);
	glm::vec4 ambientLight1(0.05f, 0.05f, 0.05f, 1.0f);
	glm::vec3 lightPosition1(0.0f, 30.0f, -10.0f);
	glm::vec3 lightPosition2(5.5f, 3.0f, -30.5f);
	glm::vec3 lightPositionr(-4.0, 10.0f, -22.0f);
	//printf("%.3f %.3f %.3f\n", a,b,c);
	glm::vec3 lightPositiony(-4.0, 9.7f, -22.0f);
	glm::vec3 lightPositiong(-4.0, 9.4f, -22.0f);
	glm::vec3 eyePosition(0.0f, 0.0f, 0.0f);

	GLint modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
	GLint projectionMatrixUniformLocation = glGetUniformLocation(programID, "projectionMatrix");
	GLint rotationMatrixUniformLocation = glGetUniformLocation(programID, "rotationMatrix");
	GLint viewMatrixUniformLocation = glGetUniformLocation(programID, "viewMatrix");
	GLuint ambientLightUniformLocation = glGetUniformLocation(programID, "ambientLight");
	GLuint eyePositionUniformLocation = glGetUniformLocation(programID, "eyePositionWorld");
	GLuint lightPosition1UniformLocation = glGetUniformLocation(programID, "lightPositionWorld1");
	GLuint lightPosition2UniformLocation = glGetUniformLocation(programID, "lightPositionWorld2");
	GLuint lightPositionrUniformLocation = glGetUniformLocation(programID, "lightPositionWorldr");
	GLuint lightPositionyUniformLocation = glGetUniformLocation(programID, "lightPositionWorldy");
	GLuint lightPositiongUniformLocation = glGetUniformLocation(programID, "lightPositionWorldg");
	GLuint textureID = glGetUniformLocation(programID, "myTextureSampler");
	GLuint dd1 = glGetUniformLocation(programID, "difdelta1");
	GLuint ddr = glGetUniformLocation(programID, "difdeltar");
	GLuint ddy = glGetUniformLocation(programID, "difdeltay");
	GLuint ddg = glGetUniformLocation(programID, "difdeltag");
	GLuint sd = glGetUniformLocation(programID, "spedelta");


	glUniformMatrix4fv(viewMatrixUniformLocation, 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniform3fv(ambientLightUniformLocation, 1, &ambientLight1[0]);
	glUniform3fv(lightPosition1UniformLocation, 1, &lightPosition1[0]);
	glUniform3fv(lightPosition2UniformLocation, 1, &lightPosition2[0]);
	glUniform3fv(lightPositionrUniformLocation, 1, &lightPositionr[0]);
	glUniform3fv(lightPositionyUniformLocation, 1, &lightPositiony[0]);
	glUniform3fv(lightPositiongUniformLocation, 1, &lightPositiong[0]);
	if (ddd > 0.5)ddd = 0.5;
	if (ddd < -0.0)ddd = 0.0;
	if (red > 0.5)red = 0.5;
	if (red < -0.0)red = 0.0;
	if (yel > 0.5)yel = 0.5;
	if (yel < -0.0)yel = 0.0;
	if (gre > 0.5)gre = 0.5;
	if (gre < -0.0)gre = 0.0;
	glUniform1f(dd1, ddd);
	glUniform1f(ddr, red);
	glUniform1f(ddy, yel);
	glUniform1f(ddg, gre);
	glUniform1f(sd, spe);

	//lamp
	glBindVertexArray(vaoID0);


	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(-15.0, -7.0f, -34.0f));

	modelTransformMatrix = glm::rotate(modelTransformMatrix, 0.0f, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture[1]);
	glUniform1i(textureID, 0);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[1]);

	//
	//car
	glBindVertexArray(vaoID1);
	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(7.0f, -8.0f, -30.0f));
	modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(carx, 0.0f, carz));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, roty_press_num*0.1f, glm::vec3(0, 1, 0));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, 4.712f, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, Texture[2]);
	glUniform1i(textureID, 1);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[2]);

	//moon
	glBindVertexArray(vaoID2);
	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(20.0f, 30.0f, -80.0f));
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.05f, 0.05f, 0.05f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, rotz_press_num*0.01f, glm::vec3(0, 0, 1));
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, Texture[3]);
	glUniform1i(textureID, 2);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[3]);


	//plane
	glBindVertexArray(vaoID3);
	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, -10.0f, -27.0f));
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(4.0f, 4.0f, 4.0f));
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, Texture[4]);
	glUniform1i(textureID, 3);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[4]);


	//slamp
	glBindVertexArray(vaoID4);
	modelTransformMatrix = glm::translate(glm::mat4(), glm::vec3(10.0f, -8.0f, -40.0f));
	modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.8f, 1.2f, 1.0f));
	modelTransformMatrix = glm::rotate(modelTransformMatrix, 4.712f, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, Texture[5]);
	glUniform1i(textureID, 4);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[5]);

/*#######################################################Particle###############################################*/

	glUseProgram(programID2);
	viewMatrixUniformLocation = glGetUniformLocation(programID2, "viewMatrix");
	glUniformMatrix4fv(viewMatrixUniformLocation, 1, GL_FALSE, &viewMatrix[0][0]);
	projectionMatrixUniformLocation = glGetUniformLocation(programID2, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixUniformLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	glBindVertexArray(ppVao);
	modelTransformMatrix = glm::mat4(1.0f);
	modelTransformMatrixUniformLocation = glGetUniformLocation(programID2, "modelTransformMatrix");
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);

	int newparticles = (int)(delta*3000.0);
	if (newparticles > (int)(0.005f*3000.0))newparticles = (int)(0.005f*3000.0);

	for (int i = 0; i<newparticles; i++) {
		int particleIndex = FindUnusedParticle();
		ParticlesContainer[particleIndex].life = 10.0f; 
		ParticlesContainer[particleIndex].pos = glm::vec3((rand() % 2000 - 1000) / 50.0f, 10, -30.0f + (rand() % 2000 - 1000.0f) / 50.0f);

		float spread = 0.0f;
		glm::vec3 maindir = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 randomdir = glm::vec3(
			(rand() % 2000 - 1000.0f) / 1000.0f,
			(rand() % 2000 - 1000.0f) / 1200.0f,
			(rand() % 2000 - 1000.0f) / 1000.0f
		);

		ParticlesContainer[particleIndex].speed = maindir + randomdir*spread;

		ParticlesContainer[particleIndex].r = 255;
		ParticlesContainer[particleIndex].g = 255;
		ParticlesContainer[particleIndex].b = 255;
		ParticlesContainer[particleIndex].a = 255;

		ParticlesContainer[particleIndex].size = (rand() % 2000 - 1000.0f) / 10000.0f + 0.001f;

	}

	// Simulate all particles
	int parcnt = 0;
	for (int i = 0; i<maxnopar; i++) {

		Particle& p = ParticlesContainer[i]; // shortcut

		if (p.life > 0.0f) {

			// Decrease life
			p.life -= delta;
			if (p.life > 0.0f) {

				// Simulate simple physics : gravity only, no collisions
				p.speed += glm::vec3(0.0f, -1.5f, 0.0f) * (float)delta * 0.5f;
				p.pos += p.speed * (float)delta;
				p.cameradistance = glm::length(p.pos - eyePosition);
				//ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

				// Fill the GPU buffer
				g_par_position_size_data[4 * parcnt + 0] = p.pos.x + (rand() % 2000 - 1000.0f) / 100000.0f;
				g_par_position_size_data[4 * parcnt + 1] = p.pos.y;
				g_par_position_size_data[4 * parcnt + 2] = p.pos.z + (rand() % 2000 - 1000.0f) / 100000.0f;
				g_par_position_size_data[4 * parcnt + 3] = p.size;
				g_par_color_data[4 * parcnt + 0] = p.r;
				g_par_color_data[4 * parcnt + 1] = p.g;
				g_par_color_data[4 * parcnt + 2] = p.b;
				g_par_color_data[4 * parcnt + 3] = p.a;

			}
			else {
				// Particles that just died will be put at the end of the buffer in SortParticles();
				p.cameradistance = -1.0f;
			}

			parcnt++;

		}
	}

	SortParticles();


	glBindBuffer(GL_ARRAY_BUFFER, partposvbo);
	glBufferData(GL_ARRAY_BUFFER, maxnopar * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, parcnt * sizeof(GLfloat) * 4, g_par_position_size_data);

	glBindBuffer(GL_ARRAY_BUFFER, partcolvbo);
	glBufferData(GL_ARRAY_BUFFER, maxnopar * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, parcnt * sizeof(GLubyte) * 4, g_par_color_data);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, billbdvbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);// array buffer offset	);

	// 2nd attribute buffer : positions of particles' centers
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, partposvbo);
	glVertexAttribPointer(	1, 	4, 	GL_FLOAT, 	GL_FALSE,	0, 	(void*)0 );

	// 3rd attribute buffer : particles' colors
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, partcolvbo);
	glVertexAttribPointer(	2, 	4, 	GL_UNSIGNED_BYTE, 	GL_TRUE, 	0, 	(void*)0 );

	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(1, 1); // positions : one per quad (its center) -> 1
	glVertexAttribDivisor(2, 1); // color : one per quad -> 1


								 // This is equivalent to :
								 // for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4),
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, parcnt);

/*###########################################################Particle####################################################*/

	glFlush();
	glutPostRedisplay();
}

void initializedGL(void)
{
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	installShaders();
	installParticleShaders();
	installSkyboxShaders();
	sendDataToOpenGL();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	//TODO:
	//Create a window with title specified
	glutCreateWindow("Assignment 2");
	glewInit();

/*##########################Particle###############################*/
	for (int i = 0; i < maxnopar; i++) {
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
	}
/*##########################Particle###############################*/

	initializedGL();
	glutDisplayFunc(paintGL);

	glutKeyboardFunc(keyboard);
	glutSpecialFunc(move);
	glutPassiveMotionFunc(PassiveMouse);

	glutMainLoop();

	return 0;
}
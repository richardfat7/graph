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
//#include "Dependencies\irrKlang\irrKlang.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

#define MAXOBJ				6
#define MAXTEXTURE			10
#define MAXDRAWNOBJ			6
#define MAXPP				5000

#define VAO_PLANET			0
#define VAO_ROCK			1
#define VAO_PLANE			2
#define VAO_STAR			3
#define VAO_LIGHTBOX		4
#define VAO_PARTICAL		5

#define TXT_NULL			999
#define TXT_EARTH			0
#define TXT_NORM_EARTH		1
#define TXT_MOON			2
#define TXT_SUN_SUN			3
#define TXT_SUN_JUPITER		4
#define TXT_ROCK			5
#define TXT_LIGHTBOX		6
#define TXT_HELICOPTER		7
#define TXT_STARFY			8
#define TXT_PARTICAL		9

#define DRAWN_SUN			0
#define DRAWN_MOON			1
#define DRAWN_EARTH			2
#define DRAWN_PLANE			3
#define DRAWN_LIGHTBOX		4
#define DRAWN_STAR			5

typedef enum txtMode { NOTXT, SIGTXT, MULTXT, NORMTXT } TXTMode;

class DrawnObj {
	public:
		GLuint vao;
		GLuint texture;
		GLuint texture2;
		GLuint norm_texture;
		TXTMode mode;
		bool isLightSource;
		mat4 modelTransformMatrix;
		void setContent(GLuint aVao, GLuint aTexture, GLuint aTexture2, GLuint aNorm_texture, TXTMode aMode, bool isLightSource);
};

int objList[MAXOBJ] = {
	VAO_PLANET,
	VAO_ROCK,
	VAO_PLANE,
	VAO_STAR,
	VAO_LIGHTBOX,
	VAO_PARTICAL
};
char* objResList[MAXOBJ] = { 
	"obj/planet.obj", 
	"obj/rock.obj",
	"obj/Arc170.obj", 
	"obj/starfy.obj", 
	"obj/lightbox2.obj" ,
	"obj/rock.obj"
};
int txtList[MAXTEXTURE] = {
	TXT_EARTH,
	TXT_NORM_EARTH,
	TXT_MOON, 
	TXT_SUN_SUN, 
	TXT_SUN_JUPITER,
	TXT_ROCK,
	TXT_LIGHTBOX,
	TXT_HELICOPTER,
	TXT_STARFY,
	TXT_PARTICAL
};
char* txtResList[MAXTEXTURE] = {
	"texture/earth.bmp", 
	"normal_map/earth_normal.bmp", 
	"oldtexture/moon.bmp", 
	"texture/sun.bmp", 
	"texture/jupiter.bmp", 
	"texture/helicopter.bmp",
	"texture/lightbox.bmp",
	"texture/helicopter.bmp", 
	"texture/starfy.bmp",
	"texture/rock.bmp"
};
int drawnListIndex[MAXDRAWNOBJ] = {
	DRAWN_SUN,
	DRAWN_MOON,
	DRAWN_EARTH, 
	DRAWN_PLANE, 
	DRAWN_LIGHTBOX,
	DRAWN_STAR
};
DrawnObj drawnList[MAXDRAWNOBJ];

GLuint commonProgram, instanceProgram, skyboxProgram;
GLint modelTransformMatrixUniformLocation, projectionMatrixUniformLocation, rotationMatrixUniformLocation, viewMatrixUniformLocation,
	ambientLightUniformLocation, eyePositionUniformLocation, lightPosition1UniformLocation, lightPosition2UniformLocation,
	lightPositionrUniformLocation, lightPositionyUniformLocation, lightPositiongUniformLocation,
	textureID, textureID2,
	dd1, ddr, ddy, ddg, sd,
	normalMapping_flagUniiformLocation, multiMapping_flagUniiformLocation,
	fog_flagUniiformLocation, fog_ColorUniiformLocation, sunUniformLocation,
	i_textureID,
	i_ambientLightUniformLocation, i_eyePositionUniformLocation, i_lightPosition1UniformLocation, i_lightPosition2UniformLocation,
	i_dd1, i_sd;

size_t drawSize[MAXOBJ + 1];
// Could define the Vao&Vbo and interaction parameter here
GLuint vao[MAXOBJ], vaoSkybox, texture[MAXTEXTURE + 1];
GLuint ppvbo;
GLuint oldtime = 0;
GLfloat xangle = 3.14f, yangle = 0.0f;
int d_num = 0, s_num = 0, viewcon = -1, roty_press_num = 0, rotz = -1, planeview = -1;
float rotz_press_num = 0.0f;
int xpos, ypos, xcen, ycen;
float xx = 1.0, lx = 0.0f, ly = 3.0f, lz = 10.0f, carx = 0.0f, carz = 0.0f, carangle = 0.0f, a=0.0f,b=0.0f,c=0.0f;
float lightboxx = 0.0, lightboxy = 0.0, lightboxz = 0.0;
float ddd = 0.0f, red = 0.0f, yel = 0.0f, gre = 0.0f, rc =0.0f, spe = 0.0f;
float planeradius=5.0f, planerot = 1.0f, planespeed = 0.1f;
float fov = 1.2f;
int justenter = 0;
bool fog_flag = false;

mat4 viewMatrix = glm::lookAt(glm::vec3(lx, ly, lz), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
mat4 viewMatrix2 = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));


vec3 fog_Color = vec3(0.0f, 0.5f, 1.0f);


struct Particle {
	vec3 pos;
	float size, angle, w, selfRotAngle, selfRotW, radius;
	float life;
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};

const int maxnopar = MAXPP;
Particle ParticlesContainer[maxnopar];
static mat4* g_par_position_size_data = new mat4[maxnopar];

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

int installShaders(char* vertexShader, char* fragmentShader)
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode(vertexShader);
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode(fragmentShader);
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID)) {
		printf("Cannot create program with %s, %s\n", vertexShader, fragmentShader);
		return -1;
	}

	int program = glCreateProgram();
	glAttachShader(program, vertexShaderID);
	glAttachShader(program, fragmentShaderID);
	glLinkProgram(program);

	if (!checkProgramStatus(program)) {
		printf("Cannot create program with %s, %s\n", vertexShader, fragmentShader);
		return -1;
	}

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	return program;
}

void keyboard(unsigned char key, int x, int y)
{
	//TODO: Use keyboard to do interactive events and animation
	if (key == 'q')ddd += 0.1f;
	if (key == 'w')ddd -= 0.1f;
	if (key == 'z')spe += 0.1f;
	if (key == 'x')spe -= 0.1f;
	if (key == 'a') {
		viewcon = -1;
		planeview = 0;
		lx = 0.0f;
		ly = 3.0f;
		lz = 10.0f;
		xangle = 3.14f, yangle = 0.0f;
	}
	if (key == 's') {
		viewcon = -1;
		planeview = 0;
		lx = 0.0f;
		ly = 3.0f;
		lz = -30.0f;
		xangle = 1.72f, yangle = 0.0f;
	}
	if (key == 'd') {
		viewcon = -1;
		planeview = 0;
		lx = 0.0f;
		ly = 20.0f;
		lz = -20.0f;
		xangle = 0.0f, yangle = -1.72f;
	}
	if (key == 'f') {
		viewcon = -1;
		planeview = 1;
		//lx = 0.0f;
		//ly = 20.0f;
		//lz = -20.0f;
		//xangle = 0.0f, yangle = -1.72f;
	}
	if (key == ' ') {
		viewcon = viewcon * -1;
		justenter = 1;
	}
	if (key == 'r')rotz *= -1;
	//if (key == 'r')red += 0.5f;
	//if (key == 't')red -= 0.5f;
	//if (key == 'y')yel += 0.5f;
	//if (key == 'u')yel -= 0.5f;
	if (key == 'i')gre += 0.5f;
	if (key == 'o')gre -= 0.5f;
}

void move(int key, int x, int y)
{
	//TODO: Use arrow keys to do interactive events and animation
	//float carangle = roty_press_num*0.1f;
	if (key == GLUT_KEY_DOWN) {
		//carx = carx + 0.3f * cos(carangle);
		//carz = carz - 0.3f * sin(carangle);
		planespeed -= 0.01f;
	}
	if (key == GLUT_KEY_UP) {
		//carx = carx - 0.3f * cos(carangle);
		//carz = carz + 0.3f * sin(carangle);
		planespeed += 0.01f;
	};
	if (key == GLUT_KEY_LEFT)planeradius -= 0.1f;//roty_press_num++;
	if (key == GLUT_KEY_RIGHT)planeradius += 0.1f;//roty_press_num--;
}

void PassiveMouse(int x, int y)
{
	//TODO: Use Mouse to do interactive events and animation
	xpos = x;
	ypos = y;
}

void MouseWheel(int button, int state, int x, int y) {
	if (button == 3 || button == 4) {
		if (state == GLUT_DOWN) {
			if (button == 3) fov = max(0.1f, fov - 0.05f);
			else fov = min(2.5f, fov + 0.05f);
		}
	}
}


bool loadOBJ(
	const char * path,
	std::vector<vec3> & out_vertices,
	std::vector<vec2> & out_uvs,
	std::vector<vec3> & out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<vec3> temp_vertices;
	std::vector<vec2> temp_uvs;
	std::vector<vec3> temp_normals;


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
			vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			vec3 normal;
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
		vec3 vertex = temp_vertices[vertexIndex - 1];
		vec2 uv = temp_uvs[uvIndex - 1];
		vec3 normal = temp_normals[normalIndex - 1];

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
	glActiveTexture(GL_TEXTURE10);
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
	//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return textureID;
}

void sendDataToOpenGL()
{
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
	drawSize[MAXOBJ] = GLint(sizeof(skyboxVertices));
	vector<const GLchar*> Skybox_faces;
	Skybox_faces.push_back("texture/universe_skybox/right.bmp");
	Skybox_faces.push_back("texture/universe_skybox/left.bmp");
	Skybox_faces.push_back("texture/universe_skybox/top.bmp");
	Skybox_faces.push_back("texture/universe_skybox/bottom.bmp");
	Skybox_faces.push_back("texture/universe_skybox/back.bmp");
	Skybox_faces.push_back("texture/universe_skybox/front.bmp");
	texture[10] = loadCubemap(Skybox_faces);

	std::vector<vec3> vertices[5];
	std::vector<vec2> uvs[5];
	std::vector<vec3> normals[5];
	GLuint vboID0, vboID1, vboID2, vboID3, vboID4;	

	int i;

	glGenVertexArrays(MAXOBJ, vao);
	for (i = 0; i < sizeof(objList) / sizeof(objList[0]); i++) {
		std::vector<vec3> vertices;
		std::vector<vec2> uvs;
		std::vector<vec3> normals;
		GLuint vertexbuffer, uvbuffer, normalbuffer;
		int index = objList[i];

		bool res = loadOBJ(objResList[index], vertices, uvs, normals);
		//vao
		glBindVertexArray(vao[index]);

		//vbo1 vertex_position
		glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//vbo2 uvbuffer
		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		//vbo3 normalcoord
		glGenBuffers(1, &normalbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), &normals[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		drawSize[index] = vertices.size();
	}

	for (i = 0; i < sizeof(txtList) / sizeof(txtList[0]); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		texture[i] = loadBMP_custom(txtResList[i]);
		glBindTexture(GL_TEXTURE_2D, texture[i]);
	}


/*#####################################Particle#####EXTRA####################################*/
	//GLuint modelmatrixvbo;
	glBindVertexArray(vao[VAO_PARTICAL]);
	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 0);
	glVertexAttribDivisor(2, 0);
	glGenBuffers(1, &ppvbo);
	glBindBuffer(GL_ARRAY_BUFFER, ppvbo);
	glBufferData(GL_ARRAY_BUFFER, maxnopar * sizeof(mat4), NULL, GL_STREAM_DRAW);
	for (i = 0; i < 4; i++) {
		glEnableVertexAttribArray(i + 3);
		glVertexAttribPointer(i + 3, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)(i * sizeof(vec4)));
		glVertexAttribDivisor(i + 3, 1);
	}
	
}

void getAllUniformLocation() {
	modelTransformMatrixUniformLocation = glGetUniformLocation(commonProgram, "modelTransformMatrix");
	projectionMatrixUniformLocation = glGetUniformLocation(commonProgram, "projectionMatrix");
	rotationMatrixUniformLocation = glGetUniformLocation(commonProgram, "rotationMatrix");
	viewMatrixUniformLocation = glGetUniformLocation(commonProgram, "viewMatrix");
	ambientLightUniformLocation = glGetUniformLocation(commonProgram, "ambientLight");
	eyePositionUniformLocation = glGetUniformLocation(commonProgram, "eyePositionWorld");
	lightPosition1UniformLocation = glGetUniformLocation(commonProgram, "lightPositionWorld1");
	lightPosition2UniformLocation = glGetUniformLocation(commonProgram, "lightPositionWorld2");
	lightPositionrUniformLocation = glGetUniformLocation(commonProgram, "lightPositionWorldr");
	lightPositionyUniformLocation = glGetUniformLocation(commonProgram, "lightPositionWorldy");
	lightPositiongUniformLocation = glGetUniformLocation(commonProgram, "lightPositionWorldg");
	textureID = glGetUniformLocation(commonProgram, "myTextureSampler");
	textureID2 = glGetUniformLocation(commonProgram, "myTextureSampler2");
	dd1 = glGetUniformLocation(commonProgram, "difdelta1");
	ddr = glGetUniformLocation(commonProgram, "difdeltar");
	ddy = glGetUniformLocation(commonProgram, "difdeltay");
	ddg = glGetUniformLocation(commonProgram, "difdeltag");
	sd = glGetUniformLocation(commonProgram, "spedelta");

	normalMapping_flagUniiformLocation = glGetUniformLocation(commonProgram, "normalMapping_flag");
	multiMapping_flagUniiformLocation = glGetUniformLocation(commonProgram, "multiMapping_flag");
	fog_flagUniiformLocation = glGetUniformLocation(commonProgram, "fog_flag");
	fog_ColorUniiformLocation = glGetUniformLocation(commonProgram, "fog_Color");
	sunUniformLocation = glGetUniformLocation(commonProgram, "sun");


	i_ambientLightUniformLocation = glGetUniformLocation(instanceProgram, "ambientLight");
	i_eyePositionUniformLocation = glGetUniformLocation(instanceProgram, "eyePositionWorld");
	i_lightPosition1UniformLocation = glGetUniformLocation(instanceProgram, "lightPositionWorld1");
	i_lightPosition2UniformLocation = glGetUniformLocation(instanceProgram, "lightPositionWorld2");
	i_dd1 = glGetUniformLocation(instanceProgram, "difdelta1");
	i_sd = glGetUniformLocation(instanceProgram, "spedelta");
	i_textureID = glGetUniformLocation(instanceProgram, "myTextureSampler");
}

void updateModelTransformMatrix() {
	mat4 m;
	m = mat4(1.0f);
	m = glm::translate(m, vec3(20.0, 0.0f, -35.0f));
	m = glm::scale(m, vec3(3.0f, 3.0f, 3.0f));
	m = glm::rotate(m, rotz_press_num*0.001f, vec3(0.1, 1, 0));
	m = glm::translate(m, vec3(-1.0f, 0.0f, 0.0f));
	drawnList[DRAWN_SUN].modelTransformMatrix = m;
	m = mat4(1.0f);
	m = glm::translate(m, vec3(-25.0, -5.0f, -38.0f));
	m = glm::scale(m, vec3(0.8f, 0.8f, 0.8f));
	m = glm::rotate(m, rotz_press_num*0.02f, vec3(-0.3, 1, 0));
	m = glm::translate(m, vec3(25.0f, 3.0f, 0.0f));
	drawnList[DRAWN_MOON].modelTransformMatrix = m;
	m = mat4(1.0f);
	m = glm::translate(m, vec3(-15.0, -5.0f, -30.0f));
	m = glm::scale(m, vec3(1.3f, 1.3f, 1.3f));
	m = glm::rotate(m, 0.5f, vec3(0, 0, 1));
	m = glm::rotate(m, -1.72f, vec3(1, 0, 0));
	m = glm::rotate(m, rotz_press_num*0.005f, vec3(0, 0, 1));
	//m = glm::translate(m, vec3(-1.0f, 0.0f, 0.0f));
	drawnList[DRAWN_EARTH].modelTransformMatrix = m;
	m = mat4(1.0f);
	m = glm::translate(m, vec3(0.0f, 0.0f, -10.0f));
	m = glm::translate(m, vec3(lightboxx, lightboxy, lightboxz));
	m = glm::scale(m, vec3(3.0f, 3.0f, 3.0f));
	drawnList[DRAWN_LIGHTBOX].modelTransformMatrix = m;
	m = mat4(1.0f);
	m = glm::translate(m, vec3(-15.0f, -5.0f, -32.0f));
	m = glm::rotate(m, planerot, vec3(1, 0, 0));
	m = glm::translate(m, vec3(0.0f, planeradius, 0.0f));
	m = glm::scale(m, vec3(0.001f, 0.001f, 0.001f));
	drawnList[DRAWN_PLANE].modelTransformMatrix = m;
	m = mat4(1.0f);
	m = glm::translate(m, vec3(15.0f, -8.0f, -44.0f));
	m = glm::scale(m, vec3(0.8f, 1.2f, 1.0f));
	m = glm::rotate(m, 4.712f, vec3(0, 1, 0));
	drawnList[DRAWN_STAR].modelTransformMatrix = m;
}

void computeAllDrawnObj() {
	drawnList[DRAWN_SUN].setContent(VAO_PLANET, TXT_SUN_SUN, TXT_SUN_JUPITER, TXT_NULL, MULTXT, TRUE);
	drawnList[DRAWN_MOON].setContent(VAO_PLANET, TXT_MOON, TXT_NULL, TXT_NULL, SIGTXT, FALSE);
	drawnList[DRAWN_EARTH].setContent(VAO_PLANET, TXT_EARTH, TXT_NULL, TXT_NORM_EARTH, NORMTXT, FALSE);
	drawnList[DRAWN_PLANE].setContent(VAO_PLANE, TXT_HELICOPTER, TXT_NULL, TXT_NULL, SIGTXT, FALSE);
	drawnList[DRAWN_LIGHTBOX].setContent(VAO_LIGHTBOX, TXT_LIGHTBOX, TXT_NULL, TXT_NULL, SIGTXT, TRUE);
	drawnList[DRAWN_STAR].setContent(VAO_STAR, TXT_STARFY, TXT_NULL, TXT_NULL, SIGTXT, FALSE);
	updateModelTransformMatrix();
}

void SortParticles() {
	std::sort(&ParticlesContainer[0], &ParticlesContainer[maxnopar]);
}

void paintGL(void)
{
	updateModelTransformMatrix();
/*#######################################################Particle###############################################*/

	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	GLint newtime, deltatime;
	newtime = glutGet(GLUT_ELAPSED_TIME);
	deltatime = newtime - oldtime;
	double delta = deltatime / 1000.0;
	vec3 planepos;
	//mouse rotate camera (dx and dy takes value from passive mouse function)
	if (planeview == 1) {
		mat4 m = mat4(1.0f);
		m = glm::translate(m, vec3(-15.0f, -5.0f, -32.0f));
		m = glm::rotate(m, planerot, vec3(1, 0, 0));
		m = glm::translate(m, vec3(0.0f, planeradius+1.0f, -1.0f));
		vec4 planepostmp = m * vec4(0.0f, 0.0f, 0.0f, 1.0f);
		planepos = vec3(planepostmp);
	}
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
			vec3 direction = vec3(cos(yangle) * sin(xangle), sin(yangle), cos(yangle) * cos(xangle));
			vec3 right1 = vec3(sin(xangle - 3.14f / 2.0f), 0, cos(xangle - 3.14f / 2.0f));
			vec3 up = glm::cross(right1, direction);
			viewMatrix = glm::lookAt(vec3(lx, ly, lz), vec3(lx, ly, lz) + direction, up);
			if (planeview == 1) {
				vec3 upp = glm::normalize(planepos - vec3(-15.0f, -5.0f, -32.0f));
				vec3 directionp = glm::cross(vec3(planerot, 0.0f, 0.0f), up);
				viewMatrix = glm::lookAt(planepos, planepos + direction + directionp, up + upp);
			}
			//viewMatrix2 = glm::lookAt(glm::vec3(0, 0, 0), direction, up);
			glutWarpPointer(xcen, ycen);
		}

	}
	//
	else {
		vec3 direction = vec3(cos(yangle) * sin(xangle), sin(yangle), cos(yangle) * cos(xangle));
		vec3 right1 = vec3(sin(xangle - 3.14f / 2.0f), 0.0f, cos(xangle - 3.14f / 2.0f));
		vec3 up = glm::cross(right1, direction);
		viewMatrix = glm::lookAt(glm::vec3(lx, ly, lz), vec3(lx, ly, lz) + direction, up);
		if (planeview == 1) {
			mat4 m = mat4(1.0f);
			m = glm::rotate(mat4(1.0f), planerot, vec3(1, 0, 0));
			vec4 unnormup = m * vec4(0,1,0,1);
			vec3 upp = glm::normalize(vec3(unnormup));//glm::normalize(planepos - vec3(-15.0f, -5.0f, -32.0f));
			vec3 directionp = glm::cross(vec3(planerot, 0.0f, 0.0f), up);
			viewMatrix = glm::lookAt(planepos, planepos + directionp, upp);
		}
	}
	//planet movement
	oldtime = newtime;
	//if (rotz == 1)
		rotz_press_num += deltatime / 10.0f;
	planerot+=planespeed;
	//lightbox movement
	lightboxx = 0.0;
	lightboxy = sin(oldtime/1000.0f)*40.0f + 10.0f;
	lightboxz = cos(oldtime/1000.0f)*20.0f - 30.f;

	glClearColor(0.15f, 0.1f, 0.1f, 1.0f); //specify the background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glm::mat4 projectionMatrix = glm::perspective(fov, (float)vp[2] / vp[3], 1.0f, 128.0f);

	glUseProgram(skyboxProgram);
	
	
	glm::mat4 Skb_ModelMatrix = glm::translate(glm::mat4(1.0f), vec3(lx,ly,lz));
	if(planeview==1)Skb_ModelMatrix = glm::translate(glm::mat4(1.0f), planepos);
	Skb_ModelMatrix = glm::scale(Skb_ModelMatrix, vec3(50.0f));
	//remove any translation component of the view matrix
	//glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
	//glm::mat4 projection = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenWidth, 0.1f, 100.0f);
	//glm::mat4 view = viewMatrix;//glm::mat4(glm::mat3(0.0f));
	//glm::mat4 projection = glm::perspective(1.0f, 1.0f, 0.1f, 100.0f);

	glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "M"), 1, GL_FALSE, &Skb_ModelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);
	GLuint skybox_fog_flagUniiformLocation = glGetUniformLocation(skyboxProgram, "fog_flag");
	GLuint skybox_fog_ColorUniiformLocation = glGetUniformLocation(skyboxProgram, "fog_Color");
	//fog_flag = true;
	glUniform1i(skybox_fog_flagUniiformLocation, fog_flag);
	glUniform3fv(skybox_fog_ColorUniiformLocation, 1, &fog_Color[0]);
	//skybox cube
	glBindVertexArray(vaoSkybox);
	glUniform1i(glGetUniformLocation(skyboxProgram, "skybox"), 10);

	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture[10]);
	glDrawArrays(GL_TRIANGLES, 0, drawSize[MAXOBJ]);

	glBindVertexArray(0);
	glDepthMask(GL_TRUE);


	glUseProgram(commonProgram);

	glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
	glm::mat4 rotationMatrix = glm::mat4(1.0f);
	glm::mat4 Matrix = glm::mat4(1.0f);
	glm::vec4 ambientLight1(0.2f, 0.2f, 0.2f, 1.0f);
	vec3 lightPosition1(lightboxx,lightboxy,lightboxz);
	vec3 lightPosition2(20.0, 0.0f, -35.0f);
	vec3 lightPositionr(-4.0, 10.0f, -22.0f);
	//printf("%.3f %.3f %.3f\n", carx, -0.0f ,carz);
	vec3 lightPositiony(-4.0, 9.7f, -22.0f);
	vec3 lightPositiong(-4.0, 9.4f, -22.0f);
	vec3 eyePosition(0.0f, 0.0f, 0.0f);

	bool normalMapping_flag = false;
	bool multiMapping_flag = false;

	//fog_flag = true;
	glUniform1i(fog_flagUniiformLocation, fog_flag);
	glUniform3fv(fog_ColorUniiformLocation, 1, &fog_Color[0]);


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

	int i;
	for (i = 0; i < sizeof(drawnListIndex) / sizeof(drawnListIndex[0]); i++) {
		DrawnObj o = drawnList[i];
		glBindVertexArray(vao[o.vao]);
		glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &o.modelTransformMatrix[0][0]);
		glUniform1i(sunUniformLocation, o.isLightSource);
		switch (o.mode) {
		case NOTXT:
			glUniform1i(normalMapping_flagUniiformLocation, false);
			glUniform1i(multiMapping_flagUniiformLocation, false);
			break;
		case SIGTXT:
			glUniform1i(normalMapping_flagUniiformLocation, false);
			glUniform1i(multiMapping_flagUniiformLocation, false);
			glUniform1i(textureID, o.texture);
			break;
		case MULTXT:
			glUniform1i(normalMapping_flagUniiformLocation, false);
			glUniform1i(multiMapping_flagUniiformLocation, true);
			glUniform1i(textureID, o.texture);
			glUniform1i(textureID2, o.texture2);
			break;
		case NORMTXT:
			glUniform1i(normalMapping_flagUniiformLocation, true);
			glUniform1i(multiMapping_flagUniiformLocation, false);
			glUniform1i(textureID, o.texture);
			glUniform1i(textureID2, o.norm_texture);
		}
		glDrawArrays(GL_TRIANGLES, 0, drawSize[o.vao]);
	}

/*#######################################################Particle###############################################*/

	glUseProgram(instanceProgram);
	GLuint i_viewMatrixUniformLocation = glGetUniformLocation(instanceProgram, "viewMatrix");
	glUniformMatrix4fv(i_viewMatrixUniformLocation, 1, GL_FALSE, &viewMatrix[0][0]);
	GLuint i_projectionMatrixUniformLocation = glGetUniformLocation(instanceProgram, "projectionMatrix");
	glUniformMatrix4fv(i_projectionMatrixUniformLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	glBindVertexArray(vao[VAO_PARTICAL]);
	mat4 i_modelTransformMatrix = glm::mat4(1.0f);
	i_modelTransformMatrix = glm::translate(i_modelTransformMatrix, vec3(20.0, 0.0f, -35.0f));
	GLuint i_modelTransformMatrixUniformLocation = glGetUniformLocation(instanceProgram, "modelTransformMatrix");
	glUniformMatrix4fv(i_modelTransformMatrixUniformLocation, 1, GL_FALSE, &i_modelTransformMatrix[0][0]);

	// Simulate all particles
	int parcnt = 0;
	for (int i = 0; i<maxnopar; i++) {

		Particle& p = ParticlesContainer[i]; // shortcut

		if (p.life > 0.0f) {

			// Decrease life
			p.life -= delta;
			if (p.life > 0.0f) {

				// Simulate simple physics : gravity only, no collisions
				p.angle += p.w * (float)delta;
				p.selfRotAngle += p.selfRotW * (float)delta;
				p.pos = vec3(sin(p.angle) * p.radius, 0.0f, cos(p.angle) * p.radius);
				p.cameradistance = glm::length(p.pos - eyePosition);
				//ParticlesContainer[i].pos += vec3(0.0f,10.0f, 0.0f) * (float)delta;

				mat4 m = mat4(1.0f);
				m = glm::translate(m, p.pos);
				//m = glm::translate(m, vec3(-25.0, -5.0f, -40.0f));
				m = glm::scale(m, vec3(p.size));
				m = glm::rotate(m, p.selfRotAngle, vec3(-0.4, 1, 0));

				// Fill the GPU buffer
				g_par_position_size_data[parcnt] = m;

			}
			else {
				// Particles that just died will be put at the end of the buffer in SortParticles();
				p.cameradistance = -1.0f;
			}

			parcnt++;

		}
	}

	SortParticles();

	glBindBuffer(GL_ARRAY_BUFFER, ppvbo);
	glBufferData(GL_ARRAY_BUFFER, maxnopar * sizeof(mat4), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, parcnt * sizeof(mat4), g_par_position_size_data);


	//fog_flag = true;
	glUniform1i(fog_flagUniiformLocation, fog_flag);
	glUniform3fv(fog_ColorUniiformLocation, 1, &fog_Color[0]);


	glUniform3fv(i_ambientLightUniformLocation, 1, &ambientLight1[0]);
	glUniform3fv(i_lightPosition1UniformLocation, 1, &lightPosition1[0]);
	glUniform3fv(i_lightPosition2UniformLocation, 1, &lightPosition2[0]);

	glUniform1f(i_dd1, ddd);
	glUniform1f(i_sd, spe);

	glUniform1i(i_textureID, TXT_PARTICAL);
								 // This is equivalent to :
								 // for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4),
	glDrawArraysInstanced(GL_TRIANGLES, 0, drawSize[VAO_PARTICAL], parcnt);
	
/*###########################################################Particle####################################################*/

	glFlush();
	glutPostRedisplay();
}

void generateAllPartical() {

	int newparticles = (int)(MAXPP);
	//if (newparticles > (int)(0.005f*3000.0))newparticles = (int)(0.005f*3000.0);

	for (int i = 0; i<newparticles; i++) {
		int particleIndex = FindUnusedParticle();
		ParticlesContainer[particleIndex].life = 100.0f;
		//ParticlesContainer[particleIndex].pos = vec3((rand() % 2000 - 1000) / 50.0f, 10, -30.0f + (rand() % 2000 - 1000.0f) / 50.0f);

		ParticlesContainer[particleIndex].angle = 6.28f * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
		ParticlesContainer[particleIndex].selfRotAngle = 6.28f * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
		ParticlesContainer[particleIndex].w = 0.01f + 0.0f * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
		ParticlesContainer[particleIndex].selfRotW = 0.01f + 0.1f * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));

		ParticlesContainer[particleIndex].size = 0.05f + 0.5f * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
		ParticlesContainer[particleIndex].radius = 45.0f + 30.0f * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));

	}

}

void initializedGL(void)
{
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	commonProgram = installShaders("VertexShaderCode.glsl", "FragmentShaderCode.glsl");
	skyboxProgram = installShaders("SkyboxVertexShaderCode.glsl", "SkyboxFragmentShaderCode.glsl");
	instanceProgram = installShaders("InstanceVertexShaderCode.glsl", "InstanceFragmentShaderCode.glsl");
	sendDataToOpenGL();
	getAllUniformLocation();
	computeAllDrawnObj();
	generateAllPartical();
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
	glutMouseFunc(MouseWheel);

	glutMainLoop();

	return 0;
}

void DrawnObj::setContent(GLuint aVao, GLuint aTexture, GLuint aTexture2, GLuint aNorm_texture, TXTMode aMode, bool aIsLightSource)
{
	vao = aVao;
	texture = aTexture;
	texture2 = aTexture2;
	norm_texture = aNorm_texture;
	mode = aMode;
	modelTransformMatrix = mat4(1.0f);
	isLightSource = aIsLightSource;
}

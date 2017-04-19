#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"
#include "helpers.h"


Point3D lightSourcesColorsArr[] = { {1.0f, 0.0f, 0.0f}, // Red light
{0.0f, 1.0f, 0.0f}, // Green light
{0.0f, 0.0f, 1.0f}, // Blue light
{1.0f, 1.0f, 1.0f} }; // White light
GLfloat specularExponent[] = {10.0, 20.0, 60.0, 5.0};
GLint isDirectional[] = {0,0,1,1};
Point3D lightSourcesDirectionsPositions[] = { {10.0f, 5.0f, 0.0f}, // Red light, positional
{0.0f, 5.0f, 10.0f}, // Green light, positional
{-1.0f, 0.0f, 0.0f}, // Blue light along X
{0.0f, 0.0f, -1.0f} }; // White light along Z

TextureData ttex; // terrain

float deltaTime = 20;
float oldTimeSinceStart = 0;
mat4 camMatrix,projectionMatrix;

int time = 0;
Model *m, *m2, *terrain, *groundSphere, *tree, *skyBox;
GLuint program, skyBoxProgram;
GLuint tex1, tex2, texBranch, coconut, skyBoxTex;
vec3 cam = {0, 5, 0};
vec3 position = { 0, 0, 5 };
GLfloat speed = 0.01f; // 3 units / second
GLfloat mouseSpeed = 0.00005f;
float distance = 20 * 0.01f;

// mouese coordinates
GLfloat xpos;
GLfloat ypos;
vec3 right;
vec3 direction;

typedef struct Tree {
	float x;
	float z;
	float r;
} Tree;

int numberOfTrees = 100;
//Tree trees[100] = 100;
Tree *trees;

void handleKeyPress(){
	//	printf("%f %f\n", distance, direction.x);

	if (glutKeyIsDown('w')) {
		position.x += distance * direction.x;
		position.z += distance * direction.z;
	}
	if (glutKeyIsDown('s')) {
		position.x -= distance * direction.x;
		position.z -= distance * direction.z;
	}
	if (glutKeyIsDown('a')) {
		position.x += distance * right.x;
		position.z += distance * right.z;
	}
	if (glutKeyIsDown('d')) {
		position.x -= distance * right.x;
		position.z -= distance * right.z;
	}
}

void handleCollision() {
	int i;
	double xDiff, zDiff;
	//printf("camera pos: %f %f\n", position.x, position.z);

	for(i = 0; i < numberOfTrees; i++){
		xDiff = abs(trees[i].x - position.x);
		zDiff = abs(trees[i].z - position.z);
		if(xDiff < trees[i].r && zDiff < trees[i].r){
			// Hanle collision by moving camera to closest border of collision radie.
			// Closes middle of object on x axis : keep x position and move camera to outer boarder on z axis
			if(xDiff < zDiff){
				if(	position.z > trees[i].z) position.z = trees[i].z + trees[i].r;
				else position.z = trees[i].z - trees[i].r;
			}
			else {
				if(	position.x > trees[i].x) position.x = trees[i].x + trees[i].r;
				else position.x = trees[i].x - trees[i].r;
			}
		}
	}
}

void updateCameraPos(){
	handleKeyPress();
	position.y = getGroundY(position.x, position.z, &ttex) - 1;
	handleCollision();
}



void generateTrees(){
	int i;
	double xRandom;
	double zRandom;
	double rRandom;
	trees = malloc(numberOfTrees*20);

	for(i = 0; i < numberOfTrees; i++){
		xRandom = random() % 40 + 1 + 0.01; // +0.01 to avoid borders
		zRandom = random() % 40 + 1 + 0.01;
		rRandom = 0.6 + (random() % 3) * 0.1;
		Tree t = {xRandom, zRandom, rRandom};
		trees[i] = t;
	}
}


void init(void)
{
	// GL inits
	glClearColor(0.2,0.2,0.5,0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	printError("GL inits");

	projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 50.0);

	// Load and compile shader
	program = loadShaders("terrain-new.vert", "terrain-new.frag");
	skyBoxProgram = loadShaders("skybox.vert", "skybox.frag");

	glUseProgram(program);
	printError("init shader");

	glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform1i(glGetUniformLocation(program, "tex"), 0); // Texture unit 0
	LoadTGATextureSimple("maskros512.tga", &tex1);
	LoadTGATextureSimple("SkyBox512.tga", &tex2);
	LoadTGATextureSimple("grass.tga", &texBranch);
	LoadTGATextureSimple("maskros512.tga", &coconut);

	glUniform3fv(glGetUniformLocation(program, "lightSourcesDirPosArr"), 4, &lightSourcesDirectionsPositions[0].x);
	glUniform3fv(glGetUniformLocation(program, "lightSourcesColorArr"), 4, &lightSourcesColorsArr[0].x);
	glUniform1fv(glGetUniformLocation(program, "specularExponent"), 4, specularExponent);
	glUniform1iv(glGetUniformLocation(program, "isDirectional"), 4, isDirectional);

	// Load terrain data
	LoadTGATextureData("fft-terrain.tga", &ttex);
	terrain = GenerateTerrain(&ttex);
	printError("init terrain");

	groundSphere = LoadModelPlus("groundsphere.obj");
	tree = LoadModelPlus("fir.obj");
	skyBox = LoadModelPlus("skybox.obj");

	glUseProgram(skyBoxProgram);
	// Skybox texturea
	LoadTGATextureSimple("SkyBox512.tga", &skyBoxTex);
	glUniformMatrix4fv(glGetUniformLocation(skyBoxProgram, "projection"), 1, GL_TRUE, projectionMatrix.m);
	glUniform1i(glGetUniformLocation(skyBoxProgram, "skyBoxTex"), 0);

	glUseProgram(program);

	generateTrees();
}



void draw(mat4 modelView, Model *m, GLuint texture){
	mat4 total = Mult(camMatrix, modelView);
	glUniformMatrix4fv(glGetUniformLocation(program, "mdlMatrix"), 1, GL_TRUE, total.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelView"), 1, GL_TRUE, modelView.m);
	glBindTexture(GL_TEXTURE_2D, texture);		// Bind Our Texture tex1
	DrawModel(m, program, "inPosition", "inNormal", "inTexCoord");
}

void drawSphere(){
	vec3 pos;
	float groundSphereHeight = 0.8;
	pos.x = 30 + 3* sin(time*0.05);
	pos.z = 30 + 3* cos (time*0.05);
	pos.y = getGroundY(pos.x,	pos.z, &ttex);
	mat4 t = T(pos.x, pos.y - groundSphereHeight, pos.z);
	mat4 s = S(0.4,0.4,0.4);
	mat4 modelView = Mult(t, s);
	draw(modelView, groundSphere, tex2);
}


void drawTree1(Tree curTree){
	double sizeConstant = 0.7;
	double y = getGroundY(curTree.x,	curTree.z, &ttex);
	mat4 t = T(curTree.x, y,curTree.z);
	mat4 r = Rx(3.14);
	mat4 s = S(curTree.r * sizeConstant, curTree.r * sizeConstant, curTree.r * sizeConstant);
	mat4 modelView = Mult(Mult(t, s),r);
	draw(modelView, tree, coconut);
}

void drawTrees(){
	int i;
	for(i = 0; i < numberOfTrees; i++){
		drawTree1(trees[i]);
	}
}

void drawTerrain(){
	mat4 modelView = IdentityMatrix();
	draw(modelView, terrain, texBranch);
}

void drawSkyBox(){
	mat4 skyBoxLookAt = camMatrix;
	skyBoxLookAt.m[3] = 0;
	skyBoxLookAt.m[7] = 0;
	skyBoxLookAt.m[11] = 0;
	float s = 1;
	float t = 0.2;
	mat4 trans = T(t, t, t);
	mat4 scale = S(s, s, s);
	mat4 rot = Rx(3.14);
	mat4 total = Mult(Mult(trans, scale),rot);
	glUseProgram(skyBoxProgram);
	glUniformMatrix4fv(glGetUniformLocation(skyBoxProgram, "lookAtMatrix"), 1, GL_TRUE, skyBoxLookAt.m);
	glUniformMatrix4fv(glGetUniformLocation(skyBoxProgram, "mdlMatrix"), 1, GL_TRUE, total.m);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, skyBoxTex);
	DrawModel(skyBox, skyBoxProgram, "in_Position", "in_Normal", "inTexCoord");
	glEnable(GL_DEPTH_TEST);
	//	glEnable(GL_CULL_FACE);
	glUseProgram(program);
}


GLfloat horizontalAngle = 1.2f;
GLfloat verticalAngle = 0.5f;
void updateLookAt(){

	horizontalAngle -= mouseSpeed * deltaTime * (300 - xpos) ;
	verticalAngle   -= mouseSpeed * deltaTime * (300 - ypos) ;

	vec3 newDirection = {
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	};
	direction = newDirection;

	vec3 newRight =  {
		sin(horizontalAngle - 3.14f/2.0f),
		0,
		cos(horizontalAngle - 3.14f/2.0f)
	};
	right = newRight;

	vec3 up = {0,-1,0};

	vec3 lookAtPos = {
		position.x + direction.x,
		position.y + direction.y,
		position.z + direction.z
	};
	camMatrix = lookAt(
		position.x, position.y, position.z,
		lookAtPos.x, lookAtPos.y, lookAtPos.z,
		up.x, up.y, up.z
	);
	glUniform3f(glGetUniformLocation(program, "cameraPos"), lookAtPos.x, lookAtPos.y, lookAtPos.z);
}

void display(void) {
	glutWarpPointer(300, 300);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	updateCameraPos();
	updateLookAt();

	drawSkyBox();
	drawSphere();
	drawTerrain();
	drawTrees();

	glutSwapBuffers();
}


void timer(int i)
{
	time = i;
	i++;
	glutTimerFunc(20, &timer, i);
	glutPostRedisplay();
}

void mouse(int x, int y)
{
	xpos = x;
	ypos = y;
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitContextVersion(3, 2);
	glutInitWindowSize (600, 600);
	glutCreateWindow ("E-ol");
	glutDisplayFunc(display);
	//glutSetCursor(0);
	init();
	glutTimerFunc(20, &timer, 0);
	glutPassiveMotionFunc(mouse);
	glutMainLoop();
	exit(0);
}

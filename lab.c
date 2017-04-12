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
mat4 total, camMatrix,projectionMatrix;

int time = 0;
Model *m, *m2, *terrain, *groundSphere, *tree;
GLuint program;
GLuint tex1, tex2, texBranch, coconut;
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
	float    x;
	float    y;
} Tree;




void handleKeyPress(){
//	printf("%f %f\n", distance, direction.x);

	if (glutKeyIsDown('w')) {
		position.x += distance * direction.x;
		position.y += distance * direction.y;
		position.z += distance * direction.z;
	}
	if (glutKeyIsDown('s')) {
		position.x -= distance * direction.x;
		position.y -= distance * direction.y;
		position.z -= distance * direction.z;
	}
	if (glutKeyIsDown('a')) {
		position.x += distance * right.x;
		position.y += distance * right.y;
		position.z += distance * right.z;
	}
	if (glutKeyIsDown('d')) {
		position.x -= distance * right.x;
		position.y -= distance * right.y;
		position.z -= distance * right.z;
	}
}

int numberOfTrees = 200;
//Tree trees[100] = 100;
Tree *trees;


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
	glUseProgram(program);
	printError("init shader");

	glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform1i(glGetUniformLocation(program, "tex"), 0); // Texture unit 0
	LoadTGATextureSimple("maskros512.tga", &tex1);
	LoadTGATextureSimple("SkyBox512.tga", &tex2);
	LoadTGATextureSimple("grass.tga", &texBranch);
	LoadTGATextureSimple("maskros512.tga", &coconut);


	// Upload light sources to shader
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

	// Generate random trees
	int i;
	int xRandom;

	int yRandom;
	int max = 10;
	trees = malloc(numberOfTrees*10);

	for(i = 0; i < numberOfTrees; i++){
		xRandom = random() % 40 + 1;
		yRandom = random() % 40 + 1;
		Tree t = {xRandom, yRandom};
		trees[i] = t;
	}
}


void draw(mat4 modelView, Model *m, GLuint texture){
	total = Mult(camMatrix, modelView);
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



void drawTree1(float x, float y){
	vec3 pos;
	pos.x = x;
	pos.z = y;
	pos.y = getGroundY(pos.x,	pos.z , &ttex);
	mat4 t = T(pos.x,pos.y, pos.z);
	mat4 r = Rx(3.14);
	mat4 s = S(0.4,0.4,0.4);
	mat4 modelView = Mult(Mult(t, s),r);
	draw(modelView, tree, coconut);
}

void drawTrees(){
	int a;


	int i;
	for(i = 0; i < numberOfTrees; i++){

		drawTree1(trees[i].x+0.01,trees[i].y+0.01);
	}
}

void drawTerrain(){
	mat4 modelView = IdentityMatrix();
	draw(modelView, terrain, texBranch);
}



GLfloat horizontalAngle = 1.2f;
GLfloat verticalAngle = 0.5f;
void updateCameraPos(){

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
	position.y = getGroundY(position.x, position.z, &ttex) - 1;

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
	handleKeyPress();
	updateCameraPos();
	glUseProgram(program);

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
	init ();
	glutTimerFunc(20, &timer, 0);
	glutPassiveMotionFunc(mouse);
	glutMainLoop();
	exit(0);
}

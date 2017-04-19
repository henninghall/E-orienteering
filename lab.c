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
int numberOfTrees = 100;
int numberOfRocks = 10;
int windowSize = 800;

mat4 camMatrix,projectionMatrix;

int time = 0;
Model *m, *m2, *terrain, *groundSphere, *tree, *skyBox, *rock, *map;
GLuint program, skyBoxProgram;
GLuint tex1, tex2, texBranch, coconut, skyBoxTex, stone;
vec3 cam = {0, 5, 0};
vec3 position = { 0, 0, 5 };
vec3 lookAtPos = { 0, 0, 5 };

GLfloat speed = 0.01f; // 3 units / second
GLfloat mouseSpeed = 0.00005f;
float distance = 20 * 0.01f;

// mouese coordinates
GLfloat xpos;
GLfloat ypos;
vec3 right;
vec3 direction;

WorldObject *trees;
WorldObject *rocks;

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

void handleCollision(WorldObject obj) {
	double xDiff, zDiff;

	xDiff = abs(obj.x - position.x);
	zDiff = abs(obj.z - position.z);
	if(xDiff < obj.r && zDiff < obj.r){
		// Hanle collision by moving camera to closest border of collision radie.
		// Closes middle of object on x axis : keep x position and move camera to outer boarder on z axis
		if(xDiff < zDiff){
			if(	position.z > obj.z) position.z = obj.z + obj.r;
			else position.z = obj.z - obj.r;
		}
		else {
			if(	position.x > obj.x) position.x = obj.x + obj.r;
			else position.x = obj.x - obj.r;
		}
	}
}

void handleCollisions() {
	int i;

	// trees
	for(i = 0; i < numberOfTrees; i++){
		WorldObject obj = trees[i];
		handleCollision(obj);
	}

	// rocks
	for(i = 0; i < numberOfRocks; i++){
		WorldObject obj = rocks[i];
		handleCollision(obj);
	}
}

void updateCameraPos(){
	handleKeyPress();
	position.y = getGroundY(position.x, position.z, &ttex) - 1;
	handleCollisions();
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
	LoadTGATextureSimple("conc.tga", &stone);

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
	rock = LoadModelPlus("stone.obj");
	skyBox = LoadModelPlus("skybox.obj");
	map = LoadModelPlus("cubeplus.obj");

	glUseProgram(skyBoxProgram);
	LoadTGATextureSimple("SkyBox512.tga", &skyBoxTex);
	glUniformMatrix4fv(glGetUniformLocation(skyBoxProgram, "projection"), 1, GL_TRUE, projectionMatrix.m);
	glUniform1i(glGetUniformLocation(skyBoxProgram, "skyBoxTex"), 0);

	glUseProgram(program);

	trees = GenerateTrees(numberOfTrees);
	rocks = GenerateRocks(numberOfRocks);
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

void drawTree1(WorldObject curTree){
	double sizeConstant = 0.7;
	double y = getGroundY(curTree.x,	curTree.z, &ttex);
	mat4 t = T(curTree.x, y,curTree.z);
	mat4 r = Rx(3.14);
	mat4 s = S(curTree.r * sizeConstant, curTree.r * sizeConstant, curTree.r * sizeConstant);
	mat4 modelView = Mult(Mult(t, s),r);
	draw(modelView, tree, coconut);
}

void drawRock(WorldObject obj){
	double sizeConstant = 0.7;
	double height = 0.8;
	double y = getGroundY(obj.x,obj.z, &ttex);
	mat4 t = T(obj.x, y - height,obj.z);
	mat4 r = Rx(3.14);
	mat4 s = S(obj.r * sizeConstant, obj.r * sizeConstant, obj.r * sizeConstant);
	mat4 modelView = Mult(Mult(t, s),r);
	draw(modelView, rock, stone);
}


void drawTrees(){
	int i;
	for(i = 0; i < numberOfTrees; i++){
		drawTree1(trees[i]);
	}
}

void drawRocks(){
	int i;
	for(i = 0; i < numberOfRocks; i++){
		drawRock(rocks[i]);
	}
}


void drawTerrain(){
	mat4 modelView = IdentityMatrix();
	draw(modelView, terrain, texBranch);
}

void drawMap(){


  mat4 m = IdentityMatrix();

	printf("%f %f\n", direction.z, direction.x);

	double a = atan(direction.z/direction.x);
	printf("a: %f\n", a);





	m = Mult(m, T( position.x + direction.x *0.7, position.y+0.5, position.z + direction.z * 0.7));

	m = Mult(m, Rz(3.14/2));
	m = Mult(m, Rx(-a ));

	if(direction.x > 0) m = Mult(m, Rz(1));
	else m = Mult(m, Rz(-1));

	//m = Mult(m, T( lookAtPos.x, lookAtPos.y + 0.3, lookAtPos.z ));





//m = Mult(m, Rx(tan(direction.z)/tan(lookAtPos.x)));

//	m = Mult(m, T( 2 , position.y, 1));

	m = Mult(m, S(0.5 ,0.01, 0.5));
//	m = Mult(m, T( lookAtPos.x, 1, lookAtPos.z));


	//mat4 r = Mult(Rz(lookAtPos.z),Rx(lookAtPos.x));

	//mat4 s = S(0.5 ,0.01, 0.5);
	//mat4 modelView = Mult(Mult(t, s),r);

  draw(m, map, stone);
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

	horizontalAngle -= mouseSpeed * deltaTime * (windowSize / 2 - xpos) ;
	verticalAngle   -= mouseSpeed * deltaTime * (windowSize / 2 - ypos) ;

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

	lookAtPos.x = position.x + direction.x;
	lookAtPos.y = position.y + direction.y;
	lookAtPos.z = position.z + direction.z;

	camMatrix = lookAt(
		position.x, position.y, position.z,
		lookAtPos.x, lookAtPos.y, lookAtPos.z,
		up.x, up.y, up.z
	);
	glUniform3f(glGetUniformLocation(program, "cameraPos"), lookAtPos.x, lookAtPos.y, lookAtPos.z);
}

void display(void) {
	glutWarpPointer( windowSize / 2, windowSize / 2);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	updateCameraPos();
	updateLookAt();

	drawSkyBox();
	drawSphere();
	drawTerrain();
	drawTrees();
	drawRocks();
	drawMap();


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
	glutInitWindowSize (windowSize, windowSize);
	glutCreateWindow ("E-ol");
	glutDisplayFunc(display);
	//glutSetCursor(0);
	init();
	glutTimerFunc(20, &timer, 0);
	glutPassiveMotionFunc(mouse);
	glutMainLoop();
	exit(0);
}

#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"
#include "helpers.h"

TextureData ttex; // terrain

float deltaTime = 20;
float oldTimeSinceStart = 0;
int numberOfTrees = 250, numberOfRocks = 20, numberOfControls = 6, windowSize = 800;

mat4 camMatrix, projectionMatrix;

int time = 0;
Model *m, *m2, *terrain, *groundSphere, *tree, *skyBox, *rock, *map, *circle, *circleFilled;
GLuint program, skyBoxProgram;
GLuint tex1, tex2, texBranch, coconut, skyBoxTex, stone, paper, black, brown, purple, controlPoint, yellow, fir;
vec3 cam = {0, 5, 0};
vec3 position = { 1, 0, 1 };
vec3 lookAtPos = { 0, 0, 5 };

GLfloat mouseSpeed = 0.00005f;
float distance = 20 * 0.018f;

// mouse coordinates
GLfloat xpos, ypos;
vec3 right, direction;
WorldObject *trees, *rocks, *controls, *controlPoints, *heightCurves, *openLandPoints;

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

	xDiff = fabs(obj.x - position.x);
	zDiff = fabs(obj.z - position.z);
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

int nextControlToPunch = 0;
void handleControlCollision(WorldObject obj) {
	double xDiff, zDiff;
	xDiff = fabs(obj.x - position.x);
	zDiff = fabs(obj.z - position.z -obj.r);
	if(xDiff < obj.r  && zDiff < obj.r ){
		nextControlToPunch++;
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

	// control
	handleControlCollision(controls[nextControlToPunch]);
}

void updateCameraPos(){
	handleKeyPress();
	position.y = getGroundY(position.x, position.z) - 1;
	handleCollisions();
}


void init(void)
{
	glClearColor(0.2,0.2,0.5,0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 200.0);
	program = loadShaders("terrain-new.vert", "terrain-new.frag");
	skyBoxProgram = loadShaders("skybox.vert", "skybox.frag");
	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
	//glUniform1i(glGetUniformLocation(program, "texUnit"), 0); // Texture unit 0
	LoadTGATextureSimple("maskros512.tga", &tex1);
	LoadTGATextureSimple("SkyBox512.tga", &tex2);
	LoadTGATextureSimple("grass.tga", &texBranch);
	LoadTGATextureSimple("maskros512.tga", &coconut);
	LoadTGATextureSimple("conc.tga", &stone);
	LoadTGATextureSimple("paper.tga", &paper);
	LoadTGATextureSimple("black.tga", &black);
	LoadTGATextureSimple("yellow.tga", &yellow);
	LoadTGATextureSimple("brown.tga", &brown);
	LoadTGATextureSimple("purple.tga", &purple);
	LoadTGATextureSimple("controlPoint.tga", &controlPoint);
	LoadTGATextureSimple("fir.tga", &fir);

	// Load terrain data
	LoadTGATextureData("fft-terrain.tga", &ttex);
	terrain = GenerateTerrain(&ttex);
	printError("init terrain");

	groundSphere = LoadModelPlus("groundsphere.obj");
	tree = LoadModelPlus("fir.obj");
	circle = LoadModelPlus("circle.obj");
	circleFilled = LoadModelPlus("circle-filled.obj");
	rock = LoadModelPlus("stone.obj");
	skyBox = LoadModelPlus("skybox.obj");
	map = LoadModelPlus("cubeplus.obj");

	glUseProgram(skyBoxProgram);
	LoadTGATextureSimple("SkyBox512.tga", &skyBoxTex);
	glUniformMatrix4fv(glGetUniformLocation(skyBoxProgram, "projection"), 1, GL_TRUE, projectionMatrix.m);
	glUniform1i(glGetUniformLocation(skyBoxProgram, "skyBoxTex"), 0);

	glUseProgram(program);

	rocks = GenerateRocks(numberOfRocks);
	trees = GenerateTrees(numberOfTrees);

	heightCurves = GenerateHeightCurves();
	openLandPoints = GenerateOpenLandPoints();

	WorldObject* controlPoints = malloc(numberOfControls*25);
	controlPoints[0] = rocks[8];
	controlPoints[1] = rocks[9];
	controlPoints[2] = rocks[5];
	controlPoints[3] = rocks[6];
	controlPoints[4] = rocks[2];
	controlPoints[5] = rocks[15];
	controlPoints[6] = rocks[4];
	controlPoints[7] = rocks[3];

	controls = GenerateControls(controlPoints, numberOfControls);
}


void draw(mat4 modelView, Model *m, GLuint texture, int showShadow){
	mat4 total = Mult(camMatrix, modelView);
	glUniformMatrix4fv(glGetUniformLocation(program, "mdlMatrix"), 1, GL_TRUE, total.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelView"), 1, GL_TRUE, modelView.m);
	glUniform1i(glGetUniformLocation(program, "showShadow"), showShadow);
	glBindTexture(GL_TEXTURE_2D, texture);		// Bind Our Texture tex1
	DrawModel(m, program, "inPosition", "inNormal", "inTexCoord");
}


void drawTree1(WorldObject curTree){
	double sizeConstant = 0.7;
	mat4 t = T(curTree.x, curTree.y, curTree.z);
	mat4 r = Rx(M_PI);
	mat4 s = S(curTree.r * sizeConstant, curTree.r * sizeConstant, curTree.r * sizeConstant);
	mat4 modelView = Mult(Mult(t, s),r);
	draw(modelView, tree, fir, 1);
}

void drawRock(WorldObject obj){
	double sizeConstant = 0.7;
	double height = 0.8;
	mat4 t = T(obj.x, obj.y - height,obj.z);
	mat4 r = Rx(M_PI);
	mat4 s = S(obj.r * sizeConstant, obj.r * sizeConstant, obj.r * sizeConstant);
	mat4 modelView = Mult(Mult(t, s),r);
	draw(modelView, rock, stone, 1);
}

void drawControl(WorldObject obj){
	double size = 0.4;
	double height = 0.4;
	double y = getGroundY(obj.x, obj.z - obj.r);
	mat4 t = T(obj.x, y - height, obj.z - obj.r);
	mat4 r = Rx(M_PI);
	mat4 s = S(size, size, size);
	mat4 modelView = Mult(Mult(t, s),r);
	draw(modelView, map, controlPoint, 1);
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

void drawControls(){
	int i;
	for(i = 0; i < numberOfControls; i++){
		drawControl(controls[i]);
	}
}

void drawTerrain(){
	mat4 modelView = IdentityMatrix();
	draw(modelView, terrain, texBranch, 1);
}

float worldOnMapScale = 0.0039;

void drawMapRock(mat4 m, WorldObject rockObject){
	m = Mult(m, T(-rockObject.x * worldOnMapScale + 0.5, 3, rockObject.z * worldOnMapScale - 0.5));
	m = Mult(m, Rx(M_PI/2));
	m = Mult(m, S(0.35 ,0.35,0.35));
	draw(m, circleFilled, black, 0);
}

void drawControlCount(mat4 m, int i){
	m = Mult(m, T(-0.47 + 0.08 * i, 1, -0.55));
	m = Mult(m, Rx(M_PI/2));
	m = Mult(m, S(1.35 ,1.35,1.35));
	draw(m, circleFilled, purple, 0);
}

void drawMapControl(mat4 m, WorldObject obj){
	float scale = 0.07;
	mat4 pos = Mult(m, T(-obj.x * worldOnMapScale + 0.5, 5, obj.z * worldOnMapScale - 0.5));
	pos = Mult(pos, S(scale, scale, scale));
	draw(pos, circle, purple, 0);
}

void drawMapLine(mat4 m, WorldObject obj1, WorldObject obj2){
	float xDiff = fabs(obj2.x-obj1.x);
	float zDiff = fabs(obj2.z-obj1.z);
	float xPos = (obj1.x + obj2.x) / 2;
	float zPos = (obj1.z + obj2.z) / 2;
	float length = worldOnMapScale* sqrt(xDiff * xDiff + zDiff * zDiff) - 0.11;
	float angle = atan( xDiff / zDiff);

	// correcting angle
	if( obj2.z > obj1.z && obj2.x < obj1.x) angle = -angle;
	if( obj2.x > obj1.x && obj2.z < obj1.z) angle = -angle;

	m = Mult(m, T(-xPos * worldOnMapScale + 0.5, 1, zPos * worldOnMapScale - 0.5));
	m = Mult(m, Ry(-angle));
	m = Mult(m, S(0.005, 0.01, length));

	draw(m, map, purple, 0);
}

void drawHeightCurve(mat4 m, WorldObject obj){
	float scale = 0.0040;
	m = Mult(m, T(-obj.x * worldOnMapScale + 0.5, 1, obj.z * worldOnMapScale - 0.5));
	m = Mult(m, S(scale, scale, scale));
	draw(m, map, brown, 0);
}

void drawOpenLand(mat4 m, WorldObject obj){
	float scale = 0.35;
	m = Mult(m, T(-obj.x * worldOnMapScale + 0.503, 1, obj.z * worldOnMapScale - 0.503));
	m = Mult(m, Rx(M_PI/2));
	m = Mult(m, S(scale, scale, scale));
	draw(m, circleFilled, yellow, 0);
}


double a = 0;
double aPrev = 0;
int spins = 0;
void drawMap(){
	mat4 m = IdentityMatrix();

	aPrev = a;
	a = atan(direction.z/direction.x) - M_PI * spins;

	if(fabs(aPrev - a) > 1.0) {
		spins++;
		a-= M_PI;
	}

	m = Mult(m, T( position.x + direction.x *0.7, position.y+0.65, position.z + direction.z * 0.7));
	m = Mult(m, Rz(M_PI/2));
	m = Mult(m, Rx(-a ));
	m = Mult(m, Rz(1));
	m = Mult(m, S(0.5 ,0.001, 0.5));

	draw(m, map, paper, 0);

	int i;
	for(i = 0; i < 1500; i++) drawOpenLand(m, openLandPoints[i]);
	for(i = 0; i < 10000; i++) drawHeightCurve(m, heightCurves[i]);
	for(i = 0; i < numberOfRocks; i++) drawMapRock(m, rocks[i]);
	for(i = 0; i < numberOfControls; i++) drawMapControl(m, controls[i]);
	for(i = 0; i < numberOfControls - 1; i++) drawMapLine(m, controls[i], controls[i+1]);
	for(i = 0; i < nextControlToPunch; i++) drawControlCount(m, i);
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
	mat4 rot = Rx(M_PI);
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
		sin(horizontalAngle - M_PI/2.0f),
		0,
		cos(horizontalAngle - M_PI/2.0f)
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
	drawTerrain();
	drawTrees();
	drawRocks();
	drawMap();
	drawControls();

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

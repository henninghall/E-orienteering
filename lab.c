// Lab 4, terrain generation

#ifdef __APPLE__
#include <OpenGL/gl3.h>
// Linking hint for Lightweight IDE
// uses framework Cocoa
#endif
#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"

mat4 projectionMatrix;


int time = 0;

vec3 spherePos;
int triangleCount;

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

GLfloat *vertexArray;
GLfloat *normalArray;
GLfloat *texCoordArray;
GLuint *indexArray;
TextureData ttex; // terrain


// http://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle
float sign (vec3 p1, vec3 p2, vec3 p3)
{
	return (p1.x - p3.x) * (p2.z - p3.z) - (p2.x - p3.x) * (p1.z - p3.z);
}
bool PointInTriangle (vec3 pt, vec3 v1, vec3 v2, vec3 v3)
{
	bool b1, b2, b3;

	b1 = sign(pt, v1, v2) < 0.0f;
	b2 = sign(pt, v2, v3) < 0.0f;
	b3 = sign(pt, v3, v1) < 0.0f;

	return ((b1 == b2) && (b2 == b3));
}


float getGroundY(float xIn, float zIn, TextureData *tex){

	int i;
	for (i = 0; i < triangleCount*3; i+=6){
		//	printf("Square : %i\n", i);

		// each vertex
		int v;
		float maxX, maxZ, minX, minZ;
		vec3 vert;

		// Each vertex per square
		for (v = 0; v < 6; v++){

			int vertexIndex = indexArray[i+v];
			vert.x = vertexArray[vertexIndex*3 + 0];
			vert.y = vertexArray[vertexIndex*3 + 1];
			vert.z = vertexArray[vertexIndex*3 + 2];

			if(v == 0 || vert.x > maxX) maxX = vert.x;
			if(v == 0 || vert.x < minX) minX = vert.x;
			if(v == 0 || vert.z > maxZ) maxZ = vert.z;
			if(v == 0 || vert.z < maxZ) minZ = vert.z;

		}

		// Square found.
		if((xIn <= maxX) && (xIn >= minX) && (zIn <= maxZ) && (zIn >= minZ)){
			//	printf("FOUND IN SQUARE: %i\n", i);

			for (v = 0; v < 2; v ++){
				vec3 p1, p2, p3, p;

				p.x = xIn;
				p.y = 0; // unknown
				p.z = zIn;

				int vertexIndex =  indexArray[i + v*3 + 0];
				p1.x = vertexArray[vertexIndex*3 + 0];
				p1.y = vertexArray[vertexIndex*3 + 1];
				p1.z = vertexArray[vertexIndex*3 + 2];

				vertexIndex =  indexArray[i + v*3 + 1];
				p2.x = vertexArray[vertexIndex*3 + 0];
				p2.y = vertexArray[vertexIndex*3 + 1];
				p2.z = vertexArray[vertexIndex*3 + 2];

				vertexIndex =  indexArray[i + v*3 + 2];
				p3.x = vertexArray[vertexIndex*3 + 0];
				p3.y = vertexArray[vertexIndex*3 + 1];
				p3.z = vertexArray[vertexIndex*3 + 2];



				if (PointInTriangle(p,p1,p2,p3)) {

					/*
					printf("IN TRIANGLE\n");

					printf("p x: %f\n", p.x);
					printf("p y : %f\n", p.y);
					printf("p z : %f\n", p.z);

					printf("p1 x: %f\n", p1.x);
					printf("p1 y : %f\n", p1.y);
					printf("p1 z : %f\n", p1.z);

					printf("p2 x: %f\n", p2.x);
					printf("p2 y : %f\n", p2.y);
					printf("p2 z : %f\n", p2.z);

					printf("p3 x: %f\n", p3.x);
					printf("p3 y : %f\n", p3.y);
					printf("p3 z : %f\n", p3.z);
					*/

					// first try: genomsnitt
					//p.y = (p1.y + p2.y + p3.y) / 3;

					float dist1  = fabs(p.x - p1.x) + fabs(p.z - p1.z);
					float dist2  = fabs(p.x - p2.x) + fabs(p.z - p2.z);
					float dist3  = fabs(p.x - p3.x) + fabs(p.z - p3.z);

					printf("dist1 : %f\n", dist1);
					printf("dist2 : %f\n", dist2);
					printf("dist3 : %f\n", dist3);


					float totalDist = dist1 + dist2 + dist3;

					float weight1 = (totalDist-dist1)*(totalDist-dist1);
					float weight2 = (totalDist-dist2)*(totalDist-dist2);
					float weight3 = (totalDist-dist3)*(totalDist-dist3);

					printf("weight1 : %f\n", weight1);
					printf("weight2 : %f\n", weight2);
					printf("weight3 : %f\n", weight3);


					float totalWeight = weight1 + weight2 + weight3;

					p.y = (p1.y*weight1 + p2.y*weight2 + p3.y*weight3) / totalWeight;


					/*	printf("\n");
					printf("RESULT p.y : %f\n", p.y);
					printf("\n");
					*/
					return p.y;
				}
			}
			// ERROR: Could be on a border
			printf("ERROR: Point on a border?");
			return 0;
		}
	}

	// Error no square found
	printf("ERROR:No square found");
	return 0;
}


Model* GenerateTerrain(TextureData *tex)
{
	int vertexCount = tex->width * tex->height;
	triangleCount = (tex->width-1) * (tex->height-1) * 2;
	int x, z;

	vertexArray = malloc(sizeof(GLfloat) * 3 * vertexCount);
	normalArray = malloc(sizeof(GLfloat) * 3 * vertexCount);
	texCoordArray = malloc(sizeof(GLfloat) * 2 * vertexCount);
	indexArray = malloc(sizeof(GLuint) * triangleCount*3);

	printf("bpp %d\n", tex->bpp);
	printf("tex->width %d\n", tex->width);
	printf("tex->height %d\n", tex->height);
	for (x = 0; x < tex->width; x++)
	for (z = 0; z < tex->height; z++)
	{
		// Vertex array. You need to scale this properly
		vertexArray[(x + z * tex->width)*3 + 0] = x / 1.0;
		vertexArray[(x + z * tex->width)*3 + 1] = tex->imageData[(x + z * tex->width) * (tex->bpp/8)] / 15.0;
		vertexArray[(x + z * tex->width)*3 + 2] = z / 1.0;

		// Texture coordinates. You may want to scale them.
		texCoordArray[(x + z * tex->width)*2 + 0] = x; // (float)x / tex->width;
		texCoordArray[(x + z * tex->width)*2 + 1] = z; // (float)z / tex->height;
	}
	for (x = 0; x < tex->width-1; x++)
	for (z = 0; z < tex->height-1; z++)
	{
		// Triangle 1
		indexArray[(x + z * (tex->width-1))*6 + 0] = x + z * tex->width;
		indexArray[(x + z * (tex->width-1))*6 + 1] = x + (z+1) * tex->width;
		indexArray[(x + z * (tex->width-1))*6 + 2] = x+1 + z * tex->width;


		// Triangle 2
		indexArray[(x + z * (tex->width-1))*6 + 3] = x+1 + z * tex->width;
		indexArray[(x + z * (tex->width-1))*6 + 4] = x + (z+1) * tex->width;
		indexArray[(x + z * (tex->width-1))*6 + 5] = x+1 + (z+1) * tex->width;

	}

	for (x = 0; x < tex->width-1; x++)
	for (z = 0; z < tex->height-1; z++)
	{
		vec3 normalVec;

		// skip vertices at the outer border
		if((x > 0) && (z > 0) && (z < tex->height - 1) && (x < tex->width - 1) ){

			vec3 vertA = {
				vertexArray[((x-1) + z * tex->width)*3 + 0],
				vertexArray[((x-1) + z * tex->width)*3 + 1],
				vertexArray[((x-1) + z * tex->width)*3 + 2]
			};

			vec3 vertB = {
				vertexArray[((x+1) + (z+1) * tex->width)*3 + 0],
				vertexArray[((x+1) + (z+1) * tex->width)*3 + 1],
				vertexArray[((x+1) + (z+1) * tex->width)*3 + 2]
			};

			vec3 vertC = {
				vertexArray[((x+1) + (z-1) * tex->width)*3 + 0],
				vertexArray[((x+1) + (z-1) * tex->width)*3 + 1],
				vertexArray[((x+1) + (z-1) * tex->width)*3 + 2]
			};

			vec3 vecBC = VectorSub(vertB,vertC);
			vec3 vecCA = VectorSub(vertC,vertA);

			normalVec = Normalize(CrossProduct(vecBC,vecCA));
		}

		//borders
		else {
			normalVec = (vec3){1,1,1};
		}

		// result
		normalArray[(x + z * tex->width)*3 + 0] = normalVec.x;
		normalArray[(x + z * tex->width)*3 + 1] = normalVec.y;
		normalArray[(x + z * tex->width)*3 + 2] = normalVec.z;
	}
	// End of terrain generation


	// Create Model and upload to GPU:
	Model* model = LoadDataToModel(
		vertexArray,
		normalArray,
		texCoordArray,
		NULL,
		indexArray,
		vertexCount,
		triangleCount*3);

		return model;
	}
	float deltaTime = 1;

	// vertex array object
	Model *m, *m2, *tm, *groundSphere;
	// Reference to shader program
	GLuint program;
	GLuint tex1, tex2;


	vec3 cam = {0, 5, 0};

	vec3 position = { 0, 0, 5 };
	// horizontal angle : toward -Z
	GLfloat horizontalAngle = 3.14f;
	// vertical angle : 0, look at the horizon
	GLfloat verticalAngle = 0.5f;


	GLfloat speed = 0.01f; // 3 units / second
	GLfloat mouseSpeed = 0.00005f;

	// mouese coordinates
	GLfloat xpos;
	GLfloat ypos;

	vec3 right;
	vec3 direction;


	void handleKeyPress(){
		float distance = 20 * speed;

		printf("%f %f\n", distance, direction.x);

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

	void init(void)
	{
		// GL inits
		glClearColor(0.2,0.2,0.5,0);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		printError("GL inits");

		projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 50.0);

		// Load and compile shader
		program = loadShaders("terrain.vert", "terrain.frag");
		glUseProgram(program);
		printError("init shader");

		glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1, GL_TRUE, projectionMatrix.m);
		glUniform1i(glGetUniformLocation(program, "tex"), 0); // Texture unit 0
		LoadTGATextureSimple("maskros512.tga", &tex1);

		// Upload light sources to shader
		glUniform3fv(glGetUniformLocation(program, "lightSourcesDirPosArr"), 4, &lightSourcesDirectionsPositions[0].x);
		glUniform3fv(glGetUniformLocation(program, "lightSourcesColorArr"), 4, &lightSourcesColorsArr[0].x);
		glUniform1fv(glGetUniformLocation(program, "specularExponent"), 4, specularExponent);
		glUniform1iv(glGetUniformLocation(program, "isDirectional"), 4, isDirectional);

		// Load terrain data
		LoadTGATextureData("fft-terrain.tga", &ttex);
		tm = GenerateTerrain(&ttex);
		printError("init terrain");

		groundSphere = LoadModelPlus("groundsphere.obj");
	}

	float oldTimeSinceStart = 0;

	void display(void) {
		glutWarpPointer(300, 300);

		printf("time : %i\n", time);


		// clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 total, modelView, camMatrix;

		printError("pre display");

		glUseProgram(program);

		handleKeyPress();

		deltaTime = 20;
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


		modelView = IdentityMatrix();

		total = Mult(camMatrix, modelView);
		glUniformMatrix4fv(glGetUniformLocation(program, "mdlMatrix"), 1, GL_TRUE, total.m);
		glUniformMatrix4fv(glGetUniformLocation(program, "modelView"), 1, GL_TRUE, modelView.m);

		glBindTexture(GL_TEXTURE_2D, tex1);		// Bind Our Texture tex1
		DrawModel(tm, program, "inPosition", "inNormal", "inTexCoord");


		// Calculate sphere position
		spherePos.x = 30 + 3* sin(time*0.05);
		spherePos.z = 30 + 3* cos (time*0.05);
		spherePos.y = getGroundY(spherePos.x,	spherePos.z, &ttex);


		float groundSphereHeight = 0.8;
		mat4 t = T(spherePos.x,spherePos.y - groundSphereHeight, spherePos.z);
		mat4 s = S(0.4,0.4,0.4);
		modelView = Mult(t, s);



		total = Mult(camMatrix, modelView);
		glUniformMatrix4fv(glGetUniformLocation(program, "mdlMatrix"), 1, GL_TRUE, total.m);
		glUniformMatrix4fv(glGetUniformLocation(program, "modelView"), 1, GL_TRUE, modelView.m);
		DrawModel(groundSphere, program, "inPosition", "inNormal", "inTexCoord");

		printError("display 2");

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
		glutCreateWindow ("TSBK07 Lab 4");
		glutDisplayFunc(display);
		init ();
		glutTimerFunc(20, &timer, 0);

		glutPassiveMotionFunc(mouse);

		glutMainLoop();
		exit(0);
	}

	/*

	int x, z;
	for (x = 0; x < tex->width-1; x++)
	for (z = 0; z < tex->height-1; z++)
	{

	int indexV1 = (x + z * (tex->width-1))*6 + 0;
	int indexV2 = indexV1 + 1;
	int indexV3 = indexV1 + 2;

	int indexV4 = indexV1 + 3;
	int indexV5 = indexV1 + 4;
	int indexV6 = indexV1 + 5;

	printf("xIn: %f\n", xIn);
	printf("zIn: %f\n", zIn);

	printf("x: %i\n", x);
	printf("z: %i\n", z);
	printf("Triangle 1 vertex1-index index: %i\n", indexV1);
	printf("Triangle 1 vertex2-index index: %i\n", indexV2);
	printf("Triangle 1 vertex3-index index: %i\n", indexV3);
	printf("Triangle 1 vertex 1 index: %i\n", indexArray[indexV1]);
	printf("Triangle 1 vertex 2 index %i\n", indexArray[indexV2]);
	printf("Triangle 1 vertex 3 index: %i\n", indexArray[indexV3]);
	printf("Triangle 1 vertex 1 x value: %f\n", vertexArray[indexArray[indexV1]]);
	printf("Triangle 1 vertex 1 y value: %f\n", vertexArray[indexArray[indexV1]+1]);
	printf("Triangle 1 vertex 1 z value: %f\n", vertexArray[indexArray[indexV1]+2]);
	printf("Triangle 1 vertex 2 x value: %f\n", vertexArray[indexArray[indexV2]]);
	printf("Triangle 1 vertex 2 y value: %f\n", vertexArray[indexArray[indexV2]+1]);
	printf("Triangle 1 vertex 2 z value: %f\n", vertexArray[indexArray[indexV2]+2]);
	printf("Triangle 1 vertex 3 x value: %f\n", vertexArray[indexArray[indexV3]]);
	printf("Triangle 1 vertex 3 y value: %f\n", vertexArray[indexArray[indexV3]+1]);
	printf("Triangle 1 vertex 3 z value: %f\n", vertexArray[indexArray[indexV3]+2]);

	printf("Triangle 2 vertex1-index index: %i\n", indexV4);
	printf("Triangle 2 vertex2-index index: %i\n", indexV5);
	printf("Triangle 2 vertex3-index index: %i\n", indexV6);
	printf("Triangle 2 vertex 1 index: %i\n", indexArray[indexV4]);
	printf("Triangle 2 vertex 2 index %i\n", indexArray[indexV5]);
	printf("Triangle 2 vertex 3 index: %i\n", indexArray[indexV6]);
	printf("Triangle 2 vertex 1 x value: %f\n", vertexArray[indexArray[indexV4]]);
	printf("Triangle 2 vertex 1 y value: %f\n", vertexArray[indexArray[indexV4]+1]);
	printf("Triangle 2 vertex 1 z value: %f\n", vertexArray[indexArray[indexV4]+2]);
	printf("Triangle 2 vertex 2 x value: %f\n", vertexArray[indexArray[indexV5]]);
	printf("Triangle 2 vertex 2 y value: %f\n", vertexArray[indexArray[indexV5]+1]);
	printf("Triangle 2 vertex 2 z value: %f\n", vertexArray[indexArray[indexV5]+2]);
	printf("Triangle 2 vertex 3 x value: %f\n", vertexArray[indexArray[indexV6]]);
	printf("Triangle 2 vertex 3 y value: %f\n", vertexArray[indexArray[indexV6]+1]);
	printf("Triangle 2 vertex 3 z value: %f\n", vertexArray[indexArray[indexV6]+2]);


	printf("\n");
}

// 1) Calculate what quad the point falls into.



indexArray[(x + z * (tex->width-1))*6 + 1] = x + (z+1) * tex->width;
indexArray[(x + z * (tex->width-1))*6 + 2] = x+1 + z * tex->width;
// Triangle 2
indexArray[(x + z * (tex->width-1))*6 + 3] = x+1 + z * tex->width;
indexArray[(x + z * (tex->width-1))*6 + 4] = x + (z+1) * tex->width;
indexArray[(x + z * (tex->width-1))*6 + 5] = x+1 + (z+1) * tex->width;
*/

// 2) The quad is built from two triangles. Figure out which one to use.

//3) Calculate the height value. There are several ways to do it. You can interpolate over the surface, or you can use the plane equation.

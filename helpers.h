
#ifdef __APPLE__
	#include <OpenGL/gl3.h>
#else
	#if defined(_WIN32)
		#include "glew.h"
	#endif
	#include <GL/gl.h>
#endif


#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"

typedef struct WorldObject {
	float x;
	float z;
	float r;
} WorldObject;


float getGroundY(float xIn, float zIn, TextureData *tex);
Model* GenerateTerrain(TextureData *tex);
WorldObject* GenerateTrees();
WorldObject* GenerateRocks();

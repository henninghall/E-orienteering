#include "helpers.h"

#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"

GLfloat *vertexArray;
GLfloat *normalArray;
GLfloat *texCoordArray;
GLuint *indexArray;
int triangleCount;

float treeLine = 1.9;


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

WorldObject* GenerateTrees(int numberOfTrees){
  int i;
  double xRandom, zRandom, rRandom, y;
  WorldObject* trees = malloc(numberOfTrees*16);
  int maxDistance = 250;

  for(i = 0; i < numberOfTrees; i++){
    xRandom = random() % maxDistance + 0.01;
    zRandom = random() % maxDistance + 0.01;
    rRandom = 1 + (random() % 3) * 0.4 - 0.2;
    y = getGroundY(xRandom,	zRandom);

    if(y > treeLine) {
      WorldObject t = {xRandom, zRandom, rRandom, y};
      trees[i] = t;
    }
    else i--;
  }
  return trees;
}

// ControlPoint = stone object etc
// Controls : actual control point object
WorldObject* GenerateControls(WorldObject *controlPoints, int numberOfControls){
  int i;
  double x, z, r, y;
  WorldObject* controls = malloc(numberOfControls*16);

  for(i = 0; i < numberOfControls; i++){
    x = controlPoints[i].x;
    z = controlPoints[i].z;
    r = controlPoints[i].r;
    y = getGroundY(x,	z);
    WorldObject cur = {x, z, r, y};
    controls[i] = cur;
  }
  return controls;
}

WorldObject* GenerateRocks(int numberOfRocks){
  int i;
  double x, z, r, y;
  WorldObject* rocks = malloc(numberOfRocks*16);
  int maxDistance = 250;

  for(i = 0; i < numberOfRocks; i++){
    x = random() % maxDistance + 0.01; // +0.01 to avoid borders
    z = random() % maxDistance + 0.01;
    r = 1.6 + (random() % 3) * 0.3;
    y = getGroundY(x,	z);
    WorldObject w = {x, z, r, y};
    rocks[i] = w;
  }
  return rocks;
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

  WorldObject* GenerateHeightCurves(){

    WorldObject *curvePoints;
    curvePoints = malloc(sizeof(GLfloat) * triangleCount);

    int numberOfCurvePoints = 0;
    int i;
    for (i = 0; i < triangleCount*3; i+=6){

      // each vertex
      int v;
      float maxY, minY;
      vec3 vert;

      // Each vertex per square
      for (v = 0; v < 6; v++){
        int vertexIndex = indexArray[i+v];
        vert.x = vertexArray[vertexIndex*3 + 0];
        vert.y = vertexArray[vertexIndex*3 + 1];
        vert.z = vertexArray[vertexIndex*3 + 2];

        if(v == 0 || vert.y > maxY) maxY = vert.y;
        if(v == 0 || vert.y < minY) minY = vert.y;
      }

      int j;
      int equ = 4;
      for(j = 0; j<20 ;j++) {
        if(minY < equ*j && maxY >= equ*j){
          WorldObject cur = {vert.x, vert.z, 0};
          curvePoints[numberOfCurvePoints++] = cur;
        }
      }
    }
    return curvePoints;
  }


  WorldObject* GenerateOpenLandPoints(){

    WorldObject *openLandPoints;
    openLandPoints = malloc(sizeof(GLfloat) * triangleCount);
    int numberOfOpenLandPoints = 0;
    int i;
    for (i = 0; i < triangleCount*3; i+=6){

      // each vertex
      int v;
      float minY;
      vec3 vert;

      // Each vertex per square
      for (v = 0; v < 6; v++){
        int vertexIndex = indexArray[i+v];
        vert.x = vertexArray[vertexIndex*3 + 0];
        vert.y = vertexArray[vertexIndex*3 + 1];
        vert.z = vertexArray[vertexIndex*3 + 2];

        if(v == 0 || vert.y < minY) minY = vert.y;
      }


      if(minY < treeLine && ((int)round(vert.x) % 3 == 0) && ((int)round(vert.z) % 3 == 0) ){
        WorldObject cur = {vert.x, vert.z, 0};
        openLandPoints[numberOfOpenLandPoints++] = cur;
      }
    }
    return openLandPoints;
  }


  float getGroundY(float xIn, float zIn){

    int i;
    for (i = 0; i < triangleCount*3; i+=6){

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

            float dist1  = fabs(p.x - p1.x) + fabs(p.z - p1.z);
            float dist2  = fabs(p.x - p2.x) + fabs(p.z - p2.z);
            float dist3  = fabs(p.x - p3.x) + fabs(p.z - p3.z);

            float totalDist = dist1 + dist2 + dist3;

            float weight1 = (totalDist-dist1)*(totalDist-dist1);
            float weight2 = (totalDist-dist2)*(totalDist-dist2);
            float weight3 = (totalDist-dist3)*(totalDist-dist3);

            float totalWeight = weight1 + weight2 + weight3;

            p.y = (p1.y*weight1 + p2.y*weight2 + p3.y*weight3) / totalWeight;

            return p.y;
          }
        }
        // ERROR: Could be on a border
        //printf("ERROR: Point on a border?");
        return 0;
      }
    }

    // Error no square found
    //  printf("ERROR:No square found");
    return 0;
  }

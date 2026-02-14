/*
  MIT License

  Copyright (c) 2026 Stefan Mader

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#ifndef __GAMEOBJ_HPP__
#define __GAMEOBJ_HPP__

#include "CONFIG.HPP"
#include "STL/ARRAY.HPP"
#include "STL/HASHMAP.HPP"
#include "STL/VECTOR.HPP"
#include "STL/QUATERNN.HPP"
#include "STL/MATRIX.HPP"
#include "SERIALZE.HPP"
#include "RENDER.HPP" // for camera
#include <stdio.h>

enum GAMEOBJECTTYPE {
  GAMEOBJECT_UNDEFINED = 0,
  GAMEOBJECT_MESH = 1,
  GAMEOBJECT_CAMERA = 2,
  GAMEOBJECT_SKYBOX = 3,
  GAMEOBJECT_PATH = 4,
  GAMEOBJECT_BILLBOARD = 5,
  GAMEOBJECT_SECTION = 6,
  GAMEOBJECT_TRIGGER = 7,
  GAMEOBJECT_LINK = 8,
  GAMEOBJECT_COUNT = 9
};
extern const char *gameObjectTypeNames[GAMEOBJECT_COUNT];

enum GAMEOBJECTPROPERTY {
  GAMEOBJECTPROPERTY_UNDEFINED = 0,
  GAMEOBJECTPROPERTY_BOOL = 1,
  GAMEOBJECTPROPERTY_INT = 2,
  GAMEOBJECTPROPERTY_FLOAT = 3,
  GAMEOBJECTPROPERTY_FLOAT4 = 4,
  GAMEOBJECTPROPERTY_TEXT = 5,
  GAMEOBJECTPROPERTY_COLOR = 6,
  GAMEOBJECTPROPERTY_COUNT = 7
};
extern const char *propertyTypeNames[GAMEOBJECTPROPERTY_COUNT];

class GameObjectProperty {
public:
  GameObjectProperty() {
    type = GAMEOBJECTPROPERTY_UNDEFINED;
    sprintf(name,NONAME);
    clearValue();
  }
  char name[NAMEMAX];
  unsigned int type;
  union {
    char goText[NAMEMAX];
    float goFloat;
    float goFloat4[4];
    int goInt;
    bool goBool;
    unsigned int goColor;
  };
  void setName(const char *name);
  void clearValue();
};
#define S_GAMEOBJECTPROPERTY(__name__,__o__)\
  S_OBJECT_OPEN(__name__);\
  S_NAME("name",(__o__)->name);\
  S_UINT("type",(__o__)->type);\
  switch((__o__)->type) {\
  case GAMEOBJECTPROPERTY_UNDEFINED: {} break;\
  case GAMEOBJECTPROPERTY_BOOL: {S_BOOL("goBool",(__o__)->goBool);} break;\
  case GAMEOBJECTPROPERTY_INT: {S_INT("goInt",(__o__)->goInt);} break;\
  case GAMEOBJECTPROPERTY_FLOAT: {S_FLOAT("goFloat",(__o__)->goFloat);} break;\
  case GAMEOBJECTPROPERTY_FLOAT4: {S_FLOAT4("goFloat4",(__o__)->goFloat4);} break;\
  case GAMEOBJECTPROPERTY_TEXT: {S_NAME("goText",(__o__)->goText);} break;\
  case GAMEOBJECTPROPERTY_COLOR: {S_UINT("goColor",(__o__)->goColor);} break;\
  }\
  S_OBJECT_CLOSE();


extern unsigned int currentUniqueGameObjectId;

class Grid {
public:
  Grid() {
    posXEnabled = true;
    posYEnabled = true;
    posZEnabled = true;
    gridPos = Vector(0,0,0);
    scaleXEnabled = true;
    scaleYEnabled = true;
    scaleZEnabled = true;
    rotEnabled = true;
    posGranularity.set(1,1,1);
    scaleGranularity.set(1,1,1);
    rotGranularity = 45;
    displayGridOn = true;
    displayGridSize = 100;
    displayGridSteps = 1;
    displayGridColor = 0xffe0e0ff;
  }
  bool posXEnabled;
  bool posYEnabled;
  bool posZEnabled;
  bool scaleXEnabled;
  bool scaleYEnabled;
  bool scaleZEnabled;
  bool rotEnabled;
  Vector gridPos;
  Vector posGranularity;
  Vector scaleGranularity;
  double rotGranularity;
  bool displayGridOn;
  unsigned int displayGridColor;
  float displayGridSize;
  float displayGridSteps;
};
#define S_GRID(__name__,__o__)\
  S_OBJECT_OPEN(__name__);\
  S_BOOL("posXEnabled",(__o__)->posXEnabled);\
  S_BOOL("posYEnabled",(__o__)->posYEnabled);\
  S_BOOL("posZEnabled",(__o__)->posZEnabled);\
  S_BOOL("scaleXEnabled",(__o__)->scaleXEnabled);\
  S_BOOL("scaleYEnabled",(__o__)->scaleYEnabled);\
  S_BOOL("scaleZEnabled",(__o__)->scaleZEnabled);\
  S_BOOL("rotEnabled",(__o__)->rotEnabled);\
  S_VECTOR("gridPos",(__o__)->gridPos);\
  S_VECTOR("posGranularity",(__o__)->posGranularity);\
  S_VECTOR("scaleGranularity",(__o__)->scaleGranularity);\
  S_DOUBLE("rotGranularity",(__o__)->rotGranularity);\
  S_BOOL("displayGridOn",(__o__)->displayGridOn);\
  S_UINT("displayGridColor",(__o__)->displayGridColor);\
  S_FLOAT("displayGridSize",(__o__)->displayGridSize);\
  S_FLOAT("displayGridSteps",(__o__)->displayGridSteps);\
  S_OBJECT_CLOSE();

#define MAXGRIDS 1
extern Grid grids[MAXGRIDS];
void applyGrid(Grid *grid, Vector *t, Vector *s, Quaternion *r);

class GameObject { // not virtual (clone wouldn't work)
public:
  GameObject() {
    layers = 1;
    type = GAMEOBJECT_UNDEFINED;
    sprintf(name,NONAME);
    selected = false;
    anchor.set(0,0,0);
    translation.set(0,0,0);
    scaling.set(1,1,1);
    color = 0xFFFFFFFF;
    sectionOn = true;
    sectionVisible = true;
    showLinks = false;
    visible1 = true;
    visible2 = true;
    uniqueId = currentUniqueGameObjectId++;
    useGrid = false;
    gridId = 0;
    hasDrawProblem = false;
    parentTransform.identity();
    boundingBoxMin.set(-1,-1,-1);
    boundingBoxMax.set(1,1,1);
    outlinerShouldFocusThis = false;
    fileBindingId = 0;
    memset(isCamera,0,sizeof(isCamera));
    billBoardRotateZ = false;
    billBoardScale = 1.0;
    billBoardAspect = 1.0;
    billBoardRotation = 0;
    billBoardTX0 = 0;
    billBoardTY0 = 0;
    billBoardTX1 = 0;
    billBoardTY1 = 0;
  }
  ~GameObject();

  unsigned int layers;
  unsigned int type;
  Vector translation;
  Vector scaling;
  Quaternion rotation;
  Vector anchor;
  unsigned int color;
  char name[NAMEMAX];
  bool selected;
  bool sectionOn;
  bool showLinks;
  bool sectionVisible;
  bool visible1;
  bool visible2;
  bool hasDrawProblem;
  bool useGrid;
  int gridId;
  unsigned int uniqueId;
  Vector boundingBoxMin;
  Vector boundingBoxMax;
  bool outlinerShouldFocusThis;
  Matrix parentTransform;
  unsigned int fileBindingId;
  bool billBoardRotateZ;
  float billBoardRotation;
  float billBoardScale;
  float billBoardAspect;
  int billBoardTX0;
  int billBoardTY0;
  int billBoardTX1;
  int billBoardTY1;
  bool isCamera[CAMERA_COUNT];
  Array<GameObjectProperty> properties;
  Array<unsigned int> uniqueLinkIds;
  Camera camera;

  GameObject *setName(const char *name);
  GameObject *setType(unsigned int type);
  GameObjectProperty *addProperty(const char *name);
  GameObjectProperty *addProperty(GameObjectProperty *property);
  GameObject *addFloat(const char *name, float v);
  GameObject *addInt(const char *name, int v);
  GameObject *addBool(const char *name, bool v);
  GameObject *addVector(const char *name, Vector &v);
  GameObject *addText(const char *name, const char *text);
  GameObject *addColor(const char *name, unsigned int color);
  GameObject *clone();
  Matrix getRenderTransform() const;
  Matrix getObjectTransform() const;
  GameObject *loadSkyBox(const char *fileName);
  GameObject *loadMesh(const char *fileName);
  GameObject *loadBillBoard(const char *fileName);
  GameObject *setCamera(const Camera *camera);

  Array<BakedVertex> *getBakedTriangles();
};

#define S_GAMEOBJECT(__name__,__o__)\
  S_OBJECT_OPEN(__name__);\
  S_UINT("layers",(__o__)->layers);\
  S_UINT("type",(__o__)->type);\
  S_VECTOR("translation",(__o__)->translation);\
  S_VECTOR("scaling",(__o__)->scaling);\
  S_QUATERNION("rotation",(__o__)->rotation);\
  S_VECTOR("anchor",(__o__)->anchor);\
  S_UINT("color",(__o__)->color);\
  S_NAME("name",(__o__)->name);\
  S_BOOL("selected",(__o__)->selected);\
  S_BOOL("selectionOn",(__o__)->sectionOn);\
  S_BOOL("showLinks",(__o__)->showLinks);\
  S_BOOL("sectionVisible",(__o__)->sectionVisible);\
  S_BOOL("visible1",(__o__)->visible1);\
  S_BOOL("visible2",(__o__)->visible2);\
  S_BOOL("hasDrawProblem",(__o__)->hasDrawProblem);\
  S_BOOL("useGrid", (__o__)->useGrid);\
  S_INT("gridId", (__o__)->gridId);\
  S_UINT("uniqueId", (__o__)->uniqueId);\
  S_VECTOR("boundingBoxMin",(__o__)->boundingBoxMin);\
  S_VECTOR("boundingBoxMax",(__o__)->boundingBoxMax);\
  S_BOOL("outlinerShouldFocusThis",(__o__)->outlinerShouldFocusThis);\
  S_MATRIX("parentTransform",(__o__)->parentTransform);\
  S_UINT("fileBindingId",(__o__)->fileBindingId);\
  S_BOOL("billBoardRotateZ",(__o__)->billBoardRotateZ);\
  S_FLOAT("billBoardRotation",(__o__)->billBoardRotation);\
  S_FLOAT("billBoardScale",(__o__)->billBoardScale);\
  S_FLOAT("billBoardAspect",(__o__)->billBoardAspect);\
  S_INT("billBoardTX0",(__o__)->billBoardTX0);\
  S_INT("billBoardTY0",(__o__)->billBoardTY0);\
  S_INT("billBoardTX1",(__o__)->billBoardTX1);\
  S_INT("billBoardTY1",(__o__)->billBoardTY1);\
  S_CARRAY("isCamera",(__o__)->isCamera,S_BOOL(NULL,(__o__)->isCamera[i]),CAMERA_COUNT);\
  S_ARRAY("properties",(__o__)->properties,S_GAMEOBJECTPROPERTY(NULL,&(__o__)->properties[i]));\
  S_ARRAY("uniqueLinkIds",(__o__)->uniqueLinkIds,S_UINT(NULL,(__o__)->uniqueLinkIds[i]));\
  S_CAMERA("camera",&(__o__)->camera);\
  S_OBJECT_CLOSE();

extern HashMap<unsigned int, const char*> iconNameForGameObjectType;
extern Array<GameObject*> gameObjects;

void initGameObjects();
GameObject *addGameObject(const char *name);
void deleteGameObject(int index);
void freeGameObjects();

void cloneName(char *destName, const char *originalName);

int getObjectIdByUniqueId(unsigned int uniqueId);

#endif //__GAMEOBJ_HPP__

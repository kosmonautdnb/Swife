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

extern unsigned int currentUniqueGameObjectId;

class Grid {
public:
  Grid() {
    posXEnabled = true;
    posYEnabled = true;
    posZEnabled = true;
    scaleXEnabled = true;
    scaleYEnabled = true;
    scaleZEnabled = true;
    rotEnabled = true;
    posGranularity.set(1,1,1);
    scaleGranularity.set(1,1,1);
    rotGranularity = 45;
  }
  bool posXEnabled;
  bool posYEnabled;
  bool posZEnabled;
  bool scaleXEnabled;
  bool scaleYEnabled;
  bool scaleZEnabled;
  bool rotEnabled;
  Vector posGranularity;
  Vector scaleGranularity;
  double rotGranularity;
};

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
  GameObject *setName(const char *name);
  GameObject *setType(unsigned int type);
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
  int uniqueId;
  Vector boundingBoxMin;
  Vector boundingBoxMax;
  bool outlinerShouldFocusThis;
  Array<GameObjectProperty> properties;
  Array<unsigned int> uniqueLinkIds;
  Matrix parentTransform;
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
  unsigned int fileBindingId;

  GameObject *loadSkyBox(const char *fileName);
  GameObject *loadMesh(const char *fileName);
  GameObject *loadBillBoard(const char *fileName);
  GameObject *setCamera(const Camera *camera);

  Array<BakedVertex> *getBakedTriangles();

  Camera camera;
  bool isCamera[CAMERA_COUNT];
  bool billBoardRotateZ;
  float billBoardRotation;
  float billBoardScale;
  float billBoardAspect;
  int billBoardTX0;
  int billBoardTY0;
  int billBoardTX1;
  int billBoardTY1;
};

extern HashMap<unsigned int, const char*> iconNameForGameObjectType;
extern Array<GameObject*> gameObjects;

void initGameObjects();
GameObject *addGameObject(const char *name);
void deleteGameObject(int index);
void freeGameObjects();

void cloneName(char *destName, const char *originalName);

int getObjectIdByUniqueId(unsigned int uniqueId);

#endif //__GAMEOBJ_HPP__

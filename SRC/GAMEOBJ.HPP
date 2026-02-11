#ifndef __GAMEOBJ_HPP__
#define __GAMEOBJ_HPP__

#include "CONFIG.HPP"
#include "STL/ARRAY.HPP"
#include "STL/HASHMAP.HPP"
#include "STL/VECTOR.HPP"
#include "STL/QUATERNN.HPP"
#include "STL/MATRIX.HPP"
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

class GameObject { // not virtual (clone wouldn't work)
public:
  GameObject() {
    layers = 1;
    type = GAMEOBJECT_UNDEFINED;
    sprintf(name,NONAME);
    sprintf(fileName,NONAME);
    selected = false;
    translation.set(0,0,0);
    scaling.set(1,1,1);
    color = 0xFFFFFFFF;
    sectionOn = true;
    sectionVisible = true;
    showLinks = false;
    visible1 = true;
    visible2 = true;
    uniqueId = currentUniqueGameObjectId++;
    hasDrawProblem = false;
    parentTransform.identity();
    boundingBoxMin.set(-1,-1,-1);
    boundingBoxMax.set(1,1,1);
    outlinerShouldFocusThis = false;
    fileBindingId = 0;
  }
  ~GameObject();
  GameObject *setName(const char *name);
  GameObject *setType(unsigned int type);
  unsigned int layers;
  unsigned int type;
  Vector translation;
  Vector scaling;
  Quaternion rotation;
  unsigned int color;
  char name[NAMEMAX];
  char fileName[FILENAMEMAX];
  bool selected;
  bool sectionOn;
  bool showLinks;
  bool sectionVisible;
  bool visible1;
  bool visible2;
  bool hasDrawProblem;
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
  Matrix getRenderTransform();
  Matrix getObjectTransform();
  unsigned int fileBindingId;

  GameObject *loadSkyBox(const char *fileName);
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

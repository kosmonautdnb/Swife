#ifndef __GAMEOBJ_HPP__
#define __GAMEOBJ_HPP__

#include "CONFIG.HPP"
#include "STL/ARRAY.HPP"
#include "STL/HASHMAP.HPP"
#include "STL/VECTOR.HPP"
#include <stdio.h>

enum GAMEOBJECTTYPE {
  GAMEOBJECT_UNDEFINED = 0,
  GAMEOBJECT_MESH = 1,
  GAMEOBJECT_CAMERA = 2,
  GAMEOBJECT_SKYBOX = 3,
  GAMEOBJECT_PATH = 4,
  GAMEOBJECT_BILLBOARD = 5,
  GAMEOBJECT_COUNT = 6
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
    rotation.set(0,0,0);
  }
  ~GameObject();
  GameObject *setName(const char *name);
  GameObject *setType(unsigned int type);
  unsigned int layers;
  unsigned int type;
  Vector translation;
  Vector scaling;
  Vector rotation;
  char name[NAMEMAX];
  char fileName[FILENAMEMAX];
  bool selected;
  Array<GameObjectProperty> properties;
  GameObjectProperty *addProperty(const char *name);
  GameObjectProperty *addProperty(GameObjectProperty *property);
  GameObject *addFloat(const char *name, float v);
  GameObject *addInt(const char *name, int v);
  GameObject *addBool(const char *name, bool v);
  GameObject *addVector(const char *name, Vector &v);
  GameObject *addText(const char *name, const char *text);
  GameObject *addColor(const char *name, unsigned int color);
  GameObject *clone();
};

extern HashMap<unsigned int, const char*> iconNameForGameObjectType;
extern Array<GameObject*> gameObjects;

void initGameObjects();
GameObject *addGameObject(const char *name);
void deleteGameObject(int index);

void cloneName(char *destName, const char *originalName);

#endif //__GAMEOBJ_HPP__

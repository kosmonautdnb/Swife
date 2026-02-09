#include "GAMEOBJ.HPP"
#include "RENDER.HPP"
#include "PAINT.HPP"
#include <string.h>

HashMap<unsigned int, const char*> iconNameForGameObjectType;

const char *gameObjectTypeNames[GAMEOBJECT_COUNT] = {"undefined", "mesh", "camera", "skybox", "path", "billboard","section","trigger"};
const char *propertyTypeNames[GAMEOBJECTPROPERTY_COUNT] = {"undefined", "bool", "int", "float", "float4", "text", "color"};

void GameObjectProperty::setName(const char *name) {
  sprintf(this->name,"%s",name);
}

void GameObjectProperty::clearValue() {
  memset(goText,0,sizeof(goText));
}

GameObject::~GameObject() {
}

GameObjectProperty *GameObject::addProperty(const char *name) {
  properties.push_back(GameObjectProperty());
  properties.back().setName(name);
  return &properties.back();
}

GameObjectProperty *GameObject::addProperty(GameObjectProperty *property) {
  properties.push_back(*property);
  return &properties.back();
}

GameObject *GameObject::addFloat(const char *name, float v) {
  GameObjectProperty *p = addProperty(name);
  p->goFloat = v;
  p->type = GAMEOBJECTPROPERTY_FLOAT;
  return this;
}

GameObject *GameObject::addInt(const char *name, int v) {
  GameObjectProperty *p = addProperty(name);
  p->goInt = v;
  p->type = GAMEOBJECTPROPERTY_INT;
  return this;
}

GameObject *GameObject::addBool(const char *name, bool v) {
  GameObjectProperty *p = addProperty(name);
  p->goBool = v;
  p->type = GAMEOBJECTPROPERTY_BOOL;
  return this;
}

GameObject *GameObject::addVector(const char *name, Vector &v) {
  GameObjectProperty *p = addProperty(name);
  p->goFloat4[0] = v.x;
  p->goFloat4[1] = v.y;
  p->goFloat4[2] = v.z;
  p->goFloat4[3] = v.w;
  p->type = GAMEOBJECTPROPERTY_FLOAT4;
  return this;
}

GameObject *GameObject::addText(const char *name, const char *text) {
  GameObjectProperty *p = addProperty(name);
  sprintf(p->goText,"%s",text);
  p->type = GAMEOBJECTPROPERTY_TEXT;
  return this;
}

GameObject *GameObject::addColor(const char *name, unsigned int v) {
  GameObjectProperty *p = addProperty(name);
  p->goColor = v;
  p->type = GAMEOBJECTPROPERTY_COLOR;
  return this;
}

void initGameObjects() {
  iconNameForGameObjectType[GAMEOBJECT_UNDEFINED] = "undefined";
  iconNameForGameObjectType[GAMEOBJECT_MESH] = "3d";
  iconNameForGameObjectType[GAMEOBJECT_CAMERA] = "camera";
  iconNameForGameObjectType[GAMEOBJECT_SKYBOX] = "skybox";
  iconNameForGameObjectType[GAMEOBJECT_PATH] = "path";
  iconNameForGameObjectType[GAMEOBJECT_BILLBOARD] = "billboard";
  iconNameForGameObjectType[GAMEOBJECT_TRIGGER] = "trigger";
}

GameObject *GameObject::setName(const char *name) {
  sprintf(this->name,name);
  return this;
}

GameObject *GameObject::setType(unsigned int type) {
  this->type = type;
  return this;
}

GameObject *GameObject::clone() {
  GameObject *o2 = new GameObject();
  memcpy(o2,this,sizeof(GameObject));
  return o2;
}

GameObject *GameObject::loadSkyBox() {
  ::loadSkyBox(this);
  renderingUpdated = true;
  return this;
}

Array<GameObject*> gameObjects;

GameObject *addGameObject(const char *name) {
  GameObject *o = new GameObject();
  gameObjects.push_back(o);
  o->setName(name);
  renderingUpdated = true;
  return o;
}

void deleteGameObject(int index) {
  delete gameObjects[index];
  gameObjects.erase(index,1);
  renderingUpdated = true;
}

void cloneName(char *destName, const char *originalName) {
  int l = strlen(originalName);
  if (l>NAMEMAX-2)l=NAMEMAX-2;
  memcpy(destName+1,originalName,l);
  destName[l+1]=0;
  destName[0]='_';
}

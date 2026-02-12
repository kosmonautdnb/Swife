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
#include "GL.H"
#include "GAMEOBJ.HPP"
#include "RENDER.HPP"
#include "PAINT.HPP"
#include "SERIALZE.HPP"
#include <string.h>
#include <math.h>

HashMap<unsigned int, const char*> iconNameForGameObjectType;
Grid grids[MAXGRIDS];

const char *gameObjectTypeNames[GAMEOBJECT_COUNT] = {"undefined", "mesh", "camera", "skybox", "path", "billboard","section","trigger","link"};
const char *propertyTypeNames[GAMEOBJECTPROPERTY_COUNT] = {"undefined", "bool", "int", "float", "float4", "text", "color"};

void GameObjectProperty::setName(const char *name) {
  sprintf(this->name,"%s",name);
}

void GameObjectProperty::clearValue() {
  memset(goText,0,sizeof(goText));
}

unsigned int currentUniqueGameObjectId = 1;

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
  iconNameForGameObjectType[GAMEOBJECT_LINK] = "link";
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
  unsigned int a = o2->uniqueId;
  memcpy(o2,this,sizeof(GameObject));
  o2->uniqueId = a;
  return o2;
}

GameObject *GameObject::loadSkyBox(const char *fileName) {
  loadSkyBoxForObject(this,fileName);
  return this;
}

GameObject *GameObject::loadMesh(const char *fileName) {
  loadMeshForObject(this,fileName);
  return this;
}

GameObject *GameObject::setCamera(const Camera *camera) {
  this->camera = *camera;
  return this;
}

Matrix GameObject::getRenderTransform() const {
  return parentTransform * getObjectTransform();
}

double quantize(double v, double g) {
  if (g == 0) return v;
  return floor((v / g)+0.5)*g;
}

void applyGrid(Grid *grid, Vector *t, Vector *s, Quaternion *r) {
  if (grid->posXEnabled) t->x = quantize(t->x,grid->posGranularity.x);
  if (grid->posYEnabled) t->y = quantize(t->y,grid->posGranularity.y);
  if (grid->posZEnabled) t->z = quantize(t->z,grid->posGranularity.z);
  if (grid->scaleXEnabled) s->x = quantize(s->x,grid->scaleGranularity.x);
  if (grid->scaleYEnabled) s->y = quantize(s->y,grid->scaleGranularity.y);
  if (grid->scaleZEnabled) s->z = quantize(s->z,grid->scaleGranularity.z);
  if (grid->rotEnabled) {
    Vector k = r->getYawPitchRoll();
    k *= 180.0/PI;
    k.x = quantize(k.x, grid->rotGranularity);
    k.y = quantize(k.y, grid->rotGranularity);
    k.z = quantize(k.z, grid->rotGranularity);
    k /= 180.0/PI;
    r->yawPitchRoll(k.x,k.y,k.z);
  }
}

Matrix GameObject::getObjectTransform() const {
  if (type == GAMEOBJECT_CAMERA) {
    Matrix ret;
    int mode; glGetIntegerv(GL_MATRIX_MODE, &mode);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(camera.position.x,camera.position.y,camera.position.z);
    glMultMatrixd(camera.rotation.rotationMatrix().m);
    glScalef(scaling.x,scaling.y,scaling.z);
    glGetDoublev(GL_MODELVIEW_MATRIX, ret.m);
    glPopMatrix();
    glMatrixMode(mode);
    return ret;

  }

  Vector translation2 = translation;
  Vector scaling2 = scaling;
  Quaternion rotation2 = rotation;
  if (useGrid) applyGrid(&grids[gridId],&translation2,&scaling2,&rotation2);
  Matrix ret;
  int mode; glGetIntegerv(GL_MATRIX_MODE, &mode);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(translation2.x,translation2.y,translation2.z);
  glMultMatrixd(rotation2.rotationMatrix().m);
  glScalef(scaling2.x,scaling2.y,scaling2.z);
  glTranslatef(-anchor.x,-anchor.y,-anchor.z);
  glGetDoublev(GL_MODELVIEW_MATRIX, ret.m);
  glPopMatrix();
  glMatrixMode(mode);
  return ret;
}

Array<BakedVertex> *GameObject::getBakedTriangles() {
  FileBinding *binding = getBindingById(fileBindingId);
  if (binding != NULL) {
    boundingBoxMin = binding->boundingBoxMin;
    boundingBoxMax = binding->boundingBoxMax;
    return &binding->triangles;
  }
  return NULL;
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

void freeGameObjects() {
  for (int i = 0; i < gameObjects.size(); i++) {
    delete gameObjects[i];
  }
  gameObjects.clear();
}

void cloneName(char *destName, const char *originalName) {
  int l = strlen(originalName);
  if (l>NAMEMAX-2)l=NAMEMAX-2;
  memcpy(destName+1,originalName,l);
  destName[l+1]=0;
  destName[0]='_';
}

int getObjectIdByUniqueId(unsigned int uniqueId) {
  int ret = -1;
  for (int i = 0; i < gameObjects.size(); i++) if (gameObjects[i]->uniqueId == uniqueId) return i;
  return ret;
}

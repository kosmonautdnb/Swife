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

GameObject *GameObject::loadBillBoard(const char *fileName) {
  loadBillBoardForObject(this,fileName);
  return this;
}

GameObject *GameObject::setCamera(const Camera *camera) {
  this->camera = *camera;
  return this;
}

void getCameraMatrices(Matrix *projection, Matrix *modelView) {
  int mode; glGetIntegerv(GL_MATRIX_MODE, &mode);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  cameras[currentCamera].applyToGL();
  glGetDoublev(GL_PROJECTION_MATRIX,projection->m);
  glGetDoublev(GL_MODELVIEW_MATRIX,modelView->m);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(mode);
}

Matrix toBillBoardTransform(const GameObject *o, const Matrix &m) {
  Matrix ret; 
  Matrix t; t.identity(); t.translate(m.getColumn(3));
  Matrix projection,modelView; getCameraMatrices(&projection,&modelView);
  Matrix invCam = inverse(modelView);
  double rot = o->billBoardRotation*PI/180.0;
  if (o->billBoardRotateZ) {
    Vector p0 = projection*modelView*m*Vector(0,0,0);
    Vector p1 = projection*modelView*m*Vector(0,1,0);
    Vector p2 = projection*modelView*m*Vector(1,0,0);
    if (p0.w != 0) p0 /= p0.w;
    if (p1.w != 0) p1 /= p1.w;
    if (p2.w != 0) p2 /= p2.w;
    p1 -= p0;
    p2 -= p0;
    if (length(p1)>length(p2))
       rot -= atan2(-p1.y,p1.x)+PI*0.5;
    else
       rot -= atan2(-p2.y,p2.x)+PI*0.5;
  }
  Matrix angleZ = rotationMatrixAngleAxis(rot, 0, 0, 1);
  double aspect = o->billBoardAspect; if (aspect == 0) aspect = 1;
  Matrix scale; scale.identity(); scale.scale3(Vector(o->billBoardScale,o->billBoardScale*aspect,0.001)); // z needs a bit
  ret = t * invCam.rotationMatrix3x3() * angleZ * scale;
  return ret;
}

Matrix GameObject::getRenderTransform() const {
  Matrix ret = parentTransform * getObjectTransform();
  if (type == GAMEOBJECT_BILLBOARD) ret = toBillBoardTransform(this, ret);
  return ret;
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
  if (type == GAMEOBJECT_BILLBOARD) ret = toBillBoardTransform(this, ret);
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

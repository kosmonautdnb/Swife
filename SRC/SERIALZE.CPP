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
#include "SERIALZE.HPP"
#include "DIALOGS.HPP"
#include "GAMEOBJ.HPP"
#include "STL/SET.HPP"
#include "STL/IMAGE.HPP"
#include "TEXTURES.HPP"
#include "RENDER.HPP"
#define CGLTF_IMPLEMENTATION
#include "cgltf.hpp"
#include "STL/SMPLOBJL.HPP"
#include <math.h>

char *fileTypeNames[FILETYPE_COUNT] = {"none","skybox","mesh"};

FileBinding::~FileBinding() {
  free();
}

void FileBinding::load(const char *name, unsigned int type) {
  this->fileName = name;
  this->type = type;
  reload();
}

Array<unsigned int> allGenTexturesHere;
unsigned int OBJloadTexture(const String &fileName, const String &type) {
  unsigned int texture = 0;
  if (type != "map_Kd") return texture;
  String name = shortFileName(fileName);
  RGBAImage image = RGBAImage::fromFile(name.c_str());
  if (image.data == NULL) return texture;
  correctTextureSize(&image);
  glGenTextures(1, &texture);
  allGenTexturesHere.push_back(texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,image.width,image.height,0,GL_RGBA,GL_UNSIGNED_BYTE,image.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  image.free();
  return texture;
}

unsigned int vectorToColor(const Vector &v) {
  int r = (int)floor(v.x * 255.0+0.5);
  int g = (int)floor(v.y * 255.0+0.5);
  int b = (int)floor(v.z * 255.0+0.5);
  int a = (int)floor(v.w * 255.0+0.5);
  if (r < 0) r = 0; if (r > 255) r = 255;
  if (g < 0) g = 0; if (g > 255) g = 255;
  if (b < 0) b = 0; if (b > 255) b = 255;
  if (a < 0) a = 0; if (a > 255) a = 255;
  return r|(g<<8)|(b<<16)|(a<<24);
}

bool FileBinding::reload() {
  free();
  renderingUpdated = true;
  switch(type) {
  case FILETYPE_SKYBOX: {RGBAImage s = RGBAImage::fromFile(fileName.c_str());
      if (s.data == NULL) {dialogMessage(true,(String("Couldn't load file:<")+fileName+">").c_str()); return false;}
      textureIds.resize(6); 
      skyBoxFromHorizontalCrossImage(&textureIds[0], &s);
      s.free();
      return true;
    } break;
  case FILETYPE_MESH: {
      String a = toLower(this->fileName);
      if (a.endsWith(".obj")) { // WAVEFRONT OBJECT IMPORT
        SMPL_File *obj = loadObj(fileName.c_str(), true);
        if (obj == NULL) return false;
        allGenTexturesHere.clear();
        obj->loadTextures(OBJloadTexture);
        textureIds = allGenTexturesHere;
        bool first = true;
        boundingBoxMin.set(-1,-1,-1);
        boundingBoxMax.set(1,1,1);
        for (int o = 0; o < obj->objs.size(); o++) {
          SMPL_Object *so = &obj->objs[o];
          for (int f = so->faceStart; f < so->faceEnd; f++) {
            SMPL_Face *fo = &obj->faces[f];
            unsigned int _m = fo->m;
            SMPL_Material *mat = NULL;
            if (obj->materialNames.has(_m)) {
              const String &materialName = obj->materialNames[_m];
              mat = &obj->materials[materialName];
            }
            for (int t = 0; t < 3; t++) {
              Vector *_p = &obj->vertices[fo->v[t]];
              if (first||_p->x<boundingBoxMin.x) boundingBoxMin.x = _p->x;
              if (first||_p->y<boundingBoxMin.y) boundingBoxMin.y = _p->y;
              if (first||_p->z<boundingBoxMin.z) boundingBoxMin.z = _p->z;
              if (first||_p->x>boundingBoxMax.x) boundingBoxMax.x = _p->x;
              if (first||_p->y>boundingBoxMax.y) boundingBoxMax.y = _p->y;
              if (first||_p->z>boundingBoxMax.z) boundingBoxMax.z = _p->z;
              first = false;
              Vector *_n = &obj->normals[fo->n[t]];
              Vector *_t = &obj->texCoords[fo->t[t]];
              BakedVertex k;
              k.x = _p->x;
              k.y = _p->y;
              k.z = _p->z;
              k.nx = _n->x;
              k.ny = _n->y;
              k.nz = _n->z;
              k.tx = _t->x;
              k.ty = _t->y;
              k.color = 0xffffffff;
              k.textureId = 0;
              if (mat != NULL) {
                k.color = vectorToColor(mat->diffuse);
                if (mat->mapDiffuse.used && mat->mapDiffuse.glHandle != 0) {
                  k.textureId = mat->mapDiffuse.glHandle;
                }
              }
              triangles.push_back(k);
            }
          }
        }
        delete obj;
        return true;
      }
      return false;
    } break;
  }
  return false;
}

void FileBinding::free() {
  for (int i = 0; i < textureIds.size(); i++) {
    glDeleteTextures(1,&textureIds[i]);
  }
  textureIds.clear();
  triangles.clear();
}

unsigned int bindingUniqueId = 1;
Array<FileBinding*> bindings;

FileBinding *getBindingById(unsigned int uniqueId) {
  for (int i = 0; i < bindings.size(); i++) {
    if (bindings[i]->uniqueId == uniqueId) return bindings[i];
  }
  return NULL;
}

FileBinding *getBindingByFileName(const char *fileName, unsigned int type) {
  for (int i = 0; i < bindings.size(); i++) {
    if (bindings[i]->fileName == fileName && bindings[i]->type == type) return bindings[i];
  }
  return NULL;
}

FileBinding *newBinding() {
  FileBinding *ret = new FileBinding();
  bindings.push_back(ret);
  return ret;
}

void clearAllUnusedBindings() {
  Set<unsigned int> usedBindings;
  for (int i = 0; i < gameObjects.size(); i++) {
    usedBindings.insert(gameObjects[i]->fileBindingId);
  }
  for (int j = 0; j < bindings.size(); j++) {
    if (!usedBindings.has(bindings[j]->uniqueId)) {
      delete bindings[j];
      bindings.erase(j,1);
      j--;
    }
  }
}

void freeBindings() {
  for (int i = 0; i < bindings.size(); i++) {
    delete bindings[i];
  }
  bindings.clear();
}

void loadScene(const char *path, const char *fileName) {
}

void saveScene(const char *path, const char *fileName) {
}

void loadSkyBoxForObject(class GameObject *o, const char *fileName) {
  FileBinding *binding = getBindingByFileName(fileName, FILETYPE_SKYBOX);
  if (binding == NULL) {
    binding = newBinding();
    binding->load(fileName, FILETYPE_SKYBOX);
  }
  o->fileBindingId = binding->uniqueId;
}

void loadSkyBox(const char *path, const char *fileName) {
  int j = getObjectIdByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  loadSkyBoxForObject(o,(shortFileName(String(path)+String(fileName))).c_str());
}

void loadMeshForObject(class GameObject *o, const char *fileName) {
  FileBinding *binding = getBindingByFileName(fileName, FILETYPE_MESH);
  if (binding == NULL) {
    binding = newBinding();
    binding->load(fileName, FILETYPE_MESH);
  }
  o->fileBindingId = binding->uniqueId;
}

void loadMesh(const char *path, const char *fileName) {
  int j = getObjectIdByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  loadMeshForObject(o,(shortFileName(String(path)+String(fileName))).c_str());
}

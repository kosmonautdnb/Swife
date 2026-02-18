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
#ifndef __SERIALZE_HPP__
#define __SERIALZE_HPP__

#include "STL/ARRAY.HPP"
#include "STL/VECTOR.HPP"
#include "STL/STRING.HPP"
#include <stdio.h>

enum FILETYPE {
  FILETYPE_NONE = 0,
  FILETYPE_SKYBOX = 1,
  FILETYPE_MESH = 2,
  FILETYPE_BILLBOARD = 3,
  FILETYPE_TEXTUREGRID = 4,
  FILETYPE_COUNT = 5
};
extern const char *fileTypeNames[FILETYPE_COUNT];

extern unsigned int bindingUniqueId;

class BakedVertex {
public:
  float x,y,z;
  float nx,ny,nz;
  float tx,ty;
  unsigned int color;
  unsigned int textureId;
};

class FileBinding {
public:
  FileBinding() {
    uniqueId = bindingUniqueId++;
    type = FILETYPE_NONE;
    boundingBoxMin.set(-1,-1,-1);
    boundingBoxMax.set(1,1,1);
    billBoardIsTexture = false;
    originalWidth = 0;
    originalHeight = 0;
    spriteObject = NULL;
    textureGridWidth = 0;
    textureGridHeight = 0;
    textureAtlasTileWidth = 0;
    textureAtlasTileHeight = 0;
    textureAtlasTileCountX = 0;
    textureAtlasTileCountY = 0;
    selected = -1;
    index = -1;
    index2 = -1;
    forceTwoSided = false;
  }
  ~FileBinding();
  unsigned int type;
  String fileName;
  unsigned int uniqueId;
  Vector boundingBoxMin;
  Vector boundingBoxMax;
  bool billBoardIsTexture;
  int originalWidth;
  int originalHeight;
  int textureGridWidth;
  int textureGridHeight;
  int textureAtlasTileWidth;
  int textureAtlasTileHeight;
  int textureAtlasTileCountX;
  int textureAtlasTileCountY;
  int selected;
  int index;
  int index2;
  bool forceTwoSided;
  Array<unsigned int> textureIds;
  Array<BakedVertex> triangles;
  Array<unsigned int> uintContent;
  Array<float> floatContent;
  class SpriteObject *spriteObject;
  void load(const char *fileName, unsigned int type);
  bool reload();
  void free();
  FileBinding *clone();
  void recalcTextureGridBoundingBox();
  void resizeTextureGrid(int ofsX, int ofsY, int newX, int newY);
};
#define S_FILEBINDING(__name__,__o__)\
  S_OBJECT_OPEN(__name__);\
  S_UINT("type", (__o__)->type);\
  S_FILENAME("fileName",(__o__)->fileName);\
  S_UINT("uniqueId",(__o__)->uniqueId);\
  S_VECTOR("boundingBoxMin",(__o__)->boundingBoxMin);\
  S_VECTOR("boundingBoxMax",(__o__)->boundingBoxMax);\
  S_BOOL("billBoardIsTexture",(__o__)->billBoardIsTexture);\
  S_INT("originalWidth",(__o__)->originalWidth);\
  S_INT("originalHeight",(__o__)->originalHeight);\
  S_INT("textureGridWidth",(__o__)->textureGridWidth);\
  S_INT("textureGridHeight",(__o__)->textureGridHeight);\
  S_INT("textureAtlasTileWidth",(__o__)->textureAtlasTileWidth);\
  S_INT("textureAtlasTileHeight",(__o__)->textureAtlasTileHeight);\
  S_INT("textureAtlasTileCountX",(__o__)->textureAtlasTileCountX);\
  S_INT("textureAtlasTileCountY",(__o__)->textureAtlasTileCountY);\
  S_INT("selected",(__o__)->selected);\
  S_INT("index",(__o__)->index);\
  S_INT("index2",(__o__)->index2);\
  S_BOOL("forceTwoSided",(__o__)->forceTwoSided);\
  S_ARRAY("uintContent",(__o__)->uintContent,S_UINT(NULL,(__o__)->uintContent[i]));\
  S_ARRAY("floatContent",(__o__)->floatContent,S_FLOAT(NULL,(__o__)->floatContent[i]));\
  S_OBJECT_CLOSE();

extern Array<FileBinding*> bindings;
FileBinding *getBindingById(unsigned int uniqueId);
FileBinding *getBindingByFileName(const char *fileName, unsigned int type);
FileBinding *newBinding();
void clearAllUnusedBindings();
void freeBindings();

void loadScene(const char *path, const char *fileName);
void saveScene(const char *path, const char *fileName);

void loadSkyBoxForObject(class GameObject *o, const char *fileName);
void loadSkyBox(const char *path, const char *fileName);
void loadMeshForObject(class GameObject *o, const char *fileName);
void loadMesh(const char *path, const char *fileName);
void loadBillBoardForObject(class GameObject *o, const char *fileName);
void loadBillBoard(const char *path, const char *fileName);
void loadTextureGridForObject(class GameObject *o, const char *fileName);
void loadTextureGrid(const char *path, const char *fileName);
void loadTextureGridCSV(const char *path, const char *fileName);
void saveTextureGridCSV(const char *path, const char *fileName);
void serializeAll(FILE *file, bool save);

#endif //__SERIALZE_HPP__

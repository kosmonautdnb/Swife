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
#include "STL/DOS.HPP"
#include <stdio.h>

enum FILETYPE {
  FILETYPE_NONE = 0,
  FILETYPE_SKYBOX = 1,
  FILETYPE_MESH = 2,
  FILETYPE_BILLBOARD = 3,
  FILETYPE_TEXTUREGRID = 4,
  FILETYPE_TEXTUREATLAS = 5,
  FILETYPE_HEIGHTMAP = 6,
  FILETYPE_OVERLAY = 7,
  FILETYPE_COUNT = 8
};
extern const char *fileTypeNames[FILETYPE_COUNT];

extern unsigned int bindingUniqueId;
extern int version;

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
    fileName = "";
    fileName2 = "";
    updated = false;
    doubleParameter = 0;
    boolParameter = false;
  }
  ~FileBinding();
  unsigned int type;
  String fileName;
  String fileName2;
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
  bool updated;
  double doubleParameter;
  bool boolParameter;
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
  S_FILENAME("fileName2",(__o__)->fileName2);\
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
  S_DOUBLE("doubleParameter",(__o__)->doubleParameter);\
  S_BOOL("boolParameter",(__o__)->boolParameter);\
  S_BOOL("forceTwoSided",(__o__)->forceTwoSided);\
  S_ARRAY("uintContent",(__o__)->uintContent,S_UINT(NULL,(__o__)->uintContent[i]));\
  S_ARRAY("floatContent",(__o__)->floatContent,S_FLOAT(NULL,(__o__)->floatContent[i]));\
  S_OBJECT_CLOSE();

extern Array<FileBinding*> bindings;
FileBinding *getBindingById(unsigned int uniqueId);
FileBinding *getBindingByFileName(class GameObject *o, const char *fileName, unsigned int type);
FileBinding *newBinding();
void clearAllUnusedBindings();
void freeBindings();
void serializeAll(FILE *file, bool save);

void reloadAll();

void createDataPointForNew();
void restoreByDataPointForNew();

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
void loadTextureAtlasForObject(class GameObject *o, const char *fileName);
void loadTextureAtlas(const char *path, const char *fileName);
void loadTextureGridCSV(const char *path, const char *fileName);
void saveTextureGridCSV(const char *path, const char *fileName);
void loadScript(const char *path, const char *fileName);
void saveScript(const char *path, const char *fileName);
void loadGameObjects(const char *path, const char *fileName);
void saveGameObjects(const char *path, const char *fileName);
void saveImage(const char *path, const char *fileName);
void loadHeightmapForObject(class GameObject *o, const char *fileName);
void loadHeightmap(const char *path, const char *fileName);
void loadOverlayForObject(class GameObject *o, const char *fileName);
void loadOverlay(const char *path, const char *fileName);

void exportObject(GameObject *o);
void exportObjectCSV(const char *path, const char *fileName);

double updateToolTip(unsigned int *rgba, int width, int height, const char *path, const char *fileName);

String toLocalFileName(const String &str);
extern const char *zipFolderPrefix;
extern const char *zipFolderPrefix2;
extern String lastZipFile;
extern String lastZippedFile;
String decompressFileBegin(const String &maybeExtendedFileName);
void decompressFileEnd(const String &fileName);
void getFileArray(Array<unsigned char> *arr,const String &maybeExtendedFileName);
String getFileString(const String &maybeExtendedFileName);
void unpackZipFileToFile(const char *tempFileName, const char *zipFileName, const char *zippedFileName);
void unpackZipFileToArray(Array<unsigned char> *arr, const char *zipFileName, const char *zippedFileName);
Array<File> getZipDirectory(const char *path, const char *zipFileName);
void patchFileName(String *fileName, const String &zipName, const String &zipFileName);
void patchBinding(class FileBinding *b, const String &zipName, const String &zipFileName);

#endif //__SERIALZE_HPP__

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

enum FILETYPE {
  FILETYPE_NONE = 0,
  FILETYPE_SKYBOX = 1,
  FILETYPE_MESH = 2,
  FILETYPE_BILLBOARD = 3,
  FILETYPE_COUNT = 4
};
extern char *fileTypeNames[FILETYPE_COUNT];

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
    billBoardOriginalWidth = 0;
    billBoardOriginalHeight = 0;
  }
  ~FileBinding();
  unsigned int type;
  String fileName;
  unsigned int uniqueId;
  Array<unsigned int> textureIds;
  Vector boundingBoxMin;
  Vector boundingBoxMax;
  Array<BakedVertex> triangles;
  bool billBoardIsTexture;
  int billBoardOriginalWidth;
  int billBoardOriginalHeight;
  void load(const char *fileName, unsigned int type);
  bool reload();
  void free();
};

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

#endif //__SERIALZE_HPP__

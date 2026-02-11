#include "GL.H"
#include "SERIALZE.HPP"
#include "DIALOGS.HPP"
#include "GAMEOBJ.HPP"
#include "STL/SET.HPP"
#include "STL/IMAGE.HPP"
#include "TEXTURES.HPP"
#include "RENDER.HPP"

char *fileTypeNames[FILETYPE_COUNT] = {"none","skybox"};

FileBinding::~FileBinding() {
  free();
}

void FileBinding::load(const char *name, unsigned int type) {
  this->fileName = name;
  this->type = type;
  reload();
}

void FileBinding::reload() {
  free();
  RGBAImage s = RGBAImage::fromFile(fileName.c_str());
  textureIds.resize(6); 
  skyBoxFromHorizontalCrossImage(&textureIds[0], &s);
  s.free();
  renderingUpdated = true;
}

void FileBinding::free() {
  for (int i = 0; i < textureIds.size(); i++) {
    glDeleteTextures(1,&textureIds[i]);
  }
  textureIds.clear();
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

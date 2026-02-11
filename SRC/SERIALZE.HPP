#ifndef __SERIALZE_HPP__
#define __SERIALZE_HPP__

#include "STL/ARRAY.HPP"
#include "STL/STRING.HPP"

enum FILETYPE {
  FILETYPE_NONE = 0,
  FILETYPE_SKYBOX = 1,
  FILETYPE_COUNT = 2
};
extern char *fileTypeNames[FILETYPE_COUNT];

extern unsigned int bindingUniqueId;

class FileBinding {
public:
  FileBinding() {
    uniqueId = bindingUniqueId++;
    type = FILETYPE_NONE;
  }
  ~FileBinding();
  unsigned int type;
  String fileName;
  unsigned int uniqueId;
  Array<unsigned int> textureIds;
  void load(const char *fileName, unsigned int type);
  void reload();
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

#endif //__SERIALZE_HPP__

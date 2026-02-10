#include "SERIALZE.HPP"
#include "DIALOGS.HPP"
#include "GAMEOBJ.HPP"

void loadScene(const char *path, const char *fileName) {
}

void saveScene(const char *path, const char *fileName) {
}

void loadSkyBox(const char *path, const char *fileName) {
  int j = getObjectIdByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  o->loadSkyBox(shortFileName(String(path)+String(fileName)));
}

#ifndef __DIALOGS_HPP__
#define __DIALOGS_HPP__

#include "STL/STRING.HPP"

extern bool quit;

void initDialogs();

void dialogDeleteGameObjects(bool openIt);
void dialogQuit(bool openIt);
void dialogAddObject(bool openIt, unsigned int type);
void dialogAddProperty(bool openIt, class GameObject *object);
void dialogAbout(bool openIt);
void dialogControls(bool openIt);
void dialogSelectByProperty(bool openIt);
void dialogSelectByLayer(bool openIt);
void dialogSelectInvisible(bool openIt);
void dialogColor(bool openIt, unsigned int *color);
void dialogObjectTextures(bool openIt);

void colorItem(const char *name, const char *id, unsigned int *color, bool colorPicker);

enum DIALOGMODE {
  DIALOG_NONE = 0,
  DIALOG_LOADSCENE = 1,
  DIALOG_SAVESCENE = 2,
  DIALOG_LOADSKYBOX = 3,
  DIALOG_COUNT = 4
};

String longFileName(const String &a);
String shortFileName(const String &a);

extern unsigned int dialogLoadSaveGameObjectUniqueId;
extern String dialogLoadSaveSetFileNameAndPath;
void dialogLoadSave(bool openIt, int dialogMode);

#endif // __DIALOGS_HPP__

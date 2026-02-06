#ifndef __DIALOGS_HPP__
#define __DIALOGS_HPP__

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

#endif // __DIALOGS_HPP__

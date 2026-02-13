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
  DIALOG_LOADMESH = 4,
  DIALOG_LOADBILLBOARD = 5,
  DIALOG_COUNT = 6
};

String longFileName(const String &a);
String shortFileName(const String &a);

extern unsigned int dialogLoadSaveGameObjectUniqueId;
extern String dialogLoadSaveSetFileNameAndPath;
void dialogLoadSave(bool openIt, int dialogMode);

void dialogMessage(bool openIt, const char *alertMessage);
void dialogViewBindings(bool openIt);
void dialogGridSettings(bool openIt);

#endif // __DIALOGS_HPP__

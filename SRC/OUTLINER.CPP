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
#include "OUTLINER.HPP"
#include "IMGUIGL.HPP"
#include "CONFIG.HPP"
#include "STL/STRING.HPP"
#include "GAMEOBJ.HPP"
#include "SVG.HPP"
#include "MENUBAR.HPP"
#include "DIALOGS.HPP"
#include "RENDER.HPP"
#include "TOOLBAR.HPP"

int outliner_width = 160;
char outliner_filter[NAMEMAX] = {0};

unsigned int outlinerLayers = 1;
char selectedGroupName[NAMEMAX];
bool showObjectDates = false;

unsigned int layerSelection(unsigned int a, const char *dcl) {
  char buffer[16];
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5,0));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5);
  for (int i = 0; i < 8; i++) {
    sprintf(buffer,"%c##%s",i+'0',dcl);
    if (i != 0) ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button,a & (1<<i) ? color_button_on_bg:color_button_off_bg);
    ImGui::PushStyleColor(ImGuiCol_Text,a & (1<<i) ? color_button_on_text:color_button_off_text);
    if (ImGui::Button(buffer)) a ^= 1<<i; else drawToolTip("Layer");
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
  }
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
  return a;
}

bool sectionOn;
bool sectionVisible;
int sectionCounter = -1;
bool sectionSelecting;
bool sectionSelectingType;

const char *getOutlinerName(GameObject *o) {
  static char name2[NAMEMAX];
  if (showObjectDates) {
    sprintf(name2,"%s.%s.%s %s:%s",hex2_(o->date & 255).c_str(),hex2_((o->date>>8) & 255).c_str(),hex2_((o->date>>16) & 255).c_str(),hex2_((o->time>>16) & 255).c_str(),hex2_((o->time>>8) & 255).c_str());
    return name2;
  }
  return o->name;
}

void objectSelect(unsigned int i) {
  const unsigned int t = gameObjects[i]->type;
  bool v = gameObjects[i]->visible1 && gameObjects[i]->visible2;
  if (!sectionOn && t != GAMEOBJECT_SECTION) return;

  ImVec2 cursorScreen = ImGui::GetCursorScreenPos();
  String k = String::fromInt(i);
  ImGui::SetCursorScreenPos(ImVec2(cursorScreen.x + outliner_width - k.length()*7-22,cursorScreen.y));
  ImGui::PushStyleColor(ImGuiCol_Text,gameObjects[i]->selected ? ImVec4(0.25,0.25,0.25,1) : ImVec4(0.7,0.7,0.7,1));
  ImGui::Text("%s",k.c_str());
  ImGui::PopStyleColor();
  ImGui::SetCursorScreenPos(cursorScreen);

  bool selected = gameObjects[i]->selected;
  if (t == GAMEOBJECT_SECTION) { 
    if (ImGui::Checkbox(IDINDEXED("##OBJECTSELECTSECTION",i),&gameObjects[i]->sectionOn)) {renderingUpdated = true;}
    drawToolTip(gameObjects[i]->sectionOn?"Collapse section":"Uncollapse section");
    ImGui::SameLine();
    if (ImGui::Checkbox(IDINDEXED("##OBJECTSELECTSECTIONVISIBLE",i),&gameObjects[i]->sectionVisible)) {renderingUpdated = true;}
    drawToolTip(gameObjects[i]->sectionVisible?"Section invisible":"Section visible");
    ImGui::SameLine();
    sectionOn = gameObjects[i]->sectionOn;
    sectionVisible = gameObjects[i]->sectionVisible;
    sectionCounter = gameObjects[i]->sectionLength;
    v = true;
  } else {
    const char *icon = iconNameForGameObjectType[t]; SVG *svg = NULL; if (svgs.has(icon)) svg = &svgs[icon];
    if (svg != NULL) {
      ImGui::Image((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h),ImVec2(0,0),ImVec2(1,1),ImVec4(1,1,1,v?1:0.5));
      drawToolTip("%s",gameObjectTypeNames[t]);
      ImGui::SameLine();
    }
  }
  ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(1,1,1,1));
  ImVec4 tc = selected ? color_text_on : color_text_off;
  if (!v) tc.w *= 0.675;
  ImGui::PushStyleColor(ImGuiCol_Text,tc);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(0.8,0.8,0.8,1));

  if (gameObjects[i]->type == GAMEOBJECT_LINK) {
    ImGui::Checkbox(IDINDEXED("##OBJECTSELECTLINK",i),&gameObjects[i]->showLinks);
    drawToolTip(gameObjects[i]->showLinks?"Hide links":"Show links");
    ImGui::SameLine();
  }

  bool empty = strlen(getOutlinerName(gameObjects[i])) == 0;
  if (ImGui::Button(IDINDEXED(String(empty ? NONAME : getOutlinerName(gameObjects[i]))+String("##OBJECTSELECT"),i))) {
    if (!shift) {
      for (int i = 0; i < gameObjects.size(); i++) {
        gameObjects[i]->selected = false;
      }
    }
    gameObjects[i]->selected = !gameObjects[i]->selected;
    renderingUpdated = true; // needed
    if (shift && t == GAMEOBJECT_SECTION) {sectionSelecting = true; sectionSelectingType = gameObjects[i]->selected;}
    currentSelectedAltuniqueId = gameObjects[i]->uniqueId;
  } else if (showObjectDates) drawToolTipForced("%s",gameObjects[i]->name);

  if (gameObjects[i]->type == GAMEOBJECT_LINK && gameObjects[i]->showLinks) {
    if (gameObjects[i]->uniqueLinkIds.empty()) {
      ImGui::Text("<empty>");
    } else {
      GameObject *object = gameObjects[i];
      for (int i = 0; i < object->uniqueLinkIds.size(); i++) {
        int j = getObjectsIndexByUniqueId(object->uniqueLinkIds[i]);
        if (j >= 0) {
          GameObject *o = gameObjects[j];
          ImGui::Text("%d\"%s\"",j ,getOutlinerName(o));
        } else {
          ImGui::Text("<deleted>");
        }
      }
    }
  }

  ImGui::Separator();
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
}

bool outlinerButton(const char *name, bool enabled) {
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5,0));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);
  ImGui::PushStyleColor(ImGuiCol_Button,enabled ? color_button_on_bg : color_button_off_bg);
  ImGui::PushStyleColor(ImGuiCol_Text,enabled ? color_button_on_text : color_button_off_text);
  bool ret = ImGui::Button(name);
  if (!enabled) ret = false;
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
  return ret;
}

void outlinerMoveUp() {
  if (!gameObjects.empty() && gameObjects[0]->selected) return;
  for (int i = 1; i < gameObjects.size(); i++) {
    if (gameObjects[i]->selected) {
      GameObject *t = gameObjects[i-1];
      gameObjects[i-1] = gameObjects[i];
      gameObjects[i] = t;
    }
  }
}

void outlinerMoveDown() {
  if (!gameObjects.empty() && gameObjects.back()->selected) return;
  for (int i = gameObjects.size()-2; i >= 0; i--) {
    if (gameObjects[i]->selected) {
      GameObject *t = gameObjects[i+1];
      gameObjects[i+1] = gameObjects[i];
      gameObjects[i] = t;
    }
  }
}

void outlinerCompact() {
  int firstIndex = -1;
  for (int i2 = 0; i2 < gameObjects.size(); i2++) {
    if (gameObjects[i2]->selected) {firstIndex = i2; break;}
  }
  for (int i = firstIndex+1; i < gameObjects.size(); i++) {
    if (gameObjects[i]->selected) {
      firstIndex++; 
      GameObject *t = gameObjects[i];
      gameObjects.erase(i,1);
      gameObjects.insert(t,firstIndex);

    }
  }
}

bool gameObjectDeleteButton(bool enabled) {
  SVG *svg = &svgs["delete"];
  ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.9,0.9,0.9,1));
  ImGui::PushID("##DELETEGAMEOBJECTYEAH");
  bool ret = ImGui::ImageButton((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h),ImVec2(0,0),ImVec2(1,1),-1,ImVec4(0,0,0,0),ImVec4(1,1,1,enabled?1:0.25));
  if (!enabled) ret = false;
  ImGui::PopID();
  ImGui::PopStyleColor();
  return ret;
}

void maybeDisplayPlacementTooltip() {
  if (mode != MODE_SELECT) return;
  if (!alt) return;
  ImGui::BeginTooltip();
  ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.0,0.0,0.0,1));
  bool found = false;
  for (int i = 0; i < gameObjects.size(); i++) {
    if (gameObjects[i]->uniqueId == currentSelectedAltuniqueId) {
      if (!ctrl) ImGui::Text("Placing: %d\"%s\"",i,gameObjects[i]->name);
      if (ctrl) ImGui::Text("Placing (-rotation): %d\"%s\"",i,gameObjects[i]->name);
      found = true;
      break;
    }
  }
  if (!found) ImGui::Text("Placing: <not found>");
  ImGui::PopStyleColor();
  ImGui::EndTooltip();
}

void drawOutliner() {
  bool one = false; for (int i2 = 0; i2 < gameObjects.size(); i2++) if (gameObjects[i2]->selected) {one=true; break;}
  ImGui::SetNextWindowPos(ImVec2(0,menuBar_height));
  ImGui::Begin("OUTLINER", NULL, ImVec2(outliner_width,main_window_yres-menuBar_height),1.0,FROZENWINDOW);
  drawDialogs();
  unsigned int outlinerLayersBefore = outlinerLayers;
  outlinerLayers = layerSelection(outlinerLayers,"OUTLLAY");
  if (outlinerLayers != outlinerLayersBefore) renderingUpdated = true;
  ImGui::Separator();
  ImGui::Text(" "); ImGui::SameLine();SVG *svg = &svgs["search"];ImGui::Image((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h));drawToolTip("Search");ImGui::SameLine();ImGui::InputText("##OUTLINERFILTER",outliner_filter,NAMEMAX);
  ImGui::SameLine(); if (gameObjectDeleteButton(one)) dialogDeleteGameObjects(true); else drawToolTip("Delete objects");
  if (outlinerButton("Move up",one)) {renderingUpdated = true;outlinerMoveUp();} else drawToolTip("Move selection up in hierarchy"); ImGui::SameLine(); if (outlinerButton("Move down",one)) {renderingUpdated = true; outlinerMoveDown();} drawToolTip("Move selection down in hierarchy");
  static bool selecty = false;
  if (one) selecty = true;
  bool markAllWithSelecty = false; ImGui::SameLine(); if (ImGui::Checkbox("##OUTLINERSELECTY",&selecty)) {markAllWithSelecty = true;}
  drawToolTip(selecty?"Deselect all":"Select all");
  ImGui::Separator();
  ImGui::BeginChild("OUTLINERCHILD1",ImVec2(0,main_window_yres-ImGui::GetCursorScreenPos().y-18));
  String filter = toLower(String(outliner_filter));
  sectionOn = true;
  sectionVisible = true;
  sectionSelecting = false;
  sectionSelectingType = false;
  int visibles = 0;
  char groupName[NAMEMAX];
  sprintf(selectedGroupName,"%s","");
  sprintf(groupName,"%s","");
  for (int i = 0; i < gameObjects.size(); i++) {
    if (gameObjects[i]->outlinerShouldFocusThis) {ImGui::SetScrollHere(); gameObjects[i]->outlinerShouldFocusThis = false;}
    if (sectionCounter>=0) {
      sectionCounter--;
      if (sectionCounter<0) {
        sectionOn = true;
        sectionVisible = true;
        sectionSelecting = false;
        sectionSelectingType = false;
        sprintf(groupName,"%s","");
      }
    }
    bool selectBefore = gameObjects[i]->selected;
    gameObjects[i]->visible1 = false;
    if (markAllWithSelecty) gameObjects[i]->selected = false;
    if (gameObjects[i]->type == GAMEOBJECT_SECTION && sectionSelecting) {sectionSelecting = false;}
    if (sectionSelecting) gameObjects[i]->selected = sectionSelectingType;
    int l = gameObjects[i]->layers;
    if ((l==0)||(outlinerLayers==0)||(l & outlinerLayers)) {
      bool empty = strlen(getOutlinerName(gameObjects[i])) == 0;
      const char *n = empty ? NONAME : getOutlinerName(gameObjects[i]);
      if (gameObjects[i]->type == GAMEOBJECT_SECTION || filter.empty() || (toLower(n).findFirst(filter)>=0)) {
        if (sectionOn || gameObjects[i]->type == GAMEOBJECT_SECTION) visibles++;
        if (markAllWithSelecty && sectionOn) gameObjects[i]->selected = selecty;
        gameObjects[i]->visible1 = sectionVisible;
        objectSelect(i);
      }
    }
    if (gameObjects[i]->type == GAMEOBJECT_SECTION) 
      sprintf(groupName,"%s",getOutlinerName(gameObjects[i]));
    else if (gameObjects[i]->selected && (strlen(selectedGroupName)==0)) sprintf(selectedGroupName,"%s",groupName);
    if (selectBefore != gameObjects[i]->selected) renderingUpdated = true;
  }
  int selecteds = 0;
  for (int i3 = 0; i3 < gameObjects.size(); i3++)
    if (gameObjects[i3]->selected) selecteds++;

  ImGui::EndChild();
  String k;
  String t = "All:" + String::fromInt(gameObjects.size()) + " ";
  if (selecteds>0) {k += String("S:")+String::fromInt(selecteds) + " "; t += "Selected:" +String::fromInt(selecteds) + " ";}
  if (visibles>0) {k += String("A:")+String::fromInt(visibles) + " "; t += "Visible:" +String::fromInt(visibles) + " ";}
  ImGui::Text("%s",k.c_str()); if (!t.empty()) drawToolTip(t.c_str());
  ImGui::SameLine(); if (outlinerButton("Compact",selecteds>1)) outlinerCompact();
  maybeDisplayPlacementTooltip();
  ImGui::End();
}

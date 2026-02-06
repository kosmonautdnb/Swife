#include "GL.H"
#include "OUTLINER.HPP"
#include "IMGUIGL.HPP"
#include "CONFIG.HPP"
#include "STL/STRING.HPP"
#include "GAMEOBJ.HPP"
#include "SVG.HPP"
#include "MENUBAR.HPP"
#include "DIALOGS.HPP"

int outliner_width = 160;
char outliner_filter[NAMEMAX] = {0};

unsigned int outlinerLayers = 1;
char selectedGroupName[NAMEMAX];

unsigned int layerSelection(unsigned int a, const char *dcl) {
  char buffer[16];
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5,0));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5);
  for (int i = 0; i < 8; i++) {
    sprintf(buffer,"%c##%s",i+'0',dcl);
    if (i != 0) ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button,a & (1<<i) ? color_button_on_bg:color_button_off_bg);
    ImGui::PushStyleColor(ImGuiCol_Text,a & (1<<i) ? color_button_on_text:color_button_off_text);
    if (ImGui::Button(buffer)) a ^= 1<<i;
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
  }
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
  return a;
}

bool sectionOn;
bool sectionVisible;
bool sectionSelecting;
bool sectionSelectingType;

void objectSelect(unsigned int i) {
  const int t = gameObjects[i]->type;
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
    ImGui::Checkbox((String("##OBJECTSELECTSECTION")+String::fromInt(i)).c_str(),&gameObjects[i]->sectionOn);
    ImGui::SameLine();
    ImGui::Checkbox((String("##OBJECTSELECTSECTIONVISIBLE")+String::fromInt(i)).c_str(),&gameObjects[i]->sectionVisible);
    ImGui::SameLine();
    sectionOn = gameObjects[i]->sectionOn;
    sectionVisible = gameObjects[i]->sectionVisible;
    v = true;
  } else {
    const char *icon = iconNameForGameObjectType[t]; SVG *svg = NULL; if (svgs.has(icon)) svg = &svgs[icon];
    if (svg != NULL) {
      ImGui::Image((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h),ImVec2(0,0),ImVec2(1,1),ImVec4(1,1,1,v?1:0.5));
      ImGui::SameLine();
    }
  }
  ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(1,1,1,1));
  ImVec4 tc = selected ? color_text_on : color_text_off;
  if (!v) tc.w *= 0.675;
  ImGui::PushStyleColor(ImGuiCol_Text,tc);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(0.8,0.8,0.8,1));

  bool empty = strlen(gameObjects[i]->name) == 0;
  if (ImGui::Button((String(empty ? NONAME : gameObjects[i]->name)+String("##OBJECTSELECT")+String::fromInt(i)).c_str())) {
    GLboolean shift,ctrl,alt;
    glSpecialKeys(&shift,&ctrl,&alt);
    if (!shift) {
      for (int i = 0; i < gameObjects.size(); i++) {
        gameObjects[i]->selected = false;
      }
    }
    gameObjects[i]->selected = !gameObjects[i]->selected;
    if (shift && t == GAMEOBJECT_SECTION) {sectionSelecting = true; sectionSelectingType = gameObjects[i]->selected;}
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
  if (!gameObjects.empty() && gameObjects[0]->selected) return;
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

void drawOutliner() {
  bool one = false; for (int i2 = 0; i2 < gameObjects.size(); i2++) if (gameObjects[i2]->selected) {one=true; break;}
  ImGui::SetNextWindowPos(ImVec2(0,menuBar_height));
  ImGui::Begin("OUTLINER", NULL, ImVec2(outliner_width,main_window_yres-menuBar_height),1.0,FROZENWINDOW);
  dialogQuit(false);
  dialogAddObject(false,GAMEOBJECT_UNDEFINED);
  dialogAddProperty(false,NULL);
  dialogSelectByProperty(false);
  dialogSelectByLayer(false);
  dialogSelectInvisible(false);
  dialogDeleteGameObjects(false);
  dialogAbout(false);
  dialogControls(false);
  dialogColor(false,NULL);
  dialogObjectTextures(false);
  outlinerLayers = layerSelection(outlinerLayers,"OUTLLAY");
  ImGui::Separator();
  ImGui::Text(" "); ImGui::SameLine();SVG *svg = &svgs["search"];ImGui::Image((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h));ImGui::SameLine();ImGui::InputText("##OUTLINERFILTER",outliner_filter,NAMEMAX);
  ImGui::SameLine(); if (gameObjectDeleteButton(one)) dialogDeleteGameObjects(true);
  if (outlinerButton("Move up",one)) outlinerMoveUp(); ImGui::SameLine(); if (outlinerButton("Move down",one)) outlinerMoveDown();
  static bool selecty = false;
  if (one) selecty = true;
  bool markAllWithSelecty = false; ImGui::SameLine(); if (ImGui::Checkbox("##OUTLINERSELECTY",&selecty)) {markAllWithSelecty = true;}
  ImGui::Separator();
  ImGui::BeginChild("OUTLINERCHILD1",ImVec2(0,main_window_yres-ImGui::GetCursorScreenPos().y-18));
  String filter = toLower(String(outliner_filter));
  sectionOn = true;
  sectionVisible = true;
  sectionSelecting = false;
  sectionSelectingType = false;
  int selecteds = 0;
  int visibles = 0;
  char groupName[NAMEMAX];
  sprintf(selectedGroupName,"%s","");
  sprintf(groupName,"%s","");
  for (int i = 0; i < gameObjects.size(); i++) {
    gameObjects[i]->visible1 = false;
    if (markAllWithSelecty) gameObjects[i]->selected = false;
    if (gameObjects[i]->type == GAMEOBJECT_SECTION && sectionSelecting) {sectionSelecting = false;}
    if (sectionSelecting) gameObjects[i]->selected = sectionSelectingType;
    int l = gameObjects[i]->layers;
    if ((l==0)||(outlinerLayers==0)||(l & outlinerLayers)) {
      bool empty = strlen(gameObjects[i]->name) == 0;
      char *n = empty ? NONAME : gameObjects[i]->name;
      if (gameObjects[i]->type == GAMEOBJECT_SECTION || filter.empty() || (toLower(n).findFirst(filter)>=0)) {
        if (sectionOn || gameObjects[i]->type == GAMEOBJECT_SECTION) visibles++;
        if (markAllWithSelecty && sectionOn) gameObjects[i]->selected = selecty;
        gameObjects[i]->visible1 = sectionVisible;
        objectSelect(i);
      }
    }
    if (gameObjects[i]->selected) selecteds++;
    if (gameObjects[i]->type == GAMEOBJECT_SECTION) 
      sprintf(groupName,"%s",gameObjects[i]->name);
    else if (gameObjects[i]->selected && (strlen(selectedGroupName)==0)) sprintf(selectedGroupName,"%s",groupName);
  }
  ImGui::EndChild();
  String k;
  if (selecteds>0) k += String("S:")+String::fromInt(selecteds) + " ";
  if (visibles>0) k += String("A:")+String::fromInt(visibles) + " ";
  ImGui::Text("%s",k.c_str());
  ImGui::SameLine(); if (outlinerButton("Compact",selecteds>1)) outlinerCompact();
  ImGui::End();
}

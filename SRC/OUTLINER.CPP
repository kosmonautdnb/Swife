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

void objectSelect(unsigned int i) {
  ImVec2 cursorScreen = ImGui::GetCursorScreenPos();
  String k = String::fromInt(i);
  ImGui::SetCursorScreenPos(ImVec2(cursorScreen.x + outliner_width - k.length()*7-22,cursorScreen.y));
  ImGui::PushStyleColor(ImGuiCol_Text,gameObjects[i]->selected ? ImVec4(0.25,0.25,0.25,1) : ImVec4(0.7,0.7,0.7,1));
  ImGui::Text("%s",k.c_str());
  ImGui::PopStyleColor();
  ImGui::SetCursorScreenPos(cursorScreen);

  bool selected = gameObjects[i]->selected;
  const int t = gameObjects[i]->type;
  const char *icon = iconNameForGameObjectType[t]; SVG *svg = NULL; if (svgs.has(icon)) svg = &svgs[icon];
  if (svg != NULL) {
    ImGui::Image((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h));
    ImGui::SameLine();
  }
  ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(1,1,1,1));
  ImGui::PushStyleColor(ImGuiCol_Text,selected ? color_text_on : color_text_off);
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
  dialogDeleteGameObjects(false);
  dialogAbout(false);
  dialogControls(false);
  dialogColor(false,NULL);
  outlinerLayers = layerSelection(outlinerLayers,"OUTLLAY");
  ImGui::Separator();
  ImGui::Text(" "); ImGui::SameLine();SVG *svg = &svgs["search"];ImGui::Image((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h));ImGui::SameLine();ImGui::InputText("##OUTLINERFILTER",outliner_filter,NAMEMAX);
  ImGui::SameLine(); if (gameObjectDeleteButton(one)) dialogDeleteGameObjects(true);
  if (outlinerButton("Move up",one)) outlinerMoveUp(); ImGui::SameLine(); if (outlinerButton("Move down",one)) outlinerMoveDown();
  ImGui::Separator();
  ImGui::BeginChild("OUTLINERCHILD1");
  String filter = toLower(String(outliner_filter));
  for (int i = 0; i < gameObjects.size(); i++) {
    int l = gameObjects[i]->layers;
    if ((l==0)||(outlinerLayers==0)||(l & outlinerLayers)) {
      bool empty = strlen(gameObjects[i]->name) == 0;
      char *n = empty ? NONAME : gameObjects[i]->name;
      if (filter.empty() || (toLower(n).findFirst(filter)>=0))
        objectSelect(i);
    }
  }
  ImGui::EndChild();
  ImGui::End();
}

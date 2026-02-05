#include "IMGUIGL.HPP"
#include "PROPS.HPP"
#include "CONFIG.HPP"
#include "MENUBAR.HPP"
#include "OUTLINER.HPP"
#include "GAMEOBJ.HPP"
#include "SVG.HPP"
#include "DIALOGS.HPP"

int properties_width = 160;
char properties_filter[NAMEMAX] = {0};

void vectorInput(const char *name,Vector *v, const char *dcl) {
  ImGui::Text("%s",name);
  ImGui::SameLine();
  ImGui::PushItemWidth(40);
  char dcl2[32];
  float k;
  sprintf(dcl2,"%sx",dcl); k = v->x; ImGui::InputFloat(dcl2,&k); v->x = k;
  ImGui::SameLine();
  sprintf(dcl2,"%sy",dcl); k = v->y; ImGui::InputFloat(dcl2,&k); v->y = k;
  ImGui::SameLine();                                                                                
  sprintf(dcl2,"%sz",dcl); k = v->z; ImGui::InputFloat(dcl2,&k); v->z = k;
  ImGui::PopItemWidth();
}

bool propsButton(const char *name,int k) {
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5,0));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);

  ImVec2 cursorScreen = ImGui::GetCursorScreenPos();
  ImGui::SetCursorScreenPos(ImVec2(main_window_xres - k*7,cursorScreen.y));

  ImGui::PushStyleColor(ImGuiCol_Button,color_button_on_bg);
  ImGui::PushStyleColor(ImGuiCol_Text,color_button_on_text);
  bool ret = ImGui::Button(name);
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();

  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
  return ret;
}

bool editProperty(GameObjectProperty *p, int i) {
  char buffer[64];
  sprintf(buffer,"##PROPERTYEDIT%d",i);
  ImGui::Text(" %s",p->name);
  ImGui::Text("  "); ImGui::SameLine();
  ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.9,0.9,0.9,1));
  ImGui::PushStyleColor(ImGuiCol_CheckMark,ImVec4(0.0,0.5,0.5,1));
   switch(p->type) {
  case GAMEOBJECTPROPERTY_BOOL: {ImGui::Checkbox(buffer,&p->goBool);} break;
  case GAMEOBJECTPROPERTY_INT: {ImGui::InputInt(buffer,&p->goInt);} break;
  case GAMEOBJECTPROPERTY_FLOAT: {ImGui::InputFloat(buffer,&p->goFloat);} break;
  case GAMEOBJECTPROPERTY_FLOAT4: {
    ImGui::PushItemWidth(3*7);
    sprintf(buffer,"##PROPERTYEDIT%d0",i);ImGui::InputFloat(buffer,&p->goFloat4[0]);
    ImGui::SameLine();
    sprintf(buffer,"##PROPERTYEDIT%d1",i);ImGui::InputFloat(buffer,&p->goFloat4[1]);
    ImGui::SameLine();
    sprintf(buffer,"##PROPERTYEDIT%d2",i);ImGui::InputFloat(buffer,&p->goFloat4[2]);
    ImGui::SameLine();
    sprintf(buffer,"##PROPERTYEDIT%d3",i);ImGui::InputFloat(buffer,&p->goFloat4[3]);
    ImGui::PopItemWidth();
  } break;
  case GAMEOBJECTPROPERTY_TEXT: {ImGui::InputText(buffer,p->goText,NAMEMAX);} break;
  case GAMEOBJECTPROPERTY_COLOR: {colorItem("",buffer,&p->goColor, true);} break;
  }
  ImGui::SameLine();
  SVG *svg = &svgs["delete"];
  ImVec2 cursorScreen = ImGui::GetCursorScreenPos();
  ImGui::SetCursorScreenPos(ImVec2(main_window_xres - svg->w - 20,cursorScreen.y));
  sprintf(buffer,"##PROPERTYEDIT%d_",i);
  ImGui::PushID(buffer);
  bool ret = ImGui::ImageButton((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h));
  ImGui::PopID();
  ImGui::PopStyleColor();
   ImGui::PopStyleColor();
 return ret;
}

void showMeshProperties(GameObject *object) {
  ImGui::Text("Transform");
  Vector translation = object->translation;
  Vector scaling = object->scaling;
  Vector rotation = object->rotation;
  vectorInput("P.",&translation,"##PROPTRANSLATION");
  vectorInput("S.",&scaling,"##PROPSCALING");
  vectorInput("R.",&rotation,"##PROPROTATION");
  object->translation = translation;
  object->scaling = scaling;
  object->rotation = rotation;
  ImGui::Separator();
  ImGui::Text("Binding");
  const char *fname = object->fileName;
  if (strlen(fname)>NAMEMAX)
    fname = fname + strlen(fname)-NAMEMAX;
  if (strlen(fname)==0) fname = NONAME;
  ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.9,0.9,0.9,1));
  ImGui::Text("File"); ImGui::SameLine();ImGui::Button((String(fname)+String("##PROPFILENAME")).c_str());
  if (propsButton("Reimport##PROPSREIMPORT",strlen("Reimport")+2)) {
  }
  ImGui::PopStyleColor();
  ImGui::Separator();
}

void drawProperties() {
  int count = 0;
  int layerMask = 0;
  GameObject *first = NULL;
  for(int i = 0; i < gameObjects.size(); i++) 
    if (gameObjects[i]->selected) {
      count++;
      if (count == 1) first = gameObjects[i];
      layerMask |= gameObjects[i]->layers;
    }

  ImGui::SetNextWindowPos(ImVec2(main_window_xres-properties_width,menuBar_height));
  ImGui::Begin("PROPERTIES", NULL, ImVec2(properties_width,main_window_yres-menuBar_height),1.0,FROZENWINDOW);
  ImGui::Text("Properties");
  if (first != NULL && count == 1) {
    const char *icon = iconNameForGameObjectType[first->type]; SVG *svg = NULL; if (svgs.has(icon)) svg = &svgs[icon];
    if (svg != NULL) {ImGui::Image((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h)); ImGui::SameLine();}
    ImGui::InputText("##PROPNAME",first->name,NAMEMAX);
  }
  if (count > 1) ImGui::Text("Multiple selecteds");
  ImGui::Separator();
  if (count > 0) {
    ImGui::Text("Layers");
    unsigned int layerBefore = layerMask;
    unsigned int layerAfter = layerSelection(layerBefore, "OBJLAYERS");
    unsigned int layerDiff = layerBefore ^ layerAfter;
    for(int i = 0; i < gameObjects.size(); i++) 
      if (gameObjects[i]->selected) {
        gameObjects[i]->layers ^= layerDiff;
      }
    ImGui::Separator();
    ImGui::BeginChild("PROPERTIESPROPERTIES",ImVec2(0,0));
    if (count == 1) {
      showMeshProperties(first);
    }
    ImGui::Text("Properties");
    SVG *svg = &svgs["search"];ImGui::Image((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h));ImGui::SameLine();ImGui::InputText("##PROPERTIESFILTER",properties_filter,NAMEMAX);
    ImGui::Separator();
    String filter = toLower(String(properties_filter));
    for(int i2 = 0; i2 < gameObjects.size(); i2++) {
      if (gameObjects[i2]->selected) {
        if (count>1) ImGui::Text("%d\"%s\"",i2,gameObjects[i2]->name);
        for (int j = 0; j < gameObjects[i2]->properties.size(); j++) {
          if (!(filter.empty()||(toLower(String(gameObjects[i2]->properties[j].name)).findFirst(filter)>=0))) continue;
          if (editProperty(&gameObjects[i2]->properties[j], i2 + j * 32768)) {
            gameObjects[i2]->properties.erase(j,1); j--;
          }
        }
        if (filter.empty() && propsButton((String("Add property##")+String::fromInt(i2)).c_str(),strlen("Add property")+6)) {
          dialogAddProperty(true, gameObjects[i2]);
        }
        ImGui::Separator();
      }
    }

    ImGui::EndChild();
  }
  ImGui::End();

}

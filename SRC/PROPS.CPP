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
#include "IMGUIGL.HPP"
#include "PROPS.HPP"
#include "CONFIG.HPP"
#include "MENUBAR.HPP"
#include "OUTLINER.HPP"
#include "GAMEOBJ.HPP"
#include "SVG.HPP"
#include "DIALOGS.HPP"
#include "RENDER.HPP"
#include "PAINT.HPP"
#include "TEXTURES.HPP"
#include "SERIALZE.HPP"
#include <math.h>

int properties_width = 160;
char properties_filter[NAMEMAX] = {0};

bool vectorInput(const char *name,Vector *v, const char *dcl) {
  bool ret = false;
  ImGui::Text("%s",name);
  ImGui::SameLine();
  ImGui::PushItemWidth(40);
  char dcl2[32];
  float k;
  sprintf(dcl2,"%sx",dcl); k = v->x; if (ImGui::InputFloat(dcl2,&k)) {ret = true;} v->x = k; 
  ImGui::SameLine();
  sprintf(dcl2,"%sy",dcl); k = v->y; if (ImGui::InputFloat(dcl2,&k)) {ret = true;} v->y = k; 
  ImGui::SameLine();                                                                                
  sprintf(dcl2,"%sz",dcl); k = v->z; if (ImGui::InputFloat(dcl2,&k)) {ret = true;} v->z = k; 
  ImGui::PopItemWidth();
  return ret;
}

bool quaternionInput(const char *name,Quaternion *q, const char *dcl) {
  bool ret = false;
  ImGui::Text("%s",name);
  ImGui::SameLine();
  ImGui::PushItemWidth(40);
  char dcl2[32];
  float k;
  Vector v = q->getYawPitchRoll();
  sprintf(dcl2,"%sa",dcl); k = v.x * 180 / PI; if (ImGui::InputFloat(dcl2,&k)) {ret = true;} v.x = k * PI / 180; 
  ImGui::SameLine();
  sprintf(dcl2,"%sb",dcl); k = v.y * 180 / PI; if (ImGui::InputFloat(dcl2,&k)) {ret = true;} v.y = k * PI / 180; 
  ImGui::SameLine();                                                                                
  sprintf(dcl2,"%sc",dcl); k = v.z * 180 / PI; if (ImGui::InputFloat(dcl2,&k)) {ret = true;} v.z = k * PI / 180; 
  if (ret) {
    q->yawPitchRoll(v.x,v.y,v.z);
  }
  ImGui::PopItemWidth();
  return ret;
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
  ImGui::Text(" %s",strlen(p->name)==0?NONAME:p->name);
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

void showSkyBoxProperties(GameObject *object) {
  ImGui::Text("Transform");
  Quaternion rotation = object->rotation;
  if (quaternionInput("R.",&rotation,"##SKYBOXPROPROTATION")) {renderingUpdated = true;}
  object->rotation = rotation;
  colorItem("C.","##SKYBOXPROPCOLOR",&object->color, true);
  ImGui::Separator();
  ImGui::Text("Binding");
  FileBinding *binding = getBindingById(object->fileBindingId);
  if (binding != NULL) {
    ImGui::SameLine(); if (propsButton("Reload##SKYBOXPROPRELOAD",strlen("Reload")+4)) {
      binding->reload();
    }
    if (binding->textureIds.size()==6) {
      ImGui::Text(" Binding ID:%d",binding->uniqueId);
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,ImVec2(0,0));
      int sw = 40/aspectRatio*main_window_xres/main_window_yres;
      int sh = 40;
      ImGui::InvisibleButton("##SKYBOXPROPROTATIONT0",ImVec2(sw,sh)); ImGui::SameLine();
      textureButton("##SKYBOXPROPROTATIONT1",binding->textureIds[4],sw,sh,false);
      for (int i = 0; i < 4; i++) {
        if (i != 0) ImGui::SameLine();
        textureButton(("##SKYBOXPROPROTATIONT"+String::fromInt(i+2)).c_str(),binding->textureIds[i],sw,sh,false);
      }
      ImGui::InvisibleButton("##SKYBOXPROPROTATIONT6",ImVec2(sw,sh)); ImGui::SameLine();
      textureButton("##SKYBOXPROPROTATIONT7",binding->textureIds[5],sw,sh,false);
      ImGui::PopStyleVar();
    }
    ImGui::Text("%s",binding->fileName.last(19).c_str());
  }
  if (propsButton("Load##SKYBOXPROPLOAD",strlen("Load")+4)) {
    dialogLoadSaveSetFileNameAndPath = binding == NULL ? "" : binding->fileName; dialogLoadSaveGameObjectUniqueId = object->uniqueId; dialogLoadSave(true, DIALOG_LOADSKYBOX);
  }
  ImGui::Separator();
}

void showTransform(GameObject *object, const char *id) {
  ImGui::Text("Transform");
  Vector anchor = object->anchor;
  if (vectorInput("A.",&anchor,(String(id)+String("ANCHOR")).c_str())) {renderingUpdated = true;}
  object->anchor = anchor;
  if (propsButton(("Center anchor"+String(id)+"USEGRID").c_str(),strlen("Center anchor")+4)) {object->anchor = (object->boundingBoxMin+object->boundingBoxMax)*0.5; renderingUpdated = true;}
  ImGui::Separator();
  Vector translation = object->translation;
  Vector scaling = object->scaling;
  Quaternion rotation = object->rotation;
  if (vectorInput("P.",&translation,(String(id)+String("TRANSLATION")).c_str())) {renderingUpdated = true;}
  if (vectorInput("S.",&scaling,(String(id)+String("SCALING")).c_str())) {renderingUpdated = true;}
  if (quaternionInput("R.",&rotation,(String(id)+String("ROTATION")).c_str())) {renderingUpdated = true;}
  object->translation = translation;  
  object->scaling = scaling;  
  object->rotation = rotation;
  colorItem("C.",(String(id)+String("COLOR")).c_str(),&object->color, true);
  if (propsButton(("Quantize"+String(id)+"BAKEGRID").c_str(),strlen("Grid off")+14)) {applyGrid(&grids[object->gridId],&object->translation,&object->scaling,&object->rotation); object->useGrid = false; renderingUpdated = true;}
  ImGui::SameLine();
  if (propsButton(((object->useGrid?"Grid off":"Grid on")+String(id)+"USEGRID").c_str(),strlen("Grid off")+4)) {object->useGrid = !object->useGrid; renderingUpdated = true;}
  ImGui::Separator();
}

void showMeshProperties(GameObject *object) {
  showTransform(object,"##MESHPROP");
  ImGui::Text("Binding");
  FileBinding *binding = getBindingById(object->fileBindingId);
  if (binding != NULL) {
    ImGui::SameLine(); if (propsButton("Reload##MESHPROPRELOAD",strlen("Reload")+4)) {
      binding->reload();
    }
    if (!binding->triangles.empty()) {
      ImGui::Text(" Binding ID:%d",binding->uniqueId);
      ImGui::Text("Triangles:%d",binding->triangles.size());
      ImGui::Text("Textures:%d",binding->textureIds.size());
    }
    ImGui::Text("%s",binding->fileName.last(19).c_str());
  }
  if (propsButton("Load##MESHPROPLOAD",strlen("Load")+4)) {
    dialogLoadSaveSetFileNameAndPath = binding == NULL ? "" : binding->fileName; dialogLoadSaveGameObjectUniqueId = object->uniqueId; dialogLoadSave(true, DIALOG_LOADMESH);
  }
  ImGui::Separator();
}

unsigned int getSelectionStartId = 0;
void collectLinkIds(GameObject *object) {
  object->uniqueLinkIds.clear();
  for (int i = 0; i < gameObjects.size(); i++) {
    GameObject *o = gameObjects[i];
    if (o->selected && o->uniqueId != object->uniqueId) {
      object->uniqueLinkIds.push_back(o->uniqueId);
    }
  }
}
void showLinkProperties(GameObject *object) {
  showTransform(object,"##LINKPROP");
  ImGui::Text("Linking");
  if (object->hasDrawProblem) {
    ImGui::PushStyleColor(ImGuiCol_Text,ERRORCOLOR);
    ImGui::TextWrapped("Some objects here have multiple link references upward!");
    ImGui::PopStyleColor();
  }
  int count = 0;
  int countMissing = 0;
  for (int i = 0; i < object->uniqueLinkIds.size(); i++) {
    int j = getObjectIdByUniqueId(object->uniqueLinkIds[i]);
    if (j >= 0) {
      GameObject *o = gameObjects[j];
      ImGui::Text("%d\"%s\"",j ,o->name);
      count++;
    } else {
      ImGui::Text("<deleted>");
      countMissing++;
    }
  }
  
  if (countMissing > 0 && propsButton("Remove <deleted>s##LINKPROPDELETE1",strlen("Remove <deleted>s")+4)) {
    for (int i = 0; i < object->uniqueLinkIds.size(); i++) {
      int j = getObjectIdByUniqueId(object->uniqueLinkIds[i]);
      if (j < 0) {
        object->uniqueLinkIds.erase(i,1);
        i--;
      }
    }
  }

  if (count > 0 && propsButton("select##LINKPROPSELECT",strlen("select")+4)) {
    for (int j = 0; j < gameObjects.size(); j++) gameObjects[j]->selected = false;
    bool first = true;
    for (int i = 0; i < object->uniqueLinkIds.size(); i++) {
      int j = getObjectIdByUniqueId(object->uniqueLinkIds[i]);
      if (j >= 0) {
        GameObject *o = gameObjects[j];
        o->selected = true;
        if (first) o->outlinerShouldFocusThis = true;
        first = false;
      }
    }
  }

  if (getSelectionStartId==0) {
    if (propsButton("Get selection start##LINKPROPCOLLECT",strlen("Get selection start")+4)) getSelectionStartId = object->uniqueId;
  }

  ImGui::Separator();
}

void showTriggerProperties(GameObject *object) {
  showTransform(object,"##TRIGGERPROP");
}

void showCameraProperties(GameObject *object) {
  ImGui::Text("Camera");
  for (int i = 1; i < CAMERA_COUNT; i++) { // not user camera (0)
    ImGui::Text(" Camera %d",i); ImGui::SameLine(); if (ImGui::Checkbox(("##CAMERAPROPISCAMERA"+String::fromInt(i)).c_str(),&object->isCamera[i])) {
      if (object->isCamera[i]) {
        for (int k = 1; k < CAMERA_COUNT; k++) object->isCamera[k] = false;
        for (int j = 0; j < gameObjects.size(); j++) {
          gameObjects[j]->isCamera[i] = false;
        }
        object->isCamera[i] = true;
      }
    }
    renderingUpdated = true;
  }
  ImGui::Text("     View"); ImGui::SameLine();
  if (propsButton("Get##CAMERAPROPGET",strlen("Get")+4+5)) {object->setCamera(&cameras[currentCamera]); renderingUpdated = true;}
  ImGui::SameLine();
  if (propsButton("Put##CAMERAPROPPUT",strlen("Put")+4)) {cameras[currentCamera] = object->camera; renderingUpdated = true;}
  ImGui::Separator();
  if (vectorInput("P.",&object->camera.position,"##CAMERAPROPTRANSLATION")) {renderingUpdated = true;}
  if (quaternionInput("R.",&object->camera.rotation,"##CAMERAPROPROTATION")) {renderingUpdated = true;}
  ImGui::PushItemWidth(9*7);
  ImGui::Text("Size"); ImGui::SameLine(); float k = object->scaling.x; if (ImGui::InputFloat("##CAMERAPROPSCALE",&k)) {renderingUpdated = true;} if (k<0.001) k = 0.001; object->scaling = Vector(k,k,k); 
  ImGui::Separator();
  if (!object->camera.ortho) {ImGui::Text("Fov"); ImGui::SameLine(); if (ImGui::InputFloat("##CAMERAPROPFOV",&object->camera.fov)) {renderingUpdated = true;}
  } else {ImGui::Text("Ortho scale"); ImGui::SameLine(); if (ImGui::InputFloat("##CAMERAPROPORTHOSCALE",&object->camera.orthoScale)) {renderingUpdated = true;}}
  ImGui::Text("Aspect ratio"); ImGui::SameLine(); if (ImGui::InputFloat("##CAMERAPROPASPECTRATIO",&object->camera.aspect)) {renderingUpdated = true;}
  ImGui::Text("Near plane"); ImGui::SameLine(); if (ImGui::InputFloat("##CAMERAPROPNEAR",&object->camera.nearPlane)) {renderingUpdated = true;}
  ImGui::Text("Far plane"); ImGui::SameLine(); if (ImGui::InputFloat("##CAMERAPROPFAR",&object->camera.farPlane)) {renderingUpdated = true;}
  if (object->camera.orthoScale < 0) object->camera.orthoScale = 0;
  if (object->camera.fov < 1) object->camera.fov = 1; if (object->camera.fov>180) object->camera.fov = 180;
  if (object->camera.nearPlane < 0.00001) object->camera.nearPlane = 0.00001;
  if (object->camera.farPlane < object->camera.nearPlane+0.00001) object->camera.farPlane = object->camera.nearPlane + 0.00001;
  ImGui::PopItemWidth();

  ImGui::Separator();
}

void showBillboardProperties(GameObject *object) {
  FileBinding *binding = getBindingById(object->fileBindingId);
  bool isTexture = binding != NULL && binding->billBoardIsTexture && binding->textureIds.size()==1;

  ImGui::Text("Billboard");
  if (isTexture) ImGui::Text(" Rotate around Z"); ImGui::SameLine(); if (ImGui::Checkbox("##BILLBOARDPROPROTATEZ",&object->billBoardRotateZ)) {renderingUpdated = true;}
  Vector anchor = object->anchor;
  if (vectorInput("A.",&anchor,"##BILLBOARDPROPANCHOR")) {renderingUpdated = true;}
  object->anchor = anchor;
  Vector translation = object->translation;
  if (vectorInput("P.",&translation,"##BILLBOARDPROPTRANSLATION")) {renderingUpdated = true;}
  object->translation = translation;
  if (isTexture) {ImGui::Text("Rotation"); ImGui::SameLine(); if (ImGui::InputFloat("##BILLBOARDPROPROTATION",&object->billBoardRotation)) {renderingUpdated = true;}}
  ImGui::PushItemWidth(9*7);
  ImGui::Text("Scale"); ImGui::SameLine(); if (ImGui::InputFloat("##BILLBOARDPROPSCALE",&object->billBoardScale)) {renderingUpdated = true;}
  object->scaling.x = object->scaling.y = object->scaling.z = object->billBoardScale; // just that the anchor gets processed properly
  if (isTexture) {ImGui::Text("Aspect ratio"); ImGui::SameLine(); if (ImGui::InputFloat("##BILLBOARDPROPASPECT",&object->billBoardAspect)) {renderingUpdated = true;}}
  colorItem("C.","##BILLBOARDPROPCOLOR",&object->color, true);
  ImGui::Separator();
  if (isTexture) {
    ImGui::Text("Texture X0"); ImGui::SameLine(); if (ImGui::InputInt("##BILLBOARDPROPTX0",&object->billBoardTX0,0,0)) {renderingUpdated = true;}
    ImGui::Text("Texture Y0"); ImGui::SameLine(); if (ImGui::InputInt("##BILLBOARDPROPTY0",&object->billBoardTY0,0,0)) {renderingUpdated = true;}
    ImGui::Text("Texture X1"); ImGui::SameLine(); if (ImGui::InputInt("##BILLBOARDPROPTX1",&object->billBoardTX1,0,0)) {renderingUpdated = true;}
    ImGui::Text("Texture Y1"); ImGui::SameLine(); if (ImGui::InputInt("##BILLBOARDPROPTY1",&object->billBoardTY1,0,0)) {renderingUpdated = true;}
    if (object->billBoardTX1 < object->billBoardTX0) object->billBoardTX1 = object->billBoardTX0;
    if (object->billBoardTY1 < object->billBoardTY0) object->billBoardTY1 = object->billBoardTY0;
    if (propsButton("Recalculate aspect##BILLBOARDPROPASPECT",strlen("Recalculate aspect")+4)) {
      double sx = fabs(object->billBoardTX1-object->billBoardTX0);
      double sy = fabs(object->billBoardTY1-object->billBoardTY0);
      if (sx != 0 && sy != 0) {
        object->billBoardAspect = sx/sy;
      }
      renderingUpdated = true;
    }
  }
  ImGui::PopItemWidth();
  ImGui::Separator();

  ImGui::Text("Binding");
  if (binding != NULL) {
    ImGui::SameLine(); if (propsButton("Reload##BILLBOARDPROPRELOAD",strlen("Reload")+4)) {
      binding->reload();
    }
    if (binding->billBoardIsTexture && binding->textureIds.size()==1) {
      ImGui::Text(" Binding ID:%d",binding->uniqueId);
      bool rescaledWidth = glGetTextureWidth(binding->textureIds[0]) != binding->billBoardOriginalWidth;
      bool rescaledHeight = glGetTextureHeight(binding->textureIds[0]) != binding->billBoardOriginalHeight;
      if (rescaledWidth || rescaledHeight) ImGui::Text("Texture rescaled");
      ImGui::Text("Original width:%d",binding->billBoardOriginalWidth);
      ImGui::Text("Original height:%d",binding->billBoardOriginalHeight);
    }
    if (!binding->triangles.empty()) {
      ImGui::Text(" Binding ID:%d",binding->uniqueId);
      ImGui::Text("Triangles:%d",binding->triangles.size());
      ImGui::Text("Textures:%d",binding->textureIds.size());
      ImGui::Text("Rtt width:%d",binding->billBoardOriginalWidth);
      ImGui::Text("Rtt height:%d",binding->billBoardOriginalHeight);
    }
    ImGui::Text("%s",binding->fileName.last(19).c_str());
  }
  if (propsButton("Load##BILLBOARDPROPLOAD",strlen("Load")+4)) {
    dialogLoadSaveSetFileNameAndPath = binding == NULL ? "" : binding->fileName; dialogLoadSaveGameObjectUniqueId = object->uniqueId; dialogLoadSave(true, DIALOG_LOADBILLBOARD);
  }
  ImGui::Separator();
}

void drawProperties() {
  int count = 0;
  int count2 = 0;
  int layerMask = 0;
  GameObject *first = NULL;
  GameObject *first2 = NULL;
  for(int i = 0; i < gameObjects.size(); i++) 
    if (gameObjects[i]->selected) {
      if (gameObjects[i]->type != GAMEOBJECT_SECTION) 
        count++;
      else
        count2++;
      if (count == 1 && first == NULL) first = gameObjects[i];
      if (count2 == 1 && first2 == NULL) first2 = gameObjects[i];
      layerMask |= gameObjects[i]->layers;
    }

  ImGui::SetNextWindowPos(ImVec2(main_window_xres-properties_width,menuBar_height));
  ImGui::Begin("PROPERTIES", NULL, ImVec2(properties_width,main_window_yres-menuBar_height),1.0,FROZENWINDOW);
  if (!(strlen(selectedGroupName)!=0&&count2==0&&count==1)) ImGui::Text("Properties"); else ImGui::Text("%s",selectedGroupName);
  if (first != NULL && count == 1) {
    const char *icon = iconNameForGameObjectType[first->type]; SVG *svg = NULL; if (svgs.has(icon)) svg = &svgs[icon];
    if (svg != NULL) {ImGui::Image((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h)); ImGui::SameLine();}
    ImGui::InputText("##PROPNAME",first->name,NAMEMAX);
  }
  if (first2 != NULL && count2 == 1 && count == 0) {
    ImGui::InputText("##PROPNAME",first2->name,NAMEMAX); // section
  }
  if (count > 1) ImGui::Text("Multiple selecteds");
  ImGui::Separator();
  if (getSelectionStartId!=0) { // LINK SELECTION COLLECT
    if (propsButton("Get selection end##LINKPROPCOLLECT2",strlen("Get selection end")+4)) {
      int j = getObjectIdByUniqueId(getSelectionStartId);
      if (j >= 0) {
        collectLinkIds(gameObjects[j]);
        for (int i = 0; i < gameObjects.size(); i++) gameObjects[i]->selected = false;
        gameObjects[j]->selected = true;
      }
      getSelectionStartId = 0;
      renderingUpdated = true;
    }
    ImGui::Separator();
  }
  if (count > 0) {
    ImGui::Text("Layers");
    unsigned int layerBefore = layerMask;
    unsigned int layerAfter = layerSelection(layerBefore, "OBJLAYERS");
    unsigned int layerDiff = layerBefore ^ layerAfter;
    for(int i = 0; i < gameObjects.size(); i++) 
      if (gameObjects[i]->selected) {
        gameObjects[i]->layers ^= layerDiff;
        if (layerDiff != 0) renderingUpdated = true;
      }
    ImGui::Separator();
    ImGui::BeginChild("PROPERTIESPROPERTIES",ImVec2(0,0));
    if (count == 1) {
      switch(first->type) {
      case GAMEOBJECT_MESH: {showMeshProperties(first);} break;
      case GAMEOBJECT_SKYBOX: {showSkyBoxProperties(first);} break;
      case GAMEOBJECT_LINK: {showLinkProperties(first);} break;
      case GAMEOBJECT_TRIGGER: {showTriggerProperties(first);} break;
      case GAMEOBJECT_CAMERA: {showCameraProperties(first);} break;
      case GAMEOBJECT_BILLBOARD: {showBillboardProperties(first);} break;
      default:
        showMeshProperties(first);
      }
    }
    ImGui::Text("Properties");
    SVG *svg = &svgs["search"];ImGui::Image((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h));ImGui::SameLine();ImGui::InputText("##PROPERTIESFILTER",properties_filter,NAMEMAX);
    ImGui::Separator();
    String filter = toLower(String(properties_filter));
    for(int i2 = 0; i2 < gameObjects.size(); i2++) {
      if (gameObjects[i2]->selected && gameObjects[i2]->type != GAMEOBJECT_SECTION) {
        if (count>1) ImGui::Text("%d\"%s\"",i2,gameObjects[i2]->name);
        if (filter.empty()||(String("visible").findFirst(filter)>=0)) if (ImGui::Checkbox((String("Visible##PROPERTIESGAMEOBJECTVISIBLE")+String::fromInt(i2)).c_str(),&gameObjects[i2]->visible2)) {renderingUpdated = true;}
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

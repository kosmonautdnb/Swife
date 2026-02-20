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
#include "STL/DOS.HPP"
#include <math.h>

int properties_width = 160;
char properties_filter[NAMEMAX] = {0};

unsigned int uniqueIdOfSingleDisplayedObject = 0;

bool vectorInput(const String &name,Vector *v, const String &dcl) {
  bool ret = false;
  ImGui::Text("%s",name.c_str());
  ImGui::SameLine();
  ImGui::PushItemWidth(40);
  float k;
  String dcl2;
  dcl2 = dcl + "x"; k = v->x; if (ImGui::InputFloat(dcl2.c_str(),&k)) {ret = true;} v->x = k; 
  ImGui::SameLine();
  dcl2 = dcl + "y"; k = v->y; if (ImGui::InputFloat(dcl2.c_str(),&k)) {ret = true;} v->y = k; 
  ImGui::SameLine();                                                                                
  dcl2 = dcl + "z"; k = v->z; if (ImGui::InputFloat(dcl2.c_str(),&k)) {ret = true;} v->z = k; 
  ImGui::PopItemWidth();
  return ret;
}

bool vectorInput2(const String &name,Vector *v, const String &dcl) {
  bool ret = false;
  ImGui::Text("%s",name.c_str());
  ImGui::SameLine();
  ImGui::PushItemWidth(32);
  float k;
  String dcl2;
  dcl2 = dcl + "x"; k = v->x; if (ImGui::InputFloat(dcl2.c_str(),&k)) {ret = true;} v->x = k; 
  ImGui::SameLine();
  dcl2 = dcl + "y"; k = v->y; if (ImGui::InputFloat(dcl2.c_str(),&k)) {ret = true;} v->y = k; 
  ImGui::SameLine();                                                                                
  dcl2 = dcl + "z"; k = v->z; if (ImGui::InputFloat(dcl2.c_str(),&k)) {ret = true;} v->z = k; 
  ImGui::PopItemWidth();
  return ret;
}

bool quaternionInput(const char *name,Quaternion *q, const String &dcl) {
  bool ret = false;
  ImGui::Text("%s",name);
  ImGui::SameLine();
  ImGui::PushItemWidth(40);
  float k;
  String dcl2;
  Vector v = q->getYawPitchRoll();
  dcl2 = dcl + "a"; k = v.x * 180 / PI; if (ImGui::InputFloat(dcl2.c_str(),&k)) {ret = true;} v.x = k * PI / 180; 
  ImGui::SameLine();
  dcl2 = dcl + "b"; k = v.y * 180 / PI; if (ImGui::InputFloat(dcl2.c_str(),&k)) {ret = true;} v.y = k * PI / 180; 
  ImGui::SameLine();                                                                                
  dcl2 = dcl + "c"; k = v.z * 180 / PI; if (ImGui::InputFloat(dcl2.c_str(),&k)) {ret = true;} v.z = k * PI / 180; 
  if (ret) {
    q->yawPitchRoll(v.x,v.y,v.z);
  }
  ImGui::PopItemWidth();
  return ret;
}

bool quaternionInput2(const char *name,Quaternion *q, const String &dcl) {
  bool ret = false;
  ImGui::Text("%s",name);
  ImGui::SameLine();
  ImGui::PushItemWidth(32);
  float k;
  String dcl2;
  Vector v = q->getYawPitchRoll();
  dcl2 = dcl + "a"; k = v.x * 180 / PI; if (ImGui::InputFloat(dcl2.c_str(),&k)) {ret = true;} v.x = k * PI / 180; 
  ImGui::SameLine();
  dcl2 = dcl + "b"; k = v.y * 180 / PI; if (ImGui::InputFloat(dcl2.c_str(),&k)) {ret = true;} v.y = k * PI / 180; 
  ImGui::SameLine();                                                                                
  dcl2 = dcl + "c"; k = v.z * 180 / PI; if (ImGui::InputFloat(dcl2.c_str(),&k)) {ret = true;} v.z = k * PI / 180; 
  if (ret) {
    q->yawPitchRoll(v.x,v.y,v.z);
  }
  ImGui::PopItemWidth();
  return ret;
}

bool propsButton(const String &name,int k) {
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5,0));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);

  ImVec2 cursorScreen = ImGui::GetCursorScreenPos();
  ImGui::SetCursorScreenPos(ImVec2(main_window_xres - k*7,cursorScreen.y));

  ImGui::PushStyleColor(ImGuiCol_Button,color_button_on_bg);
  ImGui::PushStyleColor(ImGuiCol_Text,color_button_on_text);
  bool ret = ImGui::Button(name.c_str());
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();

  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
  return ret;
}

bool editProperty(GameObjectProperty *p, int i) {
  char buffer[64];
  sprintf(buffer,"##PROPERTYEDIT%d",i);
  ImGui::Text("%s",strlen(p->name)==0?NONAME:p->name);
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
    case GAMEOBJECTPROPERTY_LINKS: {
      ImGui::Text("Linking to:");
      for (int i2 = 0; i2 < p->goLinks.size(); i2++) {
        bool found = false;
        for (int j = 0; j < gameObjects.size(); j++) {
          if (gameObjects[j]->uniqueId == p->goLinks[i2]) {
            ImGui::Text(" %d\"%s\"",j,gameObjects[j]->name);
            found = true;
            break;
          }
        }
        if (!found) {
          ImGui::Text(" <not found>");
          ImGui::SameLine();
          sprintf(buffer,"##PROPERTYEDIT%d_x2%d",i,i2);
          ImGui::PushID(buffer);
          SVG *svg = &svgs["delete"];
          if (ImGui::ImageButton((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h))) {
            p->goLinks.erase(i2,1);
            i2--;
          }
          ImGui::PopID();
        }
      }
    } break;
    case GAMEOBJECTPROPERTY_SCRIPT: {
      ImGui::Text("Script:");
      bool found = false;
      for (int j = 0; j < gameObjects.size(); j++) {
        if (gameObjects[j]->uniqueId == p->goUniqueId) {
          ImGui::Text("%d\"%s\"",j,gameObjects[j]->name);
          found = true;
          break;
        }
      }
      if (!found) {
        ImGui::Text("<not found>");
      }
    } break;
    case GAMEOBJECTPROPERTY_REPEAT: {
      ImGui::Text("Repeat");
      ImGui::PushItemWidth(64);
      ImGui::SameLine();sprintf(buffer,"##PROPERTYEDITx%d",i);if (ImGui::InputInt(buffer,&p->goRepeat,1,1)) {renderingUpdated=true;}
      if (p->goRepeat<0) p->goRepeat=0;
      if (p->goRepeat>100) p->goRepeat = 100;
      sprintf(buffer,"##PROPERTYEDITm%d",i);if (vectorInput2("P",&p->goMove,buffer)) {renderingUpdated=true;}
      sprintf(buffer,"##PROPERTYEDITs%d",i);if (vectorInput2("S",&p->goScale,buffer)) {renderingUpdated=true;}
      sprintf(buffer,"##PROPERTYEDITr%d",i);if (quaternionInput2("R",&p->goRotate,buffer)) {renderingUpdated=true;}
      ImGui::PopItemWidth();
    } break;
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

void bindingReloadAndMakeUnique(GameObject *object, FileBinding *binding,const char *prefix) {
  ImGui::SameLine(); if (propsButton(IDCOMBINED3("Reload",prefix,"RELOAD"),strlen("Reload")+4)) {
    binding->reload();
  }
  int count = 0;
  for (int i = 0; i < gameObjects.size(); i++) {
    if (gameObjects[i]->fileBindingId == binding->uniqueId) count++;
  }
  if (count > 1) {
    if (propsButton(IDCOMBINED3("Make unique",prefix,"MAKEUNIQUE"),strlen("Make unique")+4)) {
      FileBinding *binding2 = binding->clone();
      bindings.push_back(binding2);
      object->fileBindingId = binding2->uniqueId;
    }
  }
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
    bindingReloadAndMakeUnique(object,binding,"##SKYBOXPROP");
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

Vector clampScale(const Vector &v) {
  Vector ret = v;
  if (ret.x < 0.000001) ret.x = 0.000001;
  if (ret.y < 0.000001) ret.y = 0.000001;
  if (ret.z < 0.000001) ret.z = 0.000001;
  return ret;
}

void showTransform(GameObject *object, const char *id) {
  ImGui::Text("Transform");
  Vector anchor = object->anchor;
  if (vectorInput("A.",&anchor,IDCOMBINED(id,"ANCHOR"))) {renderingUpdated = true;}
  object->anchor = anchor;
  if (propsButton(("Center anchor"+String(id)+"USEGRID").c_str(),strlen("Center anchor")+4)) {object->anchor = (object->boundingBoxMin+object->boundingBoxMax)*0.5; renderingUpdated = true;}
  ImGui::Separator();
  Vector translation = object->translation;
  Vector scaling = object->scaling;
  Quaternion rotation = object->rotation;
  if (vectorInput("P.",&translation,IDCOMBINED(id,"TRANSLATION"))) {renderingUpdated = true;}
  if (vectorInput("S.",&scaling,IDCOMBINED(id,"SCALING"))) {renderingUpdated = true;}
  if (quaternionInput("R.",&rotation,IDCOMBINED(id,"ROTATION"))) {renderingUpdated = true;}
  object->translation = translation;  
  object->scaling = clampScale(scaling);  
  object->rotation = rotation;
  colorItem("C.",String(id)+String("COLOR"),&object->color, true);
  if (propsButton(("Quantize"+String(id)+"BAKEGRID").c_str(),strlen("Grid off")+14)) {applyGrid(&grids[object->gridId],&object->translation,&object->scaling,&object->rotation); object->useGrid = false; renderingUpdated = true;}
  ImGui::SameLine();
  if (propsButton(((object->useGrid?"Grid off":"Use grid")+String(id)+"USEGRID").c_str(),strlen("Grid off")+4)) {object->useGrid = !object->useGrid; renderingUpdated = true;}
  ImGui::Separator();
}

void showMeshProperties(GameObject *object) {
  showTransform(object,"##MESHPROP");
  ImGui::Text("Binding");
  FileBinding *binding = getBindingById(object->fileBindingId);
  if (binding != NULL) {
    bindingReloadAndMakeUnique(object,binding,"##MESHPROP");
    if (!binding->triangles.empty()) {
      ImGui::Text(" Binding ID:%d",binding->uniqueId);
      ImGui::Text("Triangles:%d",binding->triangles.size());
      ImGui::Text("Textures:%d",binding->textureIds.size());
    }
    ImGui::Text("Two sided"); ImGui::SameLine(); if (ImGui::Checkbox("##MESHPROPTWOSIDED",&binding->forceTwoSided)) {renderingUpdated = true;}
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
  ImGui::Text("Size"); ImGui::SameLine(); float k = object->scaling.x; if (ImGui::InputFloat("##CAMERAPROPSCALE",&k)) {renderingUpdated = true;} if (k<0.001) k = 0.001; object->scaling = clampScale(Vector(k,k,k)); 
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
  object->scaling.x = object->scaling.y = object->scaling.z = object->billBoardScale; object->scaling = clampScale(object->scaling);// just that the anchor gets processed properly
  if (isTexture) {ImGui::Text("Aspect ratio"); ImGui::SameLine(); if (ImGui::InputFloat("##BILLBOARDPROPASPECT",&object->billBoardAspect)) {renderingUpdated = true;}}
  colorItem("C.","##BILLBOARDPROPCOLOR",&object->color, true);
  if (propsButton("Quantize##BILLBOARDPROPBAKEGRID",strlen("Grid off")+14)) {applyGrid(&grids[object->gridId],&object->translation,&object->scaling,&object->rotation); object->useGrid = false; renderingUpdated = true;}
  ImGui::SameLine();
  if (propsButton(IDCOMBINED(object->useGrid?"Grid off":"Grid on","##BILLBOARDPROPUSEGRID"),strlen("Grid off")+4)) {object->useGrid = !object->useGrid; renderingUpdated = true;}
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
    bindingReloadAndMakeUnique(object,binding,"##BILLBOARDPROP");
    if (binding->billBoardIsTexture && binding->textureIds.size()==1) {
      ImGui::Text(" Binding ID:%d",binding->uniqueId);
      bool rescaledWidth = glGetTextureWidth(binding->textureIds[0]) != binding->originalWidth;
      bool rescaledHeight = glGetTextureHeight(binding->textureIds[0]) != binding->originalHeight;
      if (rescaledWidth || rescaledHeight) ImGui::TextColored(ERRORCOLOR,"Texture rescaled");
      ImGui::Text("Original width:%d",binding->originalWidth);
      ImGui::Text("Original height:%d",binding->originalHeight);
    }
    if (!binding->triangles.empty()) {
      ImGui::Text(" Binding ID:%d",binding->uniqueId);
      ImGui::Text("Triangles:%d",binding->triangles.size());
      ImGui::Text("Textures:%d",binding->textureIds.size());
      ImGui::Text("Rtt width:%d",binding->originalWidth);
      ImGui::Text("Rtt height:%d",binding->originalHeight);
      ImGui::Text("Two sided"); ImGui::SameLine(); if (ImGui::Checkbox("##BILLBOARDPROPTWOSIDED",&binding->forceTwoSided)) {renderingUpdated = true;}
    }
    ImGui::Text("%s",binding->fileName.last(19).c_str());
  }
  if (propsButton("Load##BILLBOARDPROPLOAD",strlen("Load")+4)) {
    dialogLoadSaveSetFileNameAndPath = binding == NULL ? "" : binding->fileName; dialogLoadSaveGameObjectUniqueId = object->uniqueId; dialogLoadSave(true, DIALOG_LOADBILLBOARD);
  }
  ImGui::Separator();
}

void showPointProperties(GameObject *object) {
  ImGui::Text("Point");
  ImGui::Separator();
  ImGui::Text("Text");ImGui::SameLine();if (ImGui::InputText("##POINTPROPCAPTION",object->caption,NAMEMAX)) {renderingUpdated = true;}
  ImGui::Separator();
  Vector translation = object->translation;
  if (vectorInput("P.",&translation,"##POINTROPTRANSLATION")) {renderingUpdated = true;}
  object->translation = translation;
  ImGui::PushItemWidth(9*7);
  float k=0;
  ImGui::Text("Size"); ImGui::SameLine(); k = object->scaling.x; if (ImGui::InputFloat("##POINTPROPSCALE",&k)) {renderingUpdated = true;}
  object->scaling.x = object->scaling.y = object->scaling.z = k;
  object->scaling = clampScale(object->scaling);
  ImGui::PopItemWidth();
  colorItem("C.","##POINTROPCOLOR",&object->color, true);
  ImGui::Separator();
}

void showTextureGridProperties(GameObject *object) {
  FileBinding *binding = getBindingById(object->fileBindingId);
  showTransform(object,"##TEXTUREGRIDPROP");
  ImGui::Text("Binding");
  if (binding != NULL) {
    bindingReloadAndMakeUnique(object,binding,"##TEXTUREGRIDPROP");
    if (binding->textureIds.size()==1) {
      ImGui::Text(" Binding ID:%d",binding->uniqueId);
      bool rescaledWidth = glGetTextureWidth(binding->textureIds[0]) != binding->originalWidth;
      bool rescaledHeight = glGetTextureHeight(binding->textureIds[0]) != binding->originalHeight;
      if (rescaledWidth || rescaledHeight) ImGui::TextColored(ERRORCOLOR,"Texture rescaled");
      ImGui::Text("Original width:%d",binding->originalWidth);
      ImGui::Text("Original height:%d",binding->originalHeight);
      ImGui::Text("");
      ImGui::Text("Map");
      ImGui::Text(" Count X:%d Y:%d",binding->textureGridWidth,binding->textureGridHeight);
      if (binding->textureGridWidth<=0||binding->textureGridHeight<=0) ImGui::TextColored(ERRORCOLOR,"  Resize map!");
      static int resizeX = 0; static int resizeY = 0;
      static int offsetX = 0; static int offsetY = 0;
      ImGui::Text("  Resize to X"); ImGui::SameLine(); if (ImGui::InputInt("##TEXTUREGRIDPROPRELOADRESIZEX",&resizeX,0,0));
      ImGui::Text("  Resize to Y"); ImGui::SameLine(); if (ImGui::InputInt("##TEXTUREGRIDPROPRELOADRESIZEY",&resizeY,0,0));
      ImGui::Text("  With offset X"); ImGui::SameLine(); if (ImGui::InputInt("##TEXTUREGRIDPROPRELOADOFFSETX",&offsetX,0,0));
      ImGui::Text("  With offset Y"); ImGui::SameLine(); if (ImGui::InputInt("##TEXTUREGRIDPROPRELOADOFFSETY",&offsetY,0,0));
      if (resizeX < 0) resizeX = 0; if (resizeX > 1024) resizeX = 1024;
      if (resizeY < 0) resizeY = 0; if (resizeY > 1024) resizeY = 1024;
      if (propsButton("Resize map##TEXTUREGRIDPROPRESIZEMAP",strlen("Resize map")+4)) {binding->resizeTextureGrid(offsetX,offsetY,resizeX,resizeY); renderingUpdated = true;}
      ImGui::Text("");
      ImGui::Text("Texture atlas");
      if (binding->textureAtlasTileWidth<=0||binding->textureAtlasTileHeight<=0||binding->textureAtlasTileCountX<=0||binding->textureAtlasTileCountY<=0) ImGui::TextColored(ERRORCOLOR," Enter values!");
      ImGui::PushItemWidth(40);
      ImGui::Text(" Tile width"); ImGui::SameLine(); if (ImGui::InputInt("##TEXTUREGRIDPROPRELOADTILEWIDTH",&binding->textureAtlasTileWidth,0,0)) {renderingUpdated = true;}
      ImGui::Text(" Tile height"); ImGui::SameLine(); if (ImGui::InputInt("##TEXTUREGRIDPROPRELOADTILEHEIGHT",&binding->textureAtlasTileHeight,0,0)) {renderingUpdated = true;}
      ImGui::Text(" Tiles in X"); ImGui::SameLine(); if (ImGui::InputInt("##TEXTUREGRIDPROPRELOADTILESX",&binding->textureAtlasTileCountX,0,0)) {renderingUpdated = true;}
      ImGui::Text(" Tiles in Y"); ImGui::SameLine(); if (ImGui::InputInt("##TEXTUREGRIDPROPRELOADTILESY",&binding->textureAtlasTileCountY,0,0)) {renderingUpdated = true;}
      if (binding->textureAtlasTileWidth < 0) binding->textureAtlasTileWidth = 0;
      if (binding->textureAtlasTileHeight < 0) binding->textureAtlasTileHeight = 0;
      if (binding->textureAtlasTileCountX < 0) binding->textureAtlasTileCountX = 0;
      if (binding->textureAtlasTileCountY < 0) binding->textureAtlasTileCountY = 0;
      if (binding->textureAtlasTileCountX * binding->textureAtlasTileWidth > binding->originalWidth) {
        if (binding->textureAtlasTileWidth>0) binding->textureAtlasTileCountX = binding->originalWidth / binding->textureAtlasTileWidth;
      }
      if (binding->textureAtlasTileCountY * binding->textureAtlasTileHeight > binding->originalHeight) {
        if (binding->textureAtlasTileHeight>0) binding->textureAtlasTileCountY = binding->originalHeight / binding->textureAtlasTileHeight;
      }
      ImGui::PopItemWidth();
    }
    ImGui::Text("%s",binding->fileName.last(19).c_str());
  }
  if (propsButton("Load##TEXTUREGRIDPROPLOAD",strlen("Load")+4)) {
    dialogLoadSaveSetFileNameAndPath = binding == NULL ? "" : binding->fileName; dialogLoadSaveGameObjectUniqueId = object->uniqueId; dialogLoadSave(true, DIALOG_LOADTEXTUREGRID);
  }
  ImGui::Separator();
  if (binding != NULL) {
    if (propsButton("export##TEXTUREGRIDPROPEXPORTCSV",strlen("export")+4)) {dialogLoadSaveSetFileNameAndPath = ""; dialogLoadSaveGameObjectUniqueId = object->uniqueId; dialogLoadSave(true, DIALOG_SAVETEXTUREGRIDCSV); renderingUpdated = true;}
    ImGui::SameLine();
  }
  if (propsButton("import##TEXTUREGRIDPROPIMPORTCSV",strlen("import")+2+strlen("export")+4)) {dialogLoadSaveSetFileNameAndPath = ""; dialogLoadSaveGameObjectUniqueId = object->uniqueId; dialogLoadSave(true, DIALOG_LOADTEXTUREGRIDCSV); renderingUpdated = true;}
  ImGui::Separator();

  if (binding != NULL && binding->textureIds.size()==1) {
    if (binding->index2>=0&&binding->index2<binding->floatContent.size()) {
      ImGui::Text("Heightmap"); 
      ImGui::PushItemWidth(80);
      ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.5,0.5,0.5,1));
      ImGui::Text(" Height");ImGui::SameLine(); if (ImGui::InputFloat("##TEXTUREGRIDPROPEDITHEIGHTA",&binding->floatContent[binding->index2],1,1)) {renderingUpdated = true; binding->recalcTextureGridBoundingBox();}
      ImGui::PopStyleColor();
      ImGui::PopItemWidth();
      ImGui::Separator();
    }
    if (binding->selected<0)
      ImGui::Text("None selected");
    else
      ImGui::Text("Selected Tile %d",binding->selected);
    ImGui::Text("Show structure"); ImGui::SameLine(); if (ImGui::Checkbox("##TEXTUREGRIDPROPLOADSHOWSTRUCTURE",&object->showStructure)) {renderingUpdated = true;}
    static bool x2 = false; ImGui::Text("X2"); ImGui::SameLine(); ImGui::Checkbox("##TEXTUREGRIDPROPLOADX2",&x2);
    ImGui::SameLine();
    float tx0=0,ty0=0,tx1=0,ty1=0;
    if (binding->selected>=0&&binding->textureAtlasTileCountX>0&&binding->originalWidth>0&&binding->originalHeight>0) {
      int tx = binding->selected % binding->textureAtlasTileCountX;
      int ty = binding->selected / binding->textureAtlasTileCountX;
      tx0 = ((float)tx * binding->textureAtlasTileWidth) / binding->originalWidth;
      ty0 = ((float)ty * binding->textureAtlasTileHeight) / binding->originalHeight;
      tx1 = ((float)(tx+1) * binding->textureAtlasTileWidth) / binding->originalWidth;
      ty1 = ((float)(ty+1) * binding->textureAtlasTileHeight) / binding->originalHeight;
      ImGui::PushID("##TEXTUREGRIDPROPBIGTILE");
      float w = 64;
      ImGui::ImageButton((ImTextureID)binding->textureIds[0],ImVec2(w,w*aspectRatio*main_window_yres/main_window_xres),ImVec2(tx0,ty0),ImVec2(tx1,ty1),-1,ImVec4(0,0,0,1));
      ImGui::PopID();
    }
    ImGui::Separator();
    int tileCount = binding->textureAtlasTileCountX*binding->textureAtlasTileCountY;
    if (tileCount > 4096) tileCount = 4096;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,ImVec2(1,1));
    static double t = 0; t += timeDelta;
    int modulo = x2 ? 4 : 8;
    int w = x2 ? 32 : 16;
    static int lastFocused = -1;
    int focusOnElement = lastFocused != binding->selected ? binding->selected : -1;
    for (int i = 0; i < tileCount; i++)  {
      if ((i % modulo)!=0) ImGui::SameLine();
      float r,g,b,a;
      r = 1;
      g = 1;
      b = 1;
      a = 1;
      if (binding->selected == i) {
        g *= sin(t*10)*0.5+0.5;
        b *= sin(t*10)*0.5+0.5;
      }
      if (binding->textureAtlasTileCountX>0&&binding->originalWidth>0&&binding->originalHeight>0) {
        int tx = i % binding->textureAtlasTileCountX;
        int ty = i / binding->textureAtlasTileCountX;
        tx0 = ((float)tx * binding->textureAtlasTileWidth) / binding->originalWidth;
        ty0 = ((float)ty * binding->textureAtlasTileHeight) / binding->originalHeight;
        tx1 = ((float)(tx+1) * binding->textureAtlasTileWidth) / binding->originalWidth;
        ty1 = ((float)(ty+1) * binding->textureAtlasTileHeight) / binding->originalHeight;
      }
      if (focusOnElement==i) ImGui::SetScrollHere();
      if (textureAtlasButton(IDINDEXED("##TEXTUREGRIDPROPBTN",i),binding->textureIds[0],w,w*aspectRatio*main_window_yres/main_window_xres,tx0,ty0,tx1,ty1,r,g,b,a)) {
        binding->selected = i;
      }
    }
    lastFocused = binding->selected;
    ImGui::PopStyleVar();
    ImGui::Separator();
  }
}

void showScriptProperties(GameObject *object) {
  ImGui::Text("Script");
  ImGui::SameLine();
  if (propsButton("Load##SCRIPTPROPLOADSCRIPT",strlen("Load")+4)) {dialogLoadSaveSetFileNameAndPath = object->scriptFileName.empty() ? "" : object->scriptFileName; dialogLoadSaveGameObjectUniqueId = object->uniqueId; dialogLoadSave(true, DIALOG_LOADSCRIPT); renderingUpdated = true;}
  if (!object->script.empty()) {
    ImGui::SameLine();
    if (propsButton("Save##SCRIPTPROPSAVESCRIPT",strlen("Save")+2+strlen("Load")+4)) {dialogLoadSaveSetFileNameAndPath = object->scriptFileName.empty() ? "" : object->scriptFileName; dialogLoadSaveGameObjectUniqueId = object->uniqueId; dialogLoadSave(true, DIALOG_SAVESCRIPT); renderingUpdated = true;}
  }
  if (propsButton("Edit##SCRIPTPROPEDIFSCRIPT",strlen("Edit")+4)) {dialogEditScript(true, object); renderingUpdated = true;}
  ImGui::Separator();
  if (!object->scriptFileName.empty()) {
    ImGui::Text("Filename:");
    ImGui::Text("%s",object->scriptFileName.last(19).c_str());
    if (propsButton("Reload##SCRIPTPROPRELOADSCRIPT",strlen("Reload")+4)) {dialogLoadSaveGameObjectUniqueId = object->uniqueId; loadScript(getPath(object->scriptFileName).c_str(),(getName(object->scriptFileName)+getExtension(object->scriptFileName)).c_str()); renderingUpdated = true;}
    ImGui::Separator();
  }
  ImGui::Text("Script text:");
  ImGui::Separator();
  ImGui::BeginChild("##SCRIPTPROPCHILD",ImVec2(0,main_window_yres/2));
  ImGui::TextWrapped("%s",object->script.c_str());
  ImGui::EndChild();
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
    ImGui::SameLine();
    bool selected = uniqueIdOfSingleDisplayedObject != 0; if (ImGui::Checkbox("##PROPSINGLE",&selected)) {uniqueIdOfSingleDisplayedObject = selected ? first->uniqueId : 0 ;renderingUpdated = true;}
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

    bool selected = uniqueIdOfSingleDisplayedObject == first->uniqueId;
    if (!selected) {
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
    } else {
      ImGui::Text("Single object view");
      ImGui::Separator();
    }
    ImGui::BeginChild("PROPERTIESPROPERTIES",ImVec2(0,0));
    if (count == 1) {
      switch(first->type) {
      case GAMEOBJECT_MESH: {showMeshProperties(first);} break;
      case GAMEOBJECT_SKYBOX: {showSkyBoxProperties(first);} break;
      case GAMEOBJECT_LINK: {showLinkProperties(first);} break;
      case GAMEOBJECT_TRIGGER: {showTriggerProperties(first);} break;
      case GAMEOBJECT_CAMERA: {showCameraProperties(first);} break;
      case GAMEOBJECT_BILLBOARD: {showBillboardProperties(first);} break;
      case GAMEOBJECT_POINT: {showPointProperties(first);} break;
      case GAMEOBJECT_TEXTUREGRID: {showTextureGridProperties(first);} break;
      case GAMEOBJECT_SCRIPT: {showScriptProperties(first);} break;
      default:
        showMeshProperties(first);
      }
    }
    ImGui::Text("Properties");
    SVG *svg = &svgs["search"];ImGui::Image((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h));ImGui::SameLine();ImGui::InputText("##PROPERTIESFILTER",properties_filter,NAMEMAX);
    ImGui::Separator();
    ImGui::Separator();
    String filter = toLower(String(properties_filter));
    String visib_ = "visible";
    for(int i2 = 0; i2 < gameObjects.size(); i2++) {
      if (gameObjects[i2]->selected && gameObjects[i2]->type != GAMEOBJECT_SECTION) {
        if (count>1) ImGui::Text(" %d\"%s\"",i2,gameObjects[i2]->name);
        if (filter.empty()||(visib_.findFirst(filter)>=0)) {if (ImGui::Checkbox(IDINDEXED("Visible##PROPERTIESGAMEOBJECTVISIBLE",i2),&gameObjects[i2]->visible2)) {renderingUpdated = true;}ImGui::Separator();}
        for (int j = 0; j < gameObjects[i2]->properties.size(); j++) {
          if (!(filter.empty()||(toLower(String(gameObjects[i2]->properties[j].name)).findFirst(filter)>=0))) continue;
          if (editProperty(&gameObjects[i2]->properties[j], i2 + j * 32768)) {
            gameObjects[i2]->properties.erase(j,1); j--;
          }
          ImGui::Separator();
        }
        if (filter.empty() && propsButton(IDINDEXED("Add property##",i2),strlen("Add property")+6)) {
          dialogAddProperty(true, gameObjects[i2]);
        }
        ImGui::Separator();
        ImGui::Separator();
      }
    }

    ImGui::EndChild();
  }
  ImGui::End();

}

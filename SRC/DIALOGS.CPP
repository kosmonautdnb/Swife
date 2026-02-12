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
#include "CONFIG.HPP"
#include "DIALOGS.HPP"
#include "GAMEOBJ.HPP"
#include "OUTLINER.HPP"
#include "RENDER.HPP"
#include "SVG.HPP"
#include "TEXTURES.HPP"
#include "STL/DOS.HPP"
#include "UTIL/LOG.HPP"
#include "SERIALZE.HPP"
#include "PROPS.HPP"
#include <math.h>

#define DIALOGSTART(__name__) static int keyboardFocusNew = 0; static bool openItNextTime = false;  if (openIt) {openItNextTime = true; keyboardFocusNew = 2; return;}  if (openItNextTime) {openItNextTime = false; ImGui::OpenPopup(__name__); } ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0,0,0,1)); ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1,1,1,1); if (ImGui::BeginPopupModal(__name__,NULL,ImGuiWindowFlags_AlwaysAutoResize)) { ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(0,0,0,1); ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.4,0.4,0.4,1));disableRenderMouseClicks();
#define DIALOGEND() ImGui::PopStyleColor(); ImGui::EndPopup();} ImGui::PopStyleColor();
#define DIALOGKEYSHERE() {if (keyboardFocusNew>0) {keyboardFocusNew--;ImGui::SetKeyboardFocusHere();}}

bool quit = false;

unsigned int colorWheelX = 160;
unsigned int colorWheelY = 160*aspectRatio*main_window_yres/main_window_xres;
unsigned int colorWheelTexture = 0;

void initDialogs() {
  glGenTextures(1,&colorWheelTexture);
  glBindTexture(GL_TEXTURE_2D,colorWheelTexture);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, colorWheelX, colorWheelY, 0, GL_RGBA, GL_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void dialogDeleteGameObjects(bool openIt) {
  DIALOGSTART("Delete objects?")
  GameObject *first = NULL;
  int count = 0;
  int f = 0;
  for(int i = 0; i < gameObjects.size(); i++) 
    if (gameObjects[i]->selected) {
      count++;
      if (count == 1) {first = gameObjects[i]; f = i;}
    }
  if (count > 1) {
    ImGui::Text("Do you want to delete %d objects?\n\n",count);
    ImGui::BeginChild("DIALOGDELETEOBJECTSCHILD",ImVec2(main_window_xres/3,main_window_yres/4));
    for(int i = 0; i < gameObjects.size(); i++) {
      if (gameObjects[i]->selected) {
        ImGui::Text("%d\"%s\"",i,gameObjects[i]->name);
      }
    }
    ImGui::EndChild();
  }
  else if (first != NULL) ImGui::Text("Do you want to delete %d\"%s\"?\n\n",f,first->name);
  ImGui::Separator();
  if (ImGui::Button("Delete##DIALOGDELETEOBJECTSYES")) {
    for(int i = 0; i < gameObjects.size(); i++) 
      if (gameObjects[i]->selected) {
        deleteGameObject(i);
        i--;
      }
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button("Close##DIALOGDELETEOBJECTSCLOSE")) {
    ImGui::CloseCurrentPopup();
  }
  DIALOGEND()
}

void dialogQuit(bool openIt) {
  DIALOGSTART("Quit?")
  ImGui::Text("Do you really want to quit (did you save)?\n\n");
  ImGui::Separator();
  if (ImGui::Button("Quit##DIALOGQUITYES")) {
    quit = true;
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button("No##DIALOGQUITNO")) {
    ImGui::CloseCurrentPopup();
  }
  DIALOGEND()
}

void dialogAddObject(bool openIt, unsigned int type) {
  static char name[NAMEMAX];
  static char title[NAMEMAX];
  static unsigned int t = GAMEOBJECT_UNDEFINED; if (type != GAMEOBJECT_UNDEFINED) {
    t = type; memset(name,0,sizeof(name));
    sprintf(title, "Add %s", gameObjectTypeNames[type]);
  } 
  DIALOGSTART(title)
  ImGui::PushItemWidth(7*NAMEMAX);
  ImGui::Text("Name"); ImGui::SameLine(); DIALOGKEYSHERE(); ImGui::InputText("##DIALOGADDGAMEOBJECTNAME", name, NAMEMAX);
  if (t == GAMEOBJECT_MESH) {
    if (ImGui::Button("Load##DIALOGADDGAMEOBJECTLOAD")) {
      GameObject *o = new GameObject();
      o->setName(name);
      o->setType(t);
      o->selected = true;
      o->layers = outlinerLayers;
      for (int i = 0; i < gameObjects.size(); i++) gameObjects[i]->selected = false;
      gameObjects.insert(o,0);
      dialogLoadSaveSetFileNameAndPath = ""; dialogLoadSaveGameObjectUniqueId = o->uniqueId; dialogLoadSave(true, DIALOG_LOADMESH);
      renderingUpdated = true;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
  }
  if (ImGui::Button("Add##DIALOGADDGAMEOBJECTADD")) {
    GameObject *o = new GameObject();
    o->setName(name);
    o->setType(t);
    o->selected = true;
    o->layers = outlinerLayers;
    for (int i = 0; i < gameObjects.size(); i++) gameObjects[i]->selected = false;
    gameObjects.insert(o,0);
    renderingUpdated = true;
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button("Close##DIALOGADDGAMEOBJECTCLOSE")) {
    ImGui::CloseCurrentPopup();
  }
  DIALOGEND()
}

void dialogAddProperty(bool openIt, class GameObject *object) {
  static GameObjectProperty p;
  static GameObject *o = NULL; if (object != NULL) {o = object;p = GameObjectProperty(); p.setName("");}
  DIALOGSTART("Add property")
  ImGui::PushItemWidth(7*NAMEMAX);
  ImGui::Text("Name"); ImGui::SameLine();  DIALOGKEYSHERE(); ImGui::InputText("##DIALOGADDPROPERTYNAME", p.name, NAMEMAX);
  ImGui::SameLine();
  if (ImGui::Combo("Type##DIALOGADDPROPERTYTYPE",(int*)&p.type, propertyTypeNames, GAMEOBJECTPROPERTY_COUNT)) p.clearValue();
  ImGui::PopItemWidth();
  ImGui::Text("Value"); ImGui::SameLine();
  ImGui::PushItemWidth(7*NAMEMAX);
  switch(p.type) {
  case GAMEOBJECTPROPERTY_UNDEFINED: {ImGui::Text("<UNDEFINED>");} break;
  case GAMEOBJECTPROPERTY_BOOL: {ImGui::Checkbox("##DIALOGADDPROPERTYCHECKBOX", &p.goBool);} break;
  case GAMEOBJECTPROPERTY_INT: {ImGui::InputInt("##DIALOGADDPROPERTYINT", &p.goInt,1);} break;
  case GAMEOBJECTPROPERTY_FLOAT: {ImGui::InputFloat("##DIALOGADDPROPERTYFLOAT", &p.goFloat);} break;
  case GAMEOBJECTPROPERTY_FLOAT4: {
    ImGui::PushItemWidth(10*7);
    ImGui::InputFloat("##DIALOGADDPROPERTYFLOAT40", &p.goFloat4[0]);
    ImGui::SameLine();
    ImGui::InputFloat("##DIALOGADDPROPERTYFLOAT41", &p.goFloat4[1]);
    ImGui::SameLine();
    ImGui::InputFloat("##DIALOGADDPROPERTYFLOAT42", &p.goFloat4[2]);
    ImGui::SameLine();
    ImGui::InputFloat("##DIALOGADDPROPERTYFLOAT43", &p.goFloat4[3]);
    ImGui::PopItemWidth();
  } break;
  case GAMEOBJECTPROPERTY_TEXT: {ImGui::InputText("##DIALOGADDPROPERTYTEXT", p.goText, NAMEMAX);} break;
  case GAMEOBJECTPROPERTY_COLOR: {colorItem("","##DIALOGADDPROPERTYCOLOR", &p.goColor, true);} break;
  }
  ImGui::PopItemWidth();
  if (ImGui::Button("Add##DIALOGADDPROPERTYADD")) {
    if (o != NULL) {o->addProperty(&p); o = NULL;}
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button("Close##DIALOGADDPROPERTYCLOSE")) {
    ImGui::CloseCurrentPopup();
  }
  DIALOGEND()
}

void dialogSelectByProperty(bool openIt) {
  static GameObjectProperty p;
  if (openIt) {
    p = GameObjectProperty();
    p.setName("");
  }
  DIALOGSTART("Select by property")
  ImGui::PushItemWidth(7*NAMEMAX);
  ImGui::Text("Name"); ImGui::SameLine(); ImGui::InputText("##DIALOGSELECTBYPROPERTYNAME", p.name, NAMEMAX);
  ImGui::SameLine();
  if (ImGui::Combo("Type##DIALOGDIALOGSELECTBYPROPERTYTYPE",(int*)&p.type, propertyTypeNames, GAMEOBJECTPROPERTY_COUNT)) p.clearValue();
  ImGui::PopItemWidth();
  ImGui::Text("Value"); ImGui::SameLine();
  ImGui::PushItemWidth(7*NAMEMAX);
  switch(p.type) {
  case GAMEOBJECTPROPERTY_UNDEFINED: {ImGui::Text("<UNDEFINED>");} break;
  case GAMEOBJECTPROPERTY_BOOL: {ImGui::Checkbox("##DIALOGSELECTBYPROPERTYCHECKBOX", &p.goBool);} break;
  case GAMEOBJECTPROPERTY_INT: {ImGui::InputInt("##DIALOGSELECTBYPROPERTYINT", &p.goInt,1);} break;
  case GAMEOBJECTPROPERTY_FLOAT: {ImGui::InputFloat("##DIALOGSELECTBYPROPERTYFLOAT", &p.goFloat);} break;
  case GAMEOBJECTPROPERTY_FLOAT4: {
    ImGui::PushItemWidth(10*7);
    ImGui::InputFloat("##DIALOGSELECTBYPROPERTYFLOAT40", &p.goFloat4[0]);
    ImGui::SameLine();
    ImGui::InputFloat("##DIALOGSELECTBYPROPERTYFLOAT41", &p.goFloat4[1]);
    ImGui::SameLine();
    ImGui::InputFloat("##DIALOGSELECTBYPROPERTYFLOAT42", &p.goFloat4[2]);
    ImGui::SameLine();
    ImGui::InputFloat("##DIALOGSELECTBYPROPERTYFLOAT43", &p.goFloat4[3]);
    ImGui::PopItemWidth();
  } break;
  case GAMEOBJECTPROPERTY_TEXT: {ImGui::InputText("##DIALOGSELECTBYPROPERTYTEXT", p.goText, NAMEMAX);} break;
  case GAMEOBJECTPROPERTY_COLOR: {colorItem("","##DIALOGSELECTBYPROPERTYCOLOR", &p.goColor, true);} break;
  }
  ImGui::PopItemWidth();
  if (ImGui::Button("Select##DIALOGSELECTBYPROPERTYSELECT")) {
    String filter = toLower(p.name);
    for (int i = 0; i < gameObjects.size(); i++) {
      GameObject *o = gameObjects[i];
      o->selected = false;
      for (int j = 0; j < o->properties.size(); j++) {
        GameObjectProperty *p2 = &o->properties[j];
        String pn = toLower(p2->name);
        if (filter.empty() || (pn.findFirst(filter)>=0)) {
          bool found = false;
          switch(p.type) {
          case GAMEOBJECTPROPERTY_UNDEFINED: {found=true;} break;
          case GAMEOBJECTPROPERTY_BOOL: {found=p.goBool == p2->goBool;} break;
          case GAMEOBJECTPROPERTY_INT: {found=p.goInt == p2->goInt;} break;
          case GAMEOBJECTPROPERTY_FLOAT: {found=p.goFloat == p2->goFloat;} break;
          case GAMEOBJECTPROPERTY_FLOAT4: {found=memcmp(p.goFloat4,p2->goFloat4,sizeof(p.goFloat4))==0;} break;
          case GAMEOBJECTPROPERTY_TEXT: {found=strcmp(p.goText,p2->goText)==0;} break;
          }
          if (found) o->selected = true;
        }
      }
    }
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button("Close##DIALOGSELECTBYPROPERTYCLOSE")) {
    ImGui::CloseCurrentPopup();
  }
  DIALOGEND()
}

void dialogSelectByLayer(bool openIt) {
  static unsigned int layers = 1;
  DIALOGSTART("Select by layer")
  layers = layerSelection(layers, "DSLLAYERS"); // needs short id text
  if (ImGui::Button("Select##DIALOGSELECTBYPROPERTYSELECT")) {
    for (int i = 0; i < gameObjects.size(); i++) {
      GameObject *o = gameObjects[i];
      o->selected = (o->layers & layers)!=0?true:false;
    }
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button("Close##DIALOGSELECTBYLAYERLAYERSCLOSE")) {
    ImGui::CloseCurrentPopup();
  }
  DIALOGEND()
}

void dialogSelectInvisible(bool openIt) {
  DIALOGSTART("Select all invisible")
  int count = 0;
  for (int i = 0; i < gameObjects.size(); i++) {
    if (!gameObjects[i]->visible2) count++;
  }
  if (count == 0)
    ImGui::Text("Currently all objects visible.");
  else
    ImGui::Text("Currently %d objects invisible.",count);
  if (count != 0 && ImGui::Button("Select##DIALOGSELECTINVISIBLESELECT")) {
    for (int i = 0; i < gameObjects.size(); i++) {
      GameObject *o = gameObjects[i];
      o->selected = !o->visible2;
    }
    ImGui::CloseCurrentPopup();
  }
  if (count != 0) ImGui::SameLine();
  if (ImGui::Button("Close##DIALOGSELECTINVISIBLECLOSE")) {
    ImGui::CloseCurrentPopup();
  }
  DIALOGEND()
}

void dialogAbout(bool openIt) {
  DIALOGSTART("About")
  ImGui::Text(
    "SWIFE - A SWIFT 3D EDITOR FOR DOS\n"
    "------------------------------------\n"
    "Coded in early 2026 by\n"
    "Stefan Mader of Puls4r\n"
    "------------------------------------\n"
  );
  if (ImGui::Button("Close##DIALOGABOUTCLOSE")) {
    ImGui::CloseCurrentPopup();
  }
  DIALOGEND()
}

void dialogControls(bool openIt) {
  DIALOGSTART("Controls")
  ImGui::Text(
    "Outliner:\n"
    "Left mouse click for select.\n"
    "Use shift for multiselect.\n"
    "Use shift on sections for group select/deselect.\n"
    "Keyboard:\n"
    "You can alter the move speed in the toolbar.\n"
    "Use W to move forward.\n"
    "Use S to move backward.\n"
    "Use D to move right.\n"
    "Use A to move left.\n"
    "Use E to move up.\n"
    "Use Q to move down.\n"
    "Use O and P to rotate view 90 degrees.\n"
    "Mouse:\n"
    "Right mouse button and mouse to rotate view.\n"
    "Right mouse button and shift and mouse to roll.\n"
    "Right mouse button and ctrl and mouse to pan.\n"
    "Tool \"selection\":\n"
    "Left Mouse select this object\n"
    "Left Mouse and shift add to selection\n"
    "Left Mouse and ctrl remove from selection\n"
  );
  if (ImGui::Button("Close##DIALOGCONTROLSCLOSE")) {
    ImGui::CloseCurrentPopup();
  }
  DIALOGEND()
}

float hueToRgb(float p, float q, float t) {
  if (t < 0) t += 1;
  if (t > 1) t -= 1;
  if (t < 1/6.0) return p + (q-p)*6*t;
  if (t < 1/2.0) return q;
  if (t < 2.0/3.0) return p + (q-p) * (2/3.0-t)*6;
  return p;
}

Vector hsl(double h,double s,double l) {
  if (h<0.00001) h = 0.00001; // hotfix
  double r,g,b;
  if (s == 0) {
    r=g=b=l;
  } else {
    const double q = l < 0.5 ? l * (1+s) : l + s - l * s;
    const double p = 2 * l - q;
    r = hueToRgb(p, q, h + 1 / 3.0);
    g = hueToRgb(p, q, h);
    b = hueToRgb(p, q, h - 1 / 3.0);
  }
  return Vector(r,g,b);
}

void getHsl(const Vector &rgb, float *hv, float *sv, float *lv) {
  double r = rgb.x;
  double g = rgb.y;
  double b = rgb.z;
  double m = r; if (g > m) m = g; if (b > m) m = b;
  double n = r; if (g < n) n = g; if (b < n) n = b;
  double h = 0, s = 0, l = (m+n)*0.5;
  if (m == n) {
    *hv = 0;
    *sv = 0;
    *lv = l;
    return;
  }
  double d = m - n;
  s = l > 0.5 ? d / (2-m-n) : d / (m+n);
  if (m == r) h = (g-b)/d+(g<b?6:0);
  if (m == g) h = (b-r)/d+2;
  if (m == b) h = (r-g)/d+4;
  h /= 6.0;
  *hv = (float)h;
  *sv = (float)s;
  *lv = (float)l;
}

void updateColorWheel(float h, int mx, int my, double *hv, double *sv, double *lv) {
  const double a = -h * 2 * PI;
  const double wheelPixels = 16.0;
  unsigned int *rgba = glGetTexturePointer(colorWheelTexture);
  double s = 1.0/(1.0/sqrt(2.0));
  for (int y = 0; y < colorWheelY; y++) {
    double fy = (double)y / (colorWheelY-1) * 2.0 - 1.0;
    for (int x = 0; x < colorWheelX; x++) {
      double fx = (double)x / (colorWheelX-1) * 2.0 - 1.0;
      double d = sqrt(fx*fx+fy*fy);
      unsigned int c = 0;
      if (d < 1.0 && d > 1.0 - wheelPixels/colorWheelX) {
        float h = atan2(fx,fy)/PI*0.5+0.5;
        Vector c2 = hsl(h,1,0.5);
        int ri = c2.x * 255.0;
        int gi = c2.y * 255.0;
        int bi = c2.z * 255.0;
        c = ri|(gi<<8)|(bi<<16)|0xff000000;
        if ((hv != NULL) && ((int)mx==x) && ((int)my==y)) *hv = h;
      }
      double fx2 = (-sin(a)*fx+cos(a)*fy)*s;
      double fy2 = (cos(a)*fx+sin(a)*fy)*s;
      fx2 += 0.25;
      if (fx2 > -1 && fx2 < 1 && fy2 > -1 && fy2 < 1) {
         if(fabs(fy2)<fx2*0.5+0.5) {
          double hx = fx2 * 0.5 + 0.5;
          double hy = fy2 * 0.5 + 0.5;
          Vector c2 = hsl(h,1.0-hx,hy-0.5+0.5);
          int ri = c2.x * 255.0;
          int gi = c2.y * 255.0;
          int bi = c2.z * 255.0;
          c = ri|(gi<<8)|(bi<<16)|0xff000000;
          if ((sv != NULL) && ((int)mx==x) && ((int)my==y)) *sv = 1.0-hx;
          if ((lv != NULL) && ((int)mx==x) && ((int)my==y)) *lv = hy-0.5+0.5;
        }
      }
      rgba[x+y*colorWheelX] = c;
    }
  }
}

void dialogColor(bool openIt, unsigned int *color) {
  static unsigned int *dest = NULL;
  static Vector c;
  static unsigned int krgb;
  if (color != NULL) {dest = color; c = Vector(((*color)&255)/255.0,((*color>>8)&255)/255.0,((*color>>16)&255)/255.0,((*color>>24)&255)/255.0); float h,s,l; getHsl(c,&h,&s,&l); updateColorWheel(h,0,0,NULL,NULL,NULL);}
  DIALOGSTART("Color select")
  float k;
  k = c.w * 255.0; if (ImGui::SliderFloat("Alpha##DIALOGCOLORCOLORALPHA",&k,0,255)) {c.w = k/255.0;}
  k = c.x * 255.0; if (ImGui::SliderFloat("Red##DIALOGCOLORCOLORRED",&k,0,255)) {c.x = k/255.0;}
  k = c.y * 255.0; if (ImGui::SliderFloat("Green##DIALOGCOLORCOLORGREEN",&k,0,255)) {c.y = k/255.0;}
  k = c.z * 255.0; if (ImGui::SliderFloat("Blue##DIALOGCOLORCOLORBLUE",&k,0,255)) {c.z = k/255.0;}
  float h,s,l; 
  getHsl(c,&h,&s,&l);
  k = h*360.0;  if (ImGui::SliderFloat("Hue##DIALOGCOLORCOLORHUE",&k,0,360)) {double w = c.w; h = k/360.0; c = hsl(h,s,l); c.w = w; updateColorWheel(h,0,0,NULL,NULL,NULL);}
  k = s;  if (ImGui::SliderFloat("Saturation##DIALOGCOLORCOLORSAT",&k,0,1)) {double w = c.w; s = k; c = hsl(h,s,l); c.w = w;}
  k = l;  if (ImGui::SliderFloat("Lightness##DIALOGCOLORCOLORLIT",&k,0,1)) {double w = c.w; l = k; c = hsl(h,s,l); c.w = w;}
  ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0,0,0,0));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(0,0,0,0));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,ImVec4(0,0,0,0));
  ImVec2 cursorHere = ImGui::GetCursorScreenPos();
  if (ImGui::ImageButton((ImTextureID)colorWheelTexture,ImVec2(colorWheelX,colorWheelY))) {
    double hv = -1;
    double hs = -1;
    double hl = -1;
    int mx = ImGui::GetMousePos().x-cursorHere.x;
    int my = ImGui::GetMousePos().y-cursorHere.y;
    updateColorWheel(h,mx,my,&hv,&hs,&hl);  
    if (hv >= 0) {double w = c.w; h = hv; c = hsl(h,s,l); c.w = w; updateColorWheel(h,0,0,NULL,NULL,NULL); }
    if (hs >= 0) {double w = c.w; s = hs; c = hsl(h,s,l); c.w = w;}
    if (hl >= 0) {double w = c.w; l = hl; c = hsl(h,s,l); c.w = w;}
  }
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  ImGui::SameLine();
  ImVec4 col(c.x,c.y,c.z,c.w);
  ImGui::Image((ImTextureID)0, ImVec2(64,64), ImVec2(0,0), ImVec2(1,1), col);
  int ri = c.x * 255.0;
  int gi = c.y * 255.0;
  int bi = c.z * 255.0;
  int ai = c.w * 255.0;
  if (ri < 0) ri = 0;
  if (gi < 0) gi = 0;
  if (bi < 0) bi = 0;
  if (ai < 0) ai = 0;
  if (ri > 255) ri = 255;
  if (gi > 255) gi = 255;
  if (bi > 255) bi = 255;
  if (ai > 255) ai = 255;
  krgb = ri|(gi<<8)|(bi<<16)|(ai<<24);
  unsigned int krgbBefore = krgb;
  colorItem("","##DIALOGCOLORCOLORHEX",&krgb, false);
  if (krgbBefore != krgb) {
    c = Vector((krgb&255)/255.0,((krgb>>8)&255)/255.0,((krgb>>16)&255)/255.0,((krgb>>24)&255)/255.0);
  }
  if (ImGui::Button("Set##DIALOGCOLORCOLORSET")) {
    if (dest != NULL) {
      *dest = krgb;
    }
    renderingUpdated = true;
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button("Close##DIALOGCOLORCOLORCLOSE")) {
    ImGui::CloseCurrentPopup();
  }
  DIALOGEND()
}

void colorItem(const char *name, const char *id, unsigned int *color, bool colorPicker) {
  ImVec4 cl = ImVec4(((*color)&255)/255.0,((*color>>8)&255)/255.0,((*color>>16)&255)/255.0,((*color>>24)&255)/255.0);
  if (colorPicker) {
    ImGui::PushID((String(id)+String("imageButton_")).c_str());
    ImGui::PushStyleColor(ImGuiCol_Button,cl.w==1?ImVec4(1-cl.x,1-cl.y,1-cl.z,1):ImVec4(0,0,0,1));
    bool a = ImGui::ImageButton((ImTextureID)0, ImVec2(11,11), ImVec2(0,0), ImVec2(1,1), -1, cl,cl);
    ImGui::PopStyleColor();
    ImGui::PopID();
    if (a) {dialogColor(true,color);}
    ImGui::SameLine();
  }
  ImGui::Text("%s#",name);
  ImGui::SameLine();
  static char colorText[11];
  sprintf(colorText,"%08x",*color);
  for (int i = 0; i < strlen(colorText); i++) if (colorText[i]>='a'&&colorText[i]<='f') colorText[i]=(colorText[i]-'a')+'A';
  ImGui::PushItemWidth(7*8);
  if (ImGui::InputText(id,colorText,9)) {
    unsigned int value = 0;
    for (int i = 0; i < 8; i++) {
      value <<= 4;
      char c = colorText[i];
      if (c>='0'&&c<='9') value += c-'0';
      if (c>='a'&&c<='f') value += c-'a'+10;
      if (c>='A'&&c<='F') value += c-'A'+10;
    }
    if (*color != value) renderingUpdated = true;
    *color = value;
  }
  ImGui::PopItemWidth();
}

void dialogObjectTextures(bool openIt) {
  int buttonSize = main_window_xres/6;
  double aspect2 = aspectRatio*main_window_yres/main_window_xres;
  DIALOGSTART("Object textures")
  ImGui::Text("Max. texture width:%d height:%d",maxTextureWidth,maxTextureHeight);
  ImGui::BeginChild("DIALOGOBJECTTEXTURESCHILD", ImVec2((buttonSize+16)*3,(buttonSize+16)*2));
  for (int i = 0; i < gameObjects.size(); i++) {
    if (gameObjects[i]->selected&&gameObjects[i]->type!=GAMEOBJECT_SECTION) {
      ImGui::Text("%d\"%s\"",i,gameObjects[i]->name);
      FileBinding *binding = getBindingById(gameObjects[i]->fileBindingId);
      if (binding != NULL) {
        for (int j = 0; j < binding->textureIds.size(); j++) {
          if ((j % 3)!=0) ImGui::SameLine();
          unsigned int textureId = binding->textureIds[j];
          textureButton((String("DIALOGOBJECTTEXTURESCHILDIMAGE")+String::fromInt(i)+"_"+String::fromInt(j)).c_str(),textureId,buttonSize,buttonSize*aspect2,true);
        }
      }
    }
  }
  ImGui::EndChild();
  if (ImGui::Button("Close##DIALOGOBJECTTEXTURESCLOSE")) {
    ImGui::CloseCurrentPopup();
  }
  DIALOGEND()
}

#define LONGFILENAME(__str__) (longFileNames ? longFileName(__str__) : __str__)
#define SHORTFILENAME(__str__) (longFileNames ? shortFileName(__str__) : __str__)
char longFileNameBuffer[MAX_LFN_PATH];

String longFileName(const String &a) {
  if (a[0]>='A'&&a[0]<='B') return a;
  if (!doslfnLongPath(longFileNameBuffer,dosFileName(a).c_str())) return a;
  return unixFileName(String(longFileNameBuffer));
}

String shortFileName(const String &a) {
  if (a[0]>='A'&&a[0]<='B') return a;
  if (!doslfnShortPath(longFileNameBuffer,dosFileName(a).c_str())) return a;
  return unixFileName(String(longFileNameBuffer));
}


void sortFileNames(Array<File> &files, int column, bool sortOrder) {
  for (int i = 0; i < files.size(); i++) {
    for (int j = i+1; j < files.size(); j++) {
      bool bigger = false;
      String nameHere = (files[i].name.name + files[i].name.extension);
      bool dirHere = files[i].attributes & DOS_ATTR_SUBDIR;
      bool dirThere = files[j].attributes & DOS_ATTR_SUBDIR;
      switch(dirHere?0:column) {
      case 0: bigger = nameHere < (files[j].name.name + files[j].name.extension); break;
      case 1: bigger = (files[i].name.extension + files[i].name.name) < (files[j].name.extension + files[j].name.name); break;
      case 2: bigger = files[i].size < files[j].size; break;
      case 3: bigger = files[i].date < files[j].date; break;
      }
      if ((!dirHere)&&sortOrder) bigger = !bigger;
      if (dirHere != dirThere) {
        if (dirHere) bigger = false; else bigger = true;
      }
      if (bigger) {
        File t = files[i];
        files[i] = files[j];
        files[j] = t;
      }
    }
  }
}

typedef void (*LOADSAVEFUNC)(const char *shortPath, const char *longFileName);
unsigned int dialogLoadSaveGameObjectUniqueId = 0;
String dialogLoadSaveSetFileNameAndPath;

void dialogLoadSave(bool openIt, int dialogMode) {

  static int mode = DIALOG_NONE; if (dialogMode != DIALOG_NONE) mode = dialogMode;

  static const char *dialogCaption[DIALOG_COUNT] = {"NO MODE","Load a scene","Save a scene","Load a skybox","Load a mesh"};
  static const char *proceedButton[DIALOG_COUNT] = {"<button>", "Load", "Save","Load","Load"};
  static LOADSAVEFUNC funcs[DIALOG_COUNT] = {NULL, loadScene, saveScene, loadSkyBox, loadMesh};
  static char filter[NAMEMAX];

  static bool driveReady = true;
  static bool needsRescanFiles = false;
  static bool longFileNames = false;
  static bool initColumns = false;
  static Array<char> driveLetters;
  static Array<File> files;
  static int sortByColumn = 0;
  static bool sortingOrder[4] = {false};
  static String paths[DIALOG_COUNT];
  static String fileNames[DIALOG_COUNT];
  static String path2;
  static String scrollToFileName;
  static int scrollToHelper = 0;
  const int NAMEMAXHERE = 100;
  if (openIt) {
    initColumns = true;
    memset(filter,0,sizeof(filter));
    needsRescanFiles = true;
    driveLetters.clear();
    for (int i = 'A'; i <= 'Z'; i++) {
      if (dosHasDrive(i)) driveLetters.push_back(i);
    }
    for (int j = 0; j < DIALOG_COUNT; j++) {
      fileNames[j] = ""; // little bug
      fileNames[j].resize(NAMEMAXHERE);
    }
    scrollToHelper=0;
    if (!dialogLoadSaveSetFileNameAndPath.empty()) {
      dialogLoadSaveSetFileNameAndPath = unixFileName(SHORTFILENAME(dialogLoadSaveSetFileNameAndPath));
      fileNames[mode] = getName(dialogLoadSaveSetFileNameAndPath) + getExtension(dialogLoadSaveSetFileNameAndPath);
      scrollToFileName = toLower(fileNames[mode]);
      scrollToHelper = 2;
      fileNames[mode].resize(NAMEMAXHERE);
      paths[mode] = getPath(dialogLoadSaveSetFileNameAndPath);
      dialogLoadSaveSetFileNameAndPath = "";
    }
  }
  bool result = false;
  DIALOGSTART(dialogCaption[mode])
  if (paths[mode].empty()) paths[mode] = getCWD();
  if (needsRescanFiles) {
    needsRescanFiles = false;
    files.clear();
    char driveLetter = paths[mode][0];
    driveReady = true;
    if (driveLetter>='A'&&driveLetter<='B') {driveReady = dosIsDriveReady(driveLetter);}
    if (driveReady) files = dosGetFiles(paths[mode]);
    for (int i = 0; i < files.size(); i++) {
      if (files[i].name.name == "." || files[i].name.name == "..") {files.erase(i,1);i--;} else
      if ((files[i].attributes & 0x1f) != DOS_ATTR_SUBDIR && (files[i].attributes & 0x1f) != DOS_ATTR_NORMAL) {files.erase(i,1);i--;}
    }
    if (longFileNames) {
      for (int i = 0; i < files.size(); i++) {
          files[i].name.full = LONGFILENAME(files[i].name.full);
          files[i].name.name = getName(files[i].name.full);
          files[i].name.extension = getExtension(files[i].name.full);
      }
    }
    sortFileNames(files, sortByColumn, sortingOrder[sortByColumn]);
    path2 = LONGFILENAME(paths[mode]);
  }
  ImGui::Text("Long filenames"); ImGui::SameLine(); if(ImGui::Checkbox("##DIALOGLOADSAVELONGFILENAMES", &longFileNames)) needsRescanFiles = true;
  ImGui::SameLine();
  for (int i = 0; i < driveLetters.size(); i++) {
    if (i != 0) ImGui::SameLine();
    char a = driveLetters[i];
    if (ImGui::Button((String::fromChar(a)+":##DIALOGLOADSAVEDRIVELETTER"+toString(i)).c_str())) {
      paths[mode] = String::fromChar(a) + ":/";
      needsRescanFiles = true;
      //log_setBiosCursor(0,0);
    }
  }
  ImGui::Separator();
  SVG *svg = &svgs["search"];ImGui::Image((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h));ImGui::SameLine();ImGui::InputText("##DIALOGLOADSAVEFILTER",filter,NAMEMAX);
  ImGui::Separator();
  ImGui::TextWrapped("%s",path2.c_str());
  ImGui::Separator();
  if (paths[mode].length() > 3 && ImGui::Button("..##DIALOGLOADSAVEPATHBACK")) {
    int k = paths[mode].substr(0,paths[mode].length()-1).findLast("/");
    if (k>=0) paths[mode] = paths[mode].substr(0,k+1);
    needsRescanFiles = true;
  }
  ImGui::SameLine(); ImGui::Text("Sort by:"); ImGui::SameLine();
  for (int l = 0; l < 4; l++) {
    if (l!=0) ImGui::SameLine();
    static const char *headerButtonNames[4] = {"Filename##DIALOGLOADSAVEFILENAMES","Ext##DIALOGLOADSAVEEXTS","Size##DIALOGLOADSAVESIZES","Date##DIALOGLOADSAVEDATES"};
    if (ImGui::Button(headerButtonNames[l])) {
      sortByColumn = l;
      sortingOrder[l] = !sortingOrder[l];
      needsRescanFiles = true;
    }
  }
  ImGui::Separator();
  int w = main_window_xres/2;
  int h = main_window_yres/2;

  if (!driveReady) {
    if (ImGui::Button("Drive not ready.. Retry.##DIALOGLOADSAVERETRY")) {
      needsRescanFiles = true;
    }
  } else {
    ImGui::BeginChild("DIALOGLOADSAVECHILD",ImVec2(w,h));
    ImGui::Columns(4);
    String filter2 = toLower(filter);  
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(1,0));  
    for (int j = 0; j < 4; j++) {
      if (initColumns) {
          if (j == 1) ImGui::SetColumnOffset(j,ImGui::GetColumnOffset(j-1)+7*20);
          if (j == 2) ImGui::SetColumnOffset(j,ImGui::GetColumnOffset(j-1)+7*5);
          if (j == 3) ImGui::SetColumnOffset(j,ImGui::GetColumnOffset(j-1)+7*10);
      }
      for (int i = 0; i < (int)files.size(); i++) {
        String fullFileName = toLower(files[i].name.name + files[i].name.extension);
        if (!filter2.empty() && fullFileName.findFirst(filter2)<0) continue;
        bool isDir = files[i].attributes & DOS_ATTR_SUBDIR;
        if (isDir) {
          if (j == 0) {
            ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0,0,0,0));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,ImVec4(0,0,0,0.5));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(0,0,0,0));
            if (ImGui::Button((files[i].name.name+"##DIALOGLOADSAVEDIRBUTTON"+String::fromInt(i)).c_str())) {
              paths[mode] += files[i].name.name + files[i].name.extension + "/";
              paths[mode] = SHORTFILENAME(paths[mode]);
              fileNames[mode] = "";
              needsRescanFiles = true;
            }
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
          } 
          if (j == 1) {
            ImGui::Text("DIR");
          }
          if (j == 2) {
            ImGui::Text("");
          }
          if (j == 3) {
            FileTime t = files[i].date;
            ImGui::Text("%02d.%02d.%04d %02d:%02d.%02d",t.day,t.month,t.year,t.hour,t.minute,t.second);
          }
        } else {
          if (j == 0) {
            ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0,0,0,0));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,ImVec4(0,0,0,0.5));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(0,0,0,0));
            if (ImGui::Button((files[i].name.name+"##DIALOGLOADSAVEFILEBUTTON"+String::fromInt(i)).c_str())) {
              fileNames[mode] = files[i].name.name + files[i].name.extension;
              fileNames[mode].resize(NAMEMAXHERE);
            }
            if (fullFileName==scrollToFileName) {ImGui::SetScrollHere();}
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
          }
          if (j == 1) {
            ImGui::Text("%s",files[i].name.extension.c_str());
          }
          if (j == 2) {
            ImGui::Text("%db",files[i].size);
          }
          if (j == 3) {
            FileTime t = files[i].date;
            ImGui::Text("%02d.%02d.%04d %02d:%02d.%02d",t.day,t.month,t.year,t.hour,t.minute,t.second);
          }
        }
      }
      ImGui::NextColumn();
    }
    ImGui::PopStyleVar();  
    initColumns = false;
    ImGui::EndChild();
  }
  ImGui::Separator();
  ImGui::PushItemWidth(w);
  ImGui::InputText("##DIALOGLOADSAVEFILENAME",&fileNames[mode][0],NAMEMAXHERE);
  ImGui::PopItemWidth();
  ImGui::Separator();

  if (ImGui::Button((String(proceedButton[mode])+String("##DIALOGLOADSAVEPROCEED")).c_str())) {
    result = true;
    ImGui::CloseCurrentPopup();
  }
  ImGui::SameLine();
  if (ImGui::Button("Close##DIALOGLOADSAVECLOSE")) {
    ImGui::CloseCurrentPopup();
  }
  DIALOGEND()
  if (scrollToHelper>0) scrollToHelper--; else scrollToFileName="";
  if (result) {
    if (funcs[mode]!=NULL) funcs[mode](paths[mode].c_str(),fileNames[mode].c_str());
  }
}

void dialogMessage(bool openIt, const char *message) {
  static String alertMessage; if (message != NULL) alertMessage = message;
  DIALOGSTART("ALERT MESSAGE")
  ImGui::Text("%s",alertMessage.c_str());
  ImGui::Separator();
  if (ImGui::Button("Close##DIALOGALERTMESSAGECLOSE")) {
    alertMessage = "<no message>";
    ImGui::CloseCurrentPopup();
  }
  DIALOGEND()
}


void dialogViewBindings(bool openIt) {
  static Array<bool> checked;
  checked.resize(bindings.size());
  if (openIt) {for (int i = 0; i < checked.size(); i++) checked[i] = false;}
  DIALOGSTART("View bindings")
  ImGui::Text("Bindings count:%d",bindings.size());
  ImGui::Separator();
  int w = main_window_xres/2;
  int h = main_window_yres/2;
  ImGui::BeginChild("DIALOGVIEWBINDINGSCHILD",ImVec2(w,h));
  for (int i = 0; i < bindings.size(); i++) {
    ImGui::Checkbox((String("##DIALOGVIEWBINDINGSTREE")+String::fromInt(i)).c_str(), &checked[i]);
    ImGui::SameLine(); ImGui::TextWrapped("%s ID:%d FileName:%s",fileTypeNames[bindings[i]->type],bindings[i]->uniqueId,bindings[i]->fileName.last(30).c_str());
    if (checked[i]) {
      for (int j = 0; j < gameObjects.size(); j++) {
        if (gameObjects[j]->fileBindingId == bindings[i]->uniqueId) {
          ImGui::Text("%d\"%s\"",j,gameObjects[j]->name);
        }
      }
    }
  }
  ImGui::EndChild();
  ImGui::Separator();
  if (ImGui::Button("Close##DIALOGVIEWBINDINGSCLOSE")) {
    ImGui::CloseCurrentPopup();
  }
  DIALOGEND()
}

void clippy(double *v) {
  if (*v < 0.0001) *v = 0.0001;
}

void dialogGridSettings(bool openIt) {
  DIALOGSTART("Grid Settings")
  Grid *grid = &grids[0];
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(1,0));
  ImGui::Text("Translation/Position");
  if (vectorInput(" XYZ",&grid->posGranularity,"##DIALOGGRIDSETTINGSISGRIDTR")) {renderingUpdated = true;}
  ImGui::Text(" Grid on X"); ImGui::SameLine();if (ImGui::Checkbox("##DIALOGGRIDSETTINGSISGRIDTX",&grid->posXEnabled)) {renderingUpdated = true;}
  ImGui::SameLine();
  ImGui::Text("Grid on Y"); ImGui::SameLine();if (ImGui::Checkbox("##DIALOGGRIDSETTINGSISGRIDTY",&grid->posYEnabled)) {renderingUpdated = true;}
  ImGui::SameLine();
  ImGui::Text("Grid on Z"); ImGui::SameLine();if (ImGui::Checkbox("##DIALOGGRIDSETTINGSISGRIDTZ",&grid->posZEnabled)) {renderingUpdated = true;}
  ImGui::Separator();
  ImGui::Text("Scaling");
  if (vectorInput(" XYZ",&grid->scaleGranularity,"##DIALOGGRIDSETTINGSISGRIDSC")) {renderingUpdated = true;}
  ImGui::Text(" Grid on X"); ImGui::SameLine();if (ImGui::Checkbox("##DIALOGGRIDSETTINGSISGRIDSX",&grid->scaleXEnabled)) {renderingUpdated = true;}
  ImGui::SameLine();
  ImGui::Text("Grid on Y"); ImGui::SameLine();if (ImGui::Checkbox("##DIALOGGRIDSETTINGSISGRIDSY",&grid->scaleYEnabled)) {renderingUpdated = true;}
  ImGui::SameLine();
  ImGui::Text("Grid on Z"); ImGui::SameLine();if (ImGui::Checkbox("##DIALOGGRIDSETTINGSISGRIDSZ",&grid->scaleZEnabled)) {renderingUpdated = true;}
  ImGui::Separator();
  ImGui::Text("Rotation");
  float k;
  ImGui::PushItemWidth(64);
  k = grid->rotGranularity; ImGui::Text(" Angle"); ImGui::SameLine(); if (ImGui::InputFloat("##DIALOGGRIDSETTINGSISGRIDR",&k)) {grid->rotGranularity = k; renderingUpdated = true;}
  ImGui::PopItemWidth();
  ImGui::Text(" Grid on Angle"); ImGui::SameLine(); if (ImGui::Checkbox("##DIALOGGRIDSETTINGSISGRIDRA",&grid->rotEnabled)) { renderingUpdated = true;}
  ImGui::Separator();
  ImGui::PopStyleVar();
  clippy(&grid->rotGranularity);
  clippy(&grid->posGranularity.x);
  clippy(&grid->posGranularity.y);
  clippy(&grid->posGranularity.z);
  clippy(&grid->scaleGranularity.x);
  clippy(&grid->scaleGranularity.y);
  clippy(&grid->scaleGranularity.z);
  if (ImGui::Button("Close##DIALOGGRIDSETTINGSCLOSE")) {
    ImGui::CloseCurrentPopup();
  }
  DIALOGEND()
}

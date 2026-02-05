#include "GL.H"
#include "IMGUIGL.HPP"
#include "CONFIG.HPP"
#include "DIALOGS.HPP"
#include "GAMEOBJ.HPP"
#include "OUTLINER.HPP"
#include "RENDER.HPP"
#include <math.h>

#define DIALOGSTART(__name__) static bool openItNextTime = false;  if (openIt) {openItNextTime = true; return;}  if (openItNextTime) {openItNextTime = false; ImGui::OpenPopup(__name__); } ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0,0,0,1)); ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1,1,1,1); if (ImGui::BeginPopupModal(__name__,NULL,ImGuiWindowFlags_AlwaysAutoResize)) { ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(0,0,0,1); ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.4,0.4,0.4,1));
#define DIALOGEND() ImGui::PopStyleColor(); ImGui::EndPopup();} ImGui::PopStyleColor();

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
  if (ImGui::Button("Yes##DIALOGDELETEOBJECTSYES")) {
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
  if (ImGui::Button("Yes##DIALOGQUITYES")) {
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
  ImGui::Text("Name"); ImGui::SameLine(); ImGui::InputText("##DIALOGADDGAMEOBJECTNAME", name, NAMEMAX);
  if (ImGui::Button("Add##DIALOGADDGAMEOBJECTADD")) {
    GameObject *o = new GameObject();
    o->setName(name);
    o->setType(t);
    gameObjects.insert(o,0);
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
  ImGui::Text("Name"); ImGui::SameLine(); ImGui::InputText("##DIALOGADDPROPERTYNAME", p.name, NAMEMAX);
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
    "Keyboard:\n"
    "You can alter the move speed in the toolbar.\n"
    "Use W to move forward.\n"
    "Use S to move backward.\n"
    "Use D to move right.\n"
    "Use A to move left.\n"
    "Use E to move up.\n"
    "Use Q to move down.\n"
    "Mouse:\n"
    "Right mouse button and mouse to rotate view.\n"
    "Right mouse button and shift and mouse to roll.\n"
    "Right mouse button and ctrl and mouse to pan.\n"
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

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
#include "IMGUIGL.HPP"
#include "VIEWPORT.HPP"
#include "OUTLINER.HPP"
#include "PROPS.HPP"
#include "CONFIG.HPP"
#include "MENUBAR.HPP"
#include "TOOLBAR.HPP"
#include "TIMELINE.HPP"
#include "RENDER.HPP"
#include "GAMEOBJ.HPP"
#include "INFO.HPP"
#include "DIALOGS.HPP"
#include "PAINT.HPP"
#include "STL/VECTOR.HPP"
#include "STL/QUATERNN.HPP"
#include <math.h>

bool viewportOn = false;
bool toolOn = false;
bool showOverlays = true;
Array<Overlay> overlays;
Vector cursor(0,0,0);
Quaternion cursorOrientation;
bool placeCursor = true;

float minScale = 0.0001;
Vector moveV(0,0,0); bool move_negate = false;
Vector scaleV(1,1,1); bool scale_negate = false;
Vector rotateV(0,0,0); bool rotate_negate = false;

void viewportLine(float cx, float cy, float lx, float ly, unsigned int color, const char *text) {
  ImVec2 p = ImGui::GetWindowPos();
  cx += p.x;
  cy += p.y;
  ImGui::GetWindowDrawList()->AddLine(ImVec2(cx,cy),ImVec2(cx+lx,cy+ly),color);
  cx -= 4;
  cy -= 6;
  ImGui::GetWindowDrawList()->AddText(ImGui::GetWindowFont(),ImGui::GetWindowFontSize(),ImVec2(cx+lx,cy+ly),color,text);
}

void clearOverlays() {
  overlays.clear();
}

void getRenderWindowPosAndSize(ImVec2 *pos, ImVec2 *size) {
  int x0 = (outliner_width);
  int x1 = (main_window_xres-properties_width);
  int y0 = (menuBar_height + toolBar_height);
  int y1 = (main_window_yres-timeLine_height);
  pos->x = x0;
  pos->y = y0;
  size->x = x1-x0;
  size->y = y1-y0;
}

void addTextOverlay(double renderX,double renderY, unsigned int col, const char *text) {
  ImVec2 windowPos,windowSize;getRenderWindowPosAndSize(&windowPos, &windowSize);
  Overlay p;
  p.x = (renderX * 0.5 + 0.5)*windowSize.x + windowPos.x;
  p.y = (-renderY * 0.5 + 0.5)*windowSize.y + windowPos.y;
  p.text = text;
  p.color = col;
  overlays.push_back(p);
}

void paintOverlays() {
  if (!showOverlays) return;
  for (int i = 0; i < overlays.size(); i++) {
    Overlay *o = &overlays[i];
    ImVec2 k = ImVec2(o->x,o->y);
    k.x -= 2;
    k.y -= 5;
    ImGui::GetWindowDrawList()->AddText(ImGui::GetWindowFont(),ImGui::GetWindowFontSize(),k,0xff000000,o->text.c_str());
    k.x -= 1;
    k.y -= 1;
    ImGui::GetWindowDrawList()->AddText(ImGui::GetWindowFont(),ImGui::GetWindowFontSize(),k,o->color,o->text.c_str());
  }
}

void drawCameraRotationGizmo() {
  Camera *c = &cameras[currentCamera];
  ImVec2 k = ImGui::GetWindowSize();
  int o = 5;
  float radX = 40;
  float radY = 40;
  Quaternion rot = normalize(inverse(c->rotation));
  Vector p[3];
  p[0] = rot.rotateVector(Vector(1,0,0));
  p[1] = rot.rotateVector(Vector(0,1,0));
  p[2] = rot.rotateVector(Vector(0,0,1));
  int d[3] = {0,1,2}; for (int i = 0; i < 3*2; i++) if (p[d[i % 3]].z < p[d[(i+1)%3]].z) {int t = d[i%3]; d[i % 3]=d[(i+1)%3];d[(i+1)%3]=t;}
  if (aspectRatio != 0) radX /= aspectRatio * main_window_yres / main_window_xres;
  for (int j = 0; j < 3; j++) {
    if (d[j] == 0) viewportLine(k.x-radX-o,radY+o,p[0].x*radX,-p[0].y*radY,0xff0000ff,"X");
    if (d[j] == 1) viewportLine(k.x-radX-o,radY+o,p[1].x*radX,-p[1].y*radY,0xff00ff00,"Y");
    if (d[j] == 2) viewportLine(k.x-radX-o,radY+o,p[2].x*radX,-p[2].y*radY,0xffff0000,"Z");
  }
}

void moveBy(float x, float y, float z, bool negate) {
  if (x == 0 && y == 0 && z == 0) return;
  x *= negate ? -1 : 1;
  y *= negate ? -1 : 1;
  z *= negate ? -1 : 1;
  for (int i2 = 0; i2 < gameObjects.size(); i2++) {
    GameObject *o =gameObjects[i2];
    if (o->selected) {
      o->translation.x += x;
      o->translation.y += y;
      o->translation.z += z;
    }
  }
  renderingUpdated = true;
}

void scaleBy(float x, float y, float z, bool negate) {
  if (x == 1 && y == 1 && z == 1) return;
  if (negate) {
    if (x != 0) x = 1.0/x;
    if (y != 0) y = 1.0/y;
    if (z != 0) z = 1.0/z;
  }
  for (int i2 = 0; i2 < gameObjects.size(); i2++) {
    GameObject *o =gameObjects[i2];
    if (o->selected) {
      o->scaling.x *= x;
      o->scaling.y *= y;
      o->scaling.z *= z;
    }
  }
  renderingUpdated = true;
}

void rotateBy(float x, float y, float z, bool negate) {
  if (x == 0 && y == 0 && z == 0) return;
  x *= negate ? -1 : 1;
  y *= negate ? -1 : 1;
  z *= negate ? -1 : 1;
  for (int i2 = 0; i2 < gameObjects.size(); i2++) {
    GameObject *o =gameObjects[i2];
    if (o->selected) {
      o->rotation.x += x;
      o->rotation.y += y;
      o->rotation.z += z;
    }
  }
  renderingUpdated = true;
}

void duplicate() {
  for (int i2 = 0; i2 < gameObjects.size(); i2++) {
    GameObject *o =gameObjects[i2];
    if (o->selected) {
      GameObject *o2 = o->clone();
      cloneName(o2->name, o->name);
      getTransformForGameObjectFromCursor(o2);
      o2->selected = false;
      gameObjects.insert(o2,i2+1);
      i2++;
    }
  }
  renderingUpdated = true;
}

void minMaxOfGameObject(const GameObject *o, Vector *m, Vector *n) {
  for (int i = 0; i < 8; i++) {
    Vector k = o->getRenderTransform() * Vector((i&1)?o->boundingBoxMax.x:o->boundingBoxMin.x,(i&2)?o->boundingBoxMax.y:o->boundingBoxMin.y,(i&4)?o->boundingBoxMax.z:o->boundingBoxMin.z);
    if (i == 0 || k.x < n->x) n->x = k.x;
    if (i == 0 || k.y < n->y) n->y = k.y;
    if (i == 0 || k.z < n->z) n->z = k.z;
    if (i == 0 || k.x > m->x) m->x = k.x;
    if (i == 0 || k.y > m->y) m->y = k.y;
    if (i == 0 || k.z > m->z) m->z = k.z;
  }
}

void centerSelection() {
  bool first = true;
  Vector n2,m2;
  for (int i2 = 0; i2 < gameObjects.size(); i2++) {
    GameObject *o =gameObjects[i2];
    if (o->selected) {
      Vector n,m;
      minMaxOfGameObject(o,&m,&n);
      if (first || n.x < n2.x) n2.x = n.x;
      if (first || n.y < n2.y) n2.y = n.y;
      if (first || n.z < n2.z) n2.z = n.z;
      if (first || m.x > m2.x) m2.x = m.x;
      if (first || m.y > m2.y) m2.y = m.y;
      if (first || m.z > m2.z) m2.z = m.z;
      first = false;
    }
  }
  if (m2 != n2) {
    Vector center = (m2 + n2)*0.5;
    double radius = length(m2 - center);
    Camera *c = &cameras[currentCamera];
    c->position = center - c->rotation.rotateVector(Vector(0,0,-radius*sqrt(3)));
    renderingUpdated = true;
  }
}

void setOrthoLookTo(double x, double y, double z) {
  Camera *c = &cameras[currentCamera];
  c->ortho = true;
  c->rotation.yawPitchRoll(-x*90*PI/180.0,y*90*PI/180.0+(z>0?180*PI/180.0:0),z*90*PI/180.0+(z!=0?90*PI/180.0:0));
  renderingUpdated = true;
}

namespace ImGui {
  void Text2(const char *text) {
    ImVec2 c = ImGui::GetCursorScreenPos();
    c.x += 1;
    c.y += 1;
    ImGui::GetWindowDrawList()->AddText(ImGui::GetWindowFont(),ImGui::GetWindowFontSize(),ImVec2(c.x,c.y),0xc0000000,text);
    c.x -= 1;
    c.y -= 1;
    ImGui::GetWindowDrawList()->AddText(ImGui::GetWindowFont(),ImGui::GetWindowFontSize(),ImVec2(c.x,c.y),0xffffffff,text);
    String text2 = text;
    for (int i = 0; i < text2.length(); i++) text2[i] = ' ';
    ImGui::Text(text2.c_str());
  }
}

void drawViewport() {
  ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(0,0,0,0));

  ImGui::SetNextWindowPos(ImVec2(outliner_width,menuBar_height+toolBar_height));
  ImGui::Begin("VIEWPORT", NULL, ImVec2(main_window_xres-outliner_width-properties_width,main_window_yres-menuBar_height-toolBar_height-timeLine_height-info_height),1.0,FROZENWINDOW);
  ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.25,0.25,0.25,1));
  ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.5,0.5,0.5,1));
  bool one = false; for (int i2 = 0; i2 < gameObjects.size(); i2++) if (gameObjects[i2]->selected && gameObjects[i2]->type != GAMEOBJECT_SECTION) {one=true; break;}
  if (one && mode != MODE_UNDEFINED && mode != MODE_SELECT) {
    ImGui::Text2("Tool"); ImGui::SameLine();
    if (ImGui::Checkbox("##VIEWPORTTOOLON",&toolOn)) {;}
    if (toolOn) {
      ImGui::PushItemWidth(15*7);
      switch(mode) {
      case MODE_MOVE: {
        if (ImGui::Button("Clear##VIEWPORTMOVECLEAR")) {moveV.set(0,0,0); move_negate = false;}
        ImGui::SameLine();ImGui::Text2("MoveXYZ"); ImGui::SameLine(); vectorInput("",&moveV,"##VIEWPORTMOVE");
        if (ImGui::Button(!move_negate ? "Add##VIEWPORTMOVEAPPLY" : "Sub##VIEWPORTMOVEAPPLY")) {
          moveBy(moveV.x,moveV.y,moveV.z, move_negate);
        }
        ImGui::SameLine(); if (ImGui::Checkbox("##VIEWPORTMOVENEGATE", &move_negate)) {;}
        ImGui::SameLine();
        if (ImGui::Button("Duplicate")) {
          duplicate();
          moveBy(moveV.x,moveV.y,moveV.z, move_negate);
        }
      } break;
      case MODE_SCALE: {
        if (ImGui::Button("Clear##VIEWPORTSCALECLEAR")) {scaleV.set(1,1,1); scale_negate = false;}
        ImGui::SameLine();ImGui::Text2("ScaleXYZ"); ImGui::SameLine(); vectorInput("",&scaleV,"##VIEWPORTSCALE");
        if (scaleV.x < minScale) scaleV.x = minScale;
        if (scaleV.y < minScale) scaleV.y = minScale; 
        if (scaleV.z < minScale) scaleV.z = minScale;
        if (ImGui::Button(!scale_negate ? "Add##VIEWPORTSCALEAPPLY" : "Sub##VIEWPORTSCALEAPPLY")) {
          scaleBy(scaleV.x,scaleV.y,scaleV.z, scale_negate);
        }
        ImGui::SameLine(); if (ImGui::Checkbox("##VIEWPORTSCALENEGATE", &scale_negate)) {;}
      } break;
      case MODE_ROTATE: {
        if (ImGui::Button("Clear##VIEWPORTROTCLEAR")) {rotateV.set(0,0,0); rotate_negate = false;}
        ImGui::SameLine();ImGui::Text2("RotABC"); ImGui::SameLine(); vectorInput("",&rotateV,"##VIEWPORTROT");
        if (ImGui::Button(!rotate_negate ? "Add##VIEWPORTROTAPPLY" : "Sub##VIEWPORTROTAPPLY")) {
          rotateBy(rotateV.x,rotateV.y,rotateV.z,rotate_negate);
        }
        ImGui::SameLine(); if (ImGui::Checkbox("##VIEWPORTROTATENEGATE", &rotate_negate)) {;}
      } break;
      }
      ImGui::PopItemWidth();
    }
  }
  drawCameraRotationGizmo();
  if (currentCamera == CAMERA_USER) {
    ImGui::Text2("Camera"); ImGui::SameLine();
    if (ImGui::Checkbox("##VIEWPORTOVERLAYON",&viewportOn)) {;}
    if (viewportOn) {
      ImGui::BeginChild("VIEWPORTSETTING");
      Camera *c = &cameras[currentCamera];
      float k;
      ImGui::PushItemWidth(15*7);
       ImGui::Text2("PosXYZ"); ImGui::SameLine(); if (vectorInput("",&c->position,"##VIEWPORTCAM")) {renderingUpdated = true;}
      Vector yawPitchRoll = c->rotation.getYawPitchRoll()*180/PI;
       ImGui::Text2("RotABC"); ImGui::SameLine(); if (vectorInput("",&yawPitchRoll,"##VIEWPORTYAWPITCHROLL")) {
        yawPitchRoll *= PI/180;
        c->rotation.yawPitchRoll(yawPitchRoll.x,yawPitchRoll.y,yawPitchRoll.z);
        renderingUpdated = true;
      }
      if (!c->ortho) {
        ImGui::Text2("Fov"); ImGui::SameLine(); if (ImGui::InputFloat("##VIEWPORTFOV",&c->fov,10,10)) {renderingUpdated = true;}
      } else {
        ImGui::Text2("Ortho scale"); ImGui::SameLine(); if (ImGui::InputFloat("##VIEWPORTORTHOSCALE",&c->orthoScale,10,10)) {renderingUpdated = true;}
      }
      if (c->orthoScale < 0) c->orthoScale = 0;
      if (c->fov < 1) c->fov = 1; if (c->fov>180) c->fov = 180;
      ImGui::Text2("Aspect ratio"); ImGui::SameLine();
  
      if (ImGui::InputFloat("##VIEWPORTASPECTRATIO",&c->aspect,0.1,0.1)) {renderingUpdated = true;}
      ImGui::PopItemWidth();
      ImGui::PushItemWidth(9*7);
      ImGui::Text2("Near plane"); ImGui::SameLine();
      if (ImGui::InputFloat("##VIEWPORTNEAR",&c->nearPlane)) {renderingUpdated = true;} if (c->nearPlane < 0.00001) c->nearPlane = 0.00001;
      ImGui::Text2("Far plane"); ImGui::SameLine();
      if (ImGui::InputFloat("##VIEWPORTFAR",&c->farPlane)) {renderingUpdated = true;} if (c->farPlane < c->nearPlane+0.00001) c->farPlane = c->nearPlane + 0.00001;
      ImGui::Text2("Background"); ImGui::SameLine();
      colorItem("","##VIEWPORTBACKGROUND", &clearColor, true);
      ImGui::PopItemWidth();
      ImGui::Text2("Cursor"); ImGui::SameLine(); if (vectorInput("",&cursor,"##VIEWPORTCURSOR")) {renderingUpdated = true; cursorOrientation = Quaternion();} 
      ImGui::SameLine(); bool isUp = cursorOrientation == Quaternion(); ImGui::Text2("Cursor up"); ImGui::SameLine(); if (ImGui::Checkbox("##VIEWPORTCURSORNORMALUP",&isUp)) {if (isUp) {cursorOrientation = Quaternion(); renderingUpdated = true;}}
      if (mode == MODE_SELECT) {
        ImGui::SameLine(); ImGui::Text2("Place cursor"); 
        ImGui::SameLine(); if (ImGui::Checkbox("##VIEWPORTPLACECURSOR",&placeCursor)) {renderingUpdated = true;}
      }
      ImGui::EndChild();
    }
  }
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  if (ImGui::IsAnyItemHovered()) if (ImGui::IsMouseDown(0)) {disableRenderMouseClicks();}
  paintOverlays();
  ImGui::End();

  ImGui::PopStyleColor();
}

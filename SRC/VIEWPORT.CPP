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
#include "STL/VECTOR.HPP"
#include "STL/QUATERNN.HPP"
#include "GIZMOS.HPP"

bool viewportOn = false;
bool toolOn = false;

float minScale = 0.0001;
float move_x = 0, move_y = 0, move_z = 0; bool move_negate = false;
float scale_x = 1, scale_y = 1, scale_z = 1; bool scale_negate = false;
float rotate_x = 0, rotate_y = 0, rotate_z = 0; bool rotate_negate = false;

void viewportLine(float cx, float cy, float lx, float ly, unsigned int color, const char *text) {
  ImVec2 p = ImGui::GetWindowPos();
  cx += p.x;
  cy += p.y;
  ImGui::GetWindowDrawList()->AddLine(ImVec2(cx,cy),ImVec2(cx+lx,cy+ly),color);
  cx -= 4;
  cy -= 6;
  ImGui::GetWindowDrawList()->AddText(ImGui::GetWindowFont(),ImGui::GetWindowFontSize(),ImVec2(cx+lx,cy+ly),color,text);
}

void drawCameraRotationGizmo() {
  Camera *c = &cameras[currentCamera];
  ImVec2 k = ImGui::GetWindowSize();
  int o = 5;
  float rad = 40;
  Quaternion rot = normalize(inverse(c->rotation));
  Vector p[3];
  p[0] = rot.rotateVector(Vector(1,0,0));
  p[1] = rot.rotateVector(Vector(0,1,0));
  p[2] = rot.rotateVector(Vector(0,0,1));
  int d[3] = {0,1,2}; for (int i = 0; i < 3*2; i++) if (p[d[i % 3]].z < p[d[(i+1)%3]].z) {int t = d[i%3]; d[i % 3]=d[(i+1)%3];d[(i+1)%3]=t;}
  for (int j = 0; j < 3; j++) {
    if (d[j] == 0) viewportLine(k.x-rad-o,rad+o,p[0].x*rad,-p[0].y*rad,0xff0000ff,"X");
    if (d[j] == 1) viewportLine(k.x-rad-o,rad+o,p[1].x*rad,-p[1].y*rad,0xff00ff00,"Y");
    if (d[j] == 2) viewportLine(k.x-rad-o,rad+o,p[2].x*rad,-p[2].y*rad,0xffff0000,"Z");
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
      o2->selected = false;
      gameObjects.insert(o2,i2+1);
      i2++;
    }
  }
  renderingUpdated = true;
}

void drawViewport() {

  ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(0,0,0,0));

  ImGui::SetNextWindowPos(ImVec2(outliner_width,menuBar_height+toolBar_height));
  ImGui::Begin("VIEWPORT", NULL, ImVec2(main_window_xres-outliner_width-properties_width,main_window_yres-menuBar_height-toolBar_height-timeLine_height-info_height),1.0,FROZENWINDOW);
  ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.25,0.25,0.25,1));
  ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.5,0.5,0.5,1));
  bool one = false; for (int i2 = 0; i2 < gameObjects.size(); i2++) if (gameObjects[i2]->selected && gameObjects[i2]->type != GAMEOBJECT_SECTION) {one=true; break;}
  if (one && mode != MODE_UNDEFINED && mode != MODE_SELECT) {
    ImGui::Checkbox("Tool##VIEWPORTTOOLON",&toolOn);
    if (toolOn) {
      ImGui::PushItemWidth(15*7);
      switch(mode) {
      case MODE_MOVE: {
        if (ImGui::Button("Clear##VIEWPORTMOVECLEAR")) {move_x = move_y = move_z = 0; move_negate = false;}
        ImGui::InputFloat("MoveX##VIEWPORTMOVEX",&move_x,10,10);
        ImGui::InputFloat("MoveY##VIEWPORTMOVEY",&move_y,10,10);
        ImGui::InputFloat("MoveZ##VIEWPORTMOVEZ",&move_z,10,10);
        if (ImGui::Button("Apply##VIEWPORTMOVEAPPLY")) {
          moveBy(move_x,move_y,move_z, move_negate);
        }
        ImGui::SameLine();
        if (ImGui::Button("Duplicate")) {
          duplicate();
          moveBy(move_x,move_y,move_z, move_negate);
        }
        ImGui::SameLine(); ImGui::Checkbox("Inverse##VIEWPORTMOVENEGATE", &move_negate);
      } break;
      case MODE_SCALE: {
        if (ImGui::Button("Clear##VIEWPORTSCALECLEAR")) {scale_x = scale_y = scale_z = 1; scale_negate = false;}
        ImGui::InputFloat("ScaleX##VIEWPORTSCALEX",&scale_x); if (scale_x < minScale) scale_x = minScale;
        ImGui::InputFloat("ScaleY##VIEWPORTSCALEY",&scale_y); if (scale_y < minScale) scale_y = minScale;
        ImGui::InputFloat("ScaleZ##VIEWPORTSCALEZ",&scale_z); if (scale_z < minScale) scale_z = minScale;
        if (ImGui::Button("Apply##VIEWPORTSCALEAPPLY")) {
          scaleBy(scale_x,scale_y,scale_z, scale_negate);
        }
        ImGui::SameLine(); ImGui::Checkbox("Inverse##VIEWPORTSCALENEGATE", &scale_negate);
      } break;
      case MODE_ROTATE: {
        if (ImGui::Button("Clear##VIEWPORTROTCLEAR")) {rotate_x = rotate_y = rotate_z = 0; rotate_negate = false;}
        ImGui::InputFloat("RotA##VIEWPORTROTA",&rotate_x,45,45);
        ImGui::InputFloat("RotB##VIEWPORTROTB",&rotate_y,45,45);
        ImGui::InputFloat("RotC##VIEWPORTROTC",&rotate_z,45,45);
        if (ImGui::Button("Apply##VIEWPORTROTAPPLY")) {
          rotateBy(rotate_x,rotate_y,rotate_z,rotate_negate);
        }
        ImGui::SameLine(); ImGui::Checkbox("Inverse##VIEWPORTROTATENEGATE", &rotate_negate);
      } break;
      }
      ImGui::PopItemWidth();
    }
  }
  ImGui::Checkbox("Camera##VIEWPORTOVERLAYON",&viewportOn);
  if (viewportOn) {
    drawCameraRotationGizmo();
    ImGui::BeginChild("VIEWPORTSETTING");
    Camera *c = &cameras[currentCamera];
    float k;
    ImGui::PushItemWidth(15*7);
    k = c->position.x;if (ImGui::InputFloat("CamX##VIEWPORTCAMX",&k,10,10)) {renderingUpdated = true;} c->position.x = k;
    k = c->position.y;if (ImGui::InputFloat("CamY##VIEWPORTCAMY",&k,10,10)) {renderingUpdated = true;} c->position.y = k;
    k = c->position.z;if (ImGui::InputFloat("CamZ##VIEWPORTCAMZ",&k,10,10)) {renderingUpdated = true;} c->position.z = k;
    Vector yawPitchRoll = c->rotation.getYawPitchRoll();
    k = yawPitchRoll.x*180.0/PI;if (ImGui::InputFloat("RotA##VIEWPORTYAW",&k,10,10)) {renderingUpdated = true; c->rotation.yawPitchRoll(k*PI/180.0,yawPitchRoll.y,yawPitchRoll.z);}
    k = yawPitchRoll.y*180.0/PI;if (ImGui::InputFloat("RotB##VIEWPORTPITCH",&k,10,10)) {renderingUpdated = true; c->rotation.yawPitchRoll(yawPitchRoll.x,k*PI/180,yawPitchRoll.z);}
    k = yawPitchRoll.z*180.0/PI;if (ImGui::InputFloat("RotC##VIEWPORTROLL",&k,10,10)) {renderingUpdated = true; c->rotation.yawPitchRoll(yawPitchRoll.x,yawPitchRoll.y,k*PI/180.0);}
    if (!c->ortho) {
      if (ImGui::InputFloat("FOV##VIEWPORTFOV",&c->fov,10,10)) {renderingUpdated = true;} 
    } else {
      if (ImGui::InputFloat("Ortho scale##VIEWPORTORTHOSCALE",&c->orthoScale,10,10)) {renderingUpdated = true;} 
    }
    if (c->orthoScale < 0) c->orthoScale = 0;
    if (c->fov < 1) c->fov = 1; if (c->fov>180) c->fov = 180;
    if (ImGui::InputFloat("Aspect ratio##VIEWPORTASPECTRATIO",&c->aspect,0.1,0.1)) {renderingUpdated = true;} 
    ImGui::PopItemWidth();
    ImGui::PushItemWidth(9*7);
    if (ImGui::InputFloat("Near plane##VIEWPORTNEAR",&c->nearPlane)) {renderingUpdated = true;} if (c->nearPlane < 0.00001) c->nearPlane = 0.00001;
    if (ImGui::InputFloat("Far plane##VIEWPORTFAR",&c->farPlane)) {renderingUpdated = true;} if (c->farPlane < c->nearPlane+0.00001) c->farPlane = c->nearPlane + 0.00001;
    if (ImGui::Checkbox("Use grid",&USEGRID)) {renderingUpdated = true;}
    if (USEGRID) {
      k = XGRIDSIZE; if (ImGui::InputFloat("Grid size##VIEWPORTGRIDSIZE",&k)) {XGRIDSIZE=ZGRIDSIZE=k;renderingUpdated = true;}
      k = GRIDSTEPS; if (ImGui::InputFloat("Grid steps##VIEWPORTGRIDSTEPS",&k)) {GRIDSTEPS=k;renderingUpdated = true;}
      colorItem("Grid","##VIEWPORTBACKGRIDCOLOR", &gridColor, true);
    }
    colorItem("Background","##VIEWPORTBACKGROUND", &clearColor, true);
    ImGui::PopItemWidth();
    ImGui::EndChild();
  }
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  ImGui::End();

  ImGui::PopStyleColor();
}

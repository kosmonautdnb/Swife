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
#include "MENUBAR.HPP"
#include "CONFIG.HPP"
#include "GL.H"
#include "IMGUIGL.HPP"
#include "STL/DOS.HPP"
#include "DIALOGS.HPP"
#include "GAMEOBJ.HPP"
#include "VIEWPORT.HPP"
#include "RENDER.HPP"
#include "SVG.HPP"

int menuBar_height = 17;

static String hex(int v) {
  const char *h="0123456789ABCDEF";
  String r;
  r.resize(2);
  r[0] = h[v / 16];
  r[1] = h[v & 15];
  return r;
}

static String timeDescString() {
  int rtc_seconds = rtcRead(0);
  int rtc_minutes = rtcRead(2);
  int rtc_hours   = rtcRead(4);
  int rtc_weekday = rtcRead(6);
  int rtc_day     = rtcRead(7);
  int rtc_month   = rtcRead(8);
  int rtc_year    = rtcRead(9);
  const char *day[8] ={"Mon","Tue","Wed","Thu","Fri","Sat","Sun","Mon"};
  return hex(rtc_day)+"."+hex(rtc_month)+"."+hex(rtc_year)+" "+String(day[(rtc_weekday-2)&7])+" "+hex(rtc_hours)+":"+hex(rtc_minutes);
}

void drawMenuBar() {
  bool on = false;
  bool one = false; for (int i2 = 0; i2 < gameObjects.size(); i2++) if (gameObjects[i2]->selected) {one=true; break;}
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10,1));
  ImGui::PushStyleColor(ImGuiCol_MenuBarBg,ImVec4(0.f,0.f,0.f,1.f));
  ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1.f,1.f,1.f,1.f));
  ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(0.f,0.f,0.f,1.f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered,ImVec4(0.1,0.1,0.1,1));
  if (ImGui::BeginMainMenuBar()) {
    SVG *svg = &svgs["appicon"];
    ImGui::Image((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h));
    ImGui::SameLine();
    
    /*ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.6,0.6,0.6,1));
    ImGui::Text(appName); 
    ImGui::PopStyleColor();
    ImGui::SameLine();*/

    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Load scene")) {on = true;  dialogLoadSave(true,DIALOG_LOADSCENE);}
      if (ImGui::MenuItem("Save scene")) {on = true;  dialogLoadSave(true,DIALOG_SAVESCENE);}
      ImGui::Separator();
      if (ImGui::MenuItem("Quit")) {
        dialogQuit(true);
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
      if (ImGui::MenuItem("Perspective")) {on = true; Camera *c = &cameras[currentCamera]; c->ortho = false; renderingUpdated = true;}
      if (ImGui::MenuItem("Ortho left")) {on = true; setOrthoLookTo(1,0,0);}
      if (ImGui::MenuItem("Ortho right")) {on = true; setOrthoLookTo(-1,0,0);}
      if (ImGui::MenuItem("Ortho top")) {on = true; setOrthoLookTo(0,-1,0);}
      if (ImGui::MenuItem("Ortho bottom")) {on = true; setOrthoLookTo(0,1,0);}
      if (ImGui::MenuItem("Ortho front")) {on = true; setOrthoLookTo(0,0,-1);}
      if (ImGui::MenuItem("Ortho back")) {on = true; setOrthoLookTo(0,0,1);}
      ImGui::Separator();
      bool selected = false;
      selected = currentCamera == CAMERA_USER; if (ImGui::MenuItem("User camera","",&selected)) {on = true; currentCamera = CAMERA_USER; renderingUpdated = true;}
      selected = currentCamera == CAMERA_CAMERA1; if (ImGui::MenuItem("Camera 1","",&selected)) {on = true; currentCamera = CAMERA_CAMERA1; renderingUpdated = true;}
      selected = currentCamera == CAMERA_CAMERA2; if (ImGui::MenuItem("Camera 2","",&selected)) {on = true; currentCamera = CAMERA_CAMERA2; renderingUpdated = true;}
      ImGui::Separator();
      selected = lightOn; if (ImGui::MenuItem("Lighting","",&selected)) {on = true; lightOn = !lightOn; renderingUpdated = true;}
      selected = texturedOn; if (ImGui::MenuItem("Textures","",&selected)) {on = true; texturedOn = !texturedOn; renderingUpdated = true;}
      selected = twoSidedOn; if (ImGui::MenuItem("Two sided","",&selected)) {on = true; twoSidedOn = !twoSidedOn; renderingUpdated = true;}
      ImGui::Separator();
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Object")) {
      if (ImGui::MenuItem("Add mesh")) {on = true; dialogAddObject(true,GAMEOBJECT_MESH);}
      if (ImGui::MenuItem("Add camera")) {on = true; dialogAddObject(true,GAMEOBJECT_CAMERA);}
      if (ImGui::MenuItem("Add skybox")) {on = true; dialogAddObject(true,GAMEOBJECT_SKYBOX);}
      if (ImGui::MenuItem("Add path")) {on = true; dialogAddObject(true,GAMEOBJECT_PATH);}
      if (ImGui::MenuItem("Add billboard")) {on = true; dialogAddObject(true,GAMEOBJECT_BILLBOARD);}
      if (ImGui::MenuItem("Add trigger")) {on = true; dialogAddObject(true,GAMEOBJECT_TRIGGER);}
      ImGui::Separator();
      if (ImGui::MenuItem("Add link")) {on = true; dialogAddObject(true,GAMEOBJECT_LINK);}
      if (ImGui::MenuItem("Add section")) {on = true; dialogAddObject(true,GAMEOBJECT_SECTION);}
      if (one) {
        ImGui::Separator();
        if (ImGui::MenuItem("Duplicate")) {on = true; duplicate();}
        if (ImGui::MenuItem("Delete")) {on = true; dialogDeleteGameObjects(true);}
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Select by property")) {on = true; dialogSelectByProperty(true);}
      if (ImGui::MenuItem("Select by layer")) {on = true; dialogSelectByLayer(true);}
      if (ImGui::MenuItem("Select all invisible")) {on = true; dialogSelectInvisible(true);}
      if (one) {
        ImGui::Separator();
        if (ImGui::MenuItem("View object textures")) {on = true; dialogObjectTextures(true);}
        if (ImGui::MenuItem("Center selection to view")) {on = true; centerSelection();}
      }
      ImGui::Separator();
      if (ImGui::MenuItem("View bindings")) {on = true;  dialogViewBindings(true);}
      if (ImGui::MenuItem("Grid settings")) {on = true;  dialogGridSettings(true);}
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
      if (ImGui::MenuItem("About")) {on = true; dialogAbout(true);}
      if (ImGui::MenuItem("Controls")) {on = true; dialogControls(true);}
      ImGui::EndMenu();
    }

    ImGui::SameLine();
    String timeDesc = timeDescString();
    ImVec2 cursorScreen = ImGui::GetCursorScreenPos();
    cursorScreen.x = main_window_xres - timeDesc.length()*7;
    ImGui::SetCursorScreenPos(cursorScreen);
    ImGui::Text("%s",timeDesc.c_str());
    ImGui::EndMainMenuBar();
  }
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
  if (on) {
    disableRenderMouseClicks();
  }
}

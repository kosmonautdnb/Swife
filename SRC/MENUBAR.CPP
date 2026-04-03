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
#include "TOOLBAR.HPP"
#include "RENDER.HPP"
#include "HITTEST.HPP"
#include "SVG.HPP"
#include "PAINT.HPP"
#include "PROPS.HPP"
#include "OUTLINER.HPP"

int menuBar_height = 17;
bool screenSaverOn = true;

String hex2_(int v) {
  const char *h="0123456789ABCDEF";
  String r;
  r.resize(2);
  r[0] = h[v / 16];
  r[1] = h[v & 15];
  return r;
}

String timeDescString() {
  int rtc_seconds = rtcRead(0);
  int rtc_minutes = rtcRead(2);
  int rtc_hours   = rtcRead(4);
  int rtc_weekday = rtcRead(6);
  int rtc_day     = rtcRead(7);
  int rtc_month   = rtcRead(8);
  int rtc_year    = rtcRead(9);
  const char *day[8] ={"Mon","Tue","Wed","Thu","Fri","Sat","Sun","Mon"};
  return hex2_(rtc_day)+"."+hex2_(rtc_month)+"."+hex2_(rtc_year)+" "+String(day[(rtc_weekday-2)&7])+" "+hex2_(rtc_hours)+":"+hex2_(rtc_minutes);
}

bool showToolTips = true;

void drawToolTip(const char *fmt, ...) {
  if (!showToolTips) return;
  if (ImGui::IsItemHovered()) {
    ImGui::BeginTooltip();
    ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.0,0.0,0.0,1));
    va_list args;
    va_start(args, fmt);
    ImGui::TextV(fmt, args);
    va_end(args);
    ImGui::PopStyleColor();
    ImGui::EndTooltip();
  }
}

void drawToolTipForced(const char *fmt, ...) {
  if (ImGui::IsItemHovered()) {
    ImGui::BeginTooltip();
    ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.0,0.0,0.0,1));
    va_list args;
    va_start(args, fmt);
    ImGui::TextV(fmt, args);
    va_end(args);
    ImGui::PopStyleColor();
    ImGui::EndTooltip();
  }
}

void drawMenuBar() {
  bool on = false;
  GameObject *first = NULL;
  int count = 0;
  bool one = false; for (int i2 = 0; i2 < gameObjects.size(); i2++) if (gameObjects[i2]->selected) {first = gameObjects[i2]; one=true; count++;}
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10,1));
  ImGui::PushStyleColor(ImGuiCol_MenuBarBg,ImVec4(0.f,0.f,0.f,1.f));
  ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1.f,1.f,1.f,1.f));
  ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(0.f,0.f,0.f,1.f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered,ImVec4(0.1,0.1,0.1,1));
  if (ImGui::BeginMainMenuBar()) {
    SVG *svg = &svgs["appicon"];
    ImGui::Image((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h));
    ImGui::SameLine();
    drawToolTip("Swife v%d.%02d",version/100,version%100);

    if (ImGui::BeginMenu("File")) {
      on = true;
      if (ImGui::MenuItem("New scene")) {dialogNew(true);}
      ImGui::Separator();
      if (ImGui::MenuItem("Load scene")) {dialogLoadSave(true,DIALOG_LOADSCENE);}
      if (ImGui::MenuItem("Save scene")) {dialogLoadSave(true,DIALOG_SAVESCENE);}
      ImGui::Separator();
      if (ImGui::MenuItem("Include selection")) {dialogLoadSave(true,DIALOG_LOADANDINSERTGAMEOBJECTS);}
      if (ImGui::MenuItem("Save selection")) {dialogLoadSave(true,DIALOG_SAVESELECTEDGAMEOBJECTS);}
      ImGui::Separator();
      if (ImGui::MenuItem("Export simple object CSV")) {dialogLoadSave(true,DIALOG_EXPORTOBJECTCSV);}
      ImGui::Separator();
      if (ImGui::MenuItem("Quit")) {
        dialogQuit(true);
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
      on = true;
      if (ImGui::MenuItem("Compact and relocate",NULL,false,one)) {dialogCompactAndMove(true);}
      if (ImGui::MenuItem("Bulk rename",NULL,false,one)) {dialogBulkRename(true);}
      ImGui::Separator();
      bool selected = false;
      selected = moveToolWorldSpace; if (ImGui::MenuItem("Movetool is in worldspace","",&selected)) {moveToolWorldSpace = !moveToolWorldSpace; renderingUpdated = true;}
      ImGui::Separator();
      selected = placeCursor; if (ImGui::MenuItem("Place cursor","",&selected, mode == MODE_SELECT)) {placeCursor = !placeCursor; renderingUpdated = true;}
      if (ImGui::MenuItem("Cursor to object","",false, count==1)) {cursorToObject(first); renderingUpdated = true;}
      if (ImGui::MenuItem("Put to cursor","",false, count==1)) {putToCursor(first); renderingUpdated = true;}
      ImGui::Separator();
      if (ImGui::MenuItem("Copy transform","",false, count==1)) {copyTransform();}
      selected = pasteAnchor; if (ImGui::MenuItem("Should paste anchor point","",&selected)) {pasteAnchor = !pasteAnchor;}
      selected = pastePosition; if (ImGui::MenuItem("Should paste position","",&selected)) {pastePosition = !pastePosition;}
      selected = pasteScaling; if (ImGui::MenuItem("Should paste scaling","",&selected)) {pasteScaling = !pasteScaling;}
      selected = pasteRotation; if (ImGui::MenuItem("Should paste rotation","",&selected)) {pasteRotation = !pasteRotation;}
      if (ImGui::MenuItem("Paste transform","",false, one)) {pasteTransform(); renderingUpdated = true;}
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
      on = true;
      if (ImGui::MenuItem("Perspective")) {Camera *c = &cameras[currentCamera]; c->ortho = false; renderingUpdated = true;}
      if (ImGui::MenuItem("Ortho left")) {setOrthoLookTo(1,0,0);}
      if (ImGui::MenuItem("Ortho right")) {setOrthoLookTo(-1,0,0);}
      if (ImGui::MenuItem("Ortho top")) {setOrthoLookTo(0,-1,0);}
      if (ImGui::MenuItem("Ortho bottom")) {setOrthoLookTo(0,1,0);}
      if (ImGui::MenuItem("Ortho front")) {setOrthoLookTo(0,0,-1);}
      if (ImGui::MenuItem("Ortho back")) {setOrthoLookTo(0,0,1);}
      ImGui::Separator();
      bool selected = false;
      selected = currentCamera == CAMERA_USER; if (ImGui::MenuItem("User camera","",&selected)) {currentCamera = CAMERA_USER; renderingUpdated = true;}
      selected = currentCamera == CAMERA_CAMERA1; if (ImGui::MenuItem("Camera 1","",&selected)) {currentCamera = CAMERA_CAMERA1; renderingUpdated = true;}
      selected = currentCamera == CAMERA_CAMERA2; if (ImGui::MenuItem("Camera 2","",&selected)) {currentCamera = CAMERA_CAMERA2; renderingUpdated = true;}
      ImGui::Separator();
      selected = lightOn; if (ImGui::MenuItem("Lighting","",&selected)) {lightOn = !lightOn; renderingUpdated = true;}
      selected = texturedOn; if (ImGui::MenuItem("Textures","",&selected)) {texturedOn = !texturedOn; renderingUpdated = true;}
      selected = twoSidedOn; if (ImGui::MenuItem("Two sided","",&selected)) {twoSidedOn = !twoSidedOn; renderingUpdated = true;}
      selected = shadowsOn; if (ImGui::MenuItem("Shadows","",&selected)) {shadowsOn = !shadowsOn; renderingUpdated = true;}
      ImGui::Separator();
      selected = showLinks; if (ImGui::MenuItem("Show links (properties)","",&selected)) {showLinks = !showLinks; renderingUpdated = true;}
      selected = showGrids; if (ImGui::MenuItem("Show grids","",&selected)) {showGrids = !showGrids; renderingUpdated = true;}
      selected = showOverlays; if (ImGui::MenuItem("Show overlays","",&selected)) {showOverlays = !showOverlays; renderingUpdated = true;}
      selected = drawObjectNames; if (ImGui::MenuItem("Show names","",&selected,showOverlays)) {drawObjectNames = !drawObjectNames; renderingUpdated = true;}
      selected = drawSelectionDesc; if (ImGui::MenuItem("Show object description","",&selected,showOverlays)) {drawSelectionDesc = !drawSelectionDesc; renderingUpdated = true;}
      ImGui::Separator();
      selected = selectionOutlineOn; if (ImGui::MenuItem("Outline around selection","",&selected)) {selectionOutlineOn = !selectionOutlineOn;}
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Object")) {
      on = true;
      if (ImGui::MenuItem("Add mesh")) {dialogAddObject(true,GAMEOBJECT_MESH);}
      if (ImGui::MenuItem("Add camera")) {dialogAddObject(true,GAMEOBJECT_CAMERA);}
      if (ImGui::MenuItem("Add skybox")) {dialogAddObject(true,GAMEOBJECT_SKYBOX);}
      if (ImGui::MenuItem("Add billboard")) {dialogAddObject(true,GAMEOBJECT_BILLBOARD);}
      if (ImGui::MenuItem("Add trigger")) {dialogAddObject(true,GAMEOBJECT_TRIGGER);}
      if (ImGui::MenuItem("Add point")) {dialogAddObject(true,GAMEOBJECT_POINT);}
      if (ImGui::MenuItem("Add path")) {dialogAddObject(true,GAMEOBJECT_PATH);}
      if (ImGui::MenuItem("Add texture grid")) {dialogAddObject(true,GAMEOBJECT_TEXTUREGRID);}
      if (ImGui::MenuItem("Add heightmap")) {dialogAddObject(true,GAMEOBJECT_HEIGHTMAP);}
      if (ImGui::MenuItem("Add valley")) {dialogAddObject(true,GAMEOBJECT_VALLEY);}
      ImGui::Separator();
      if (ImGui::MenuItem("Add image")) {dialogAddObject(true,GAMEOBJECT_IMAGE);}
      if (ImGui::MenuItem("Add script")) {dialogAddObject(true,GAMEOBJECT_SCRIPT);}
      ImGui::Separator();
      if (ImGui::MenuItem("Add link")) {dialogAddObject(true,GAMEOBJECT_LINK);}
      if (ImGui::MenuItem("Add section")) {dialogAddObject(true,GAMEOBJECT_SECTION);}
      ImGui::Separator();
      if (ImGui::MenuItem("Duplicate",NULL,false,one)) {duplicate();}
      if (ImGui::MenuItem("Delete",NULL,false,one)) {dialogDeleteGameObjects(true);}
      if (ImGui::MenuItem("Move center",NULL,false,one)) {dialogMoveByCenter(true);}
      ImGui::Separator();
      if (ImGui::MenuItem("Select by property")) {dialogSelectByProperty(true);}
      if (ImGui::MenuItem("Select by layer")) {dialogSelectByLayer(true);}
      if (ImGui::MenuItem("Select all invisible")) {dialogSelectInvisible(true);}
      ImGui::Separator();
      if (ImGui::MenuItem("View object textures",NULL,false,one)) {dialogObjectTextures(true);}
      if (ImGui::MenuItem("Center selection to view",NULL,false,one)) {centerSelection();}
      ImGui::Separator();
      if (ImGui::MenuItem("View bindings")) {dialogViewBindings(true);}
      if (ImGui::MenuItem("Grid settings")) {dialogGridSettings(true);}
      ImGui::Separator();
      if (ImGui::MenuItem("Unlink links","",false, one)) {unlinkLinks();}
      ImGui::Separator();
      if (ImGui::MenuItem("Reload all")) {reloadAll();}
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
      on = true;
      if (ImGui::MenuItem("About")) {dialogAbout(true);}
      if (ImGui::MenuItem("Controls")) {dialogControls(true);}
      ImGui::Separator();
      if (ImGui::MenuItem("Troubleshooting")) {dialogTroubleShooting(true);}
      ImGui::Separator();
      bool selected = false;
      selected = screenSaverOn; if (ImGui::MenuItem("Screen saver","",&selected)) {screenSaverOn = !screenSaverOn;}
      selected = showToolTips; if (ImGui::MenuItem("Show tooltips","",&selected)) {showToolTips = !showToolTips;}
      selected = showObjectDates; if (ImGui::MenuItem("List creation dates","",&selected)) {showObjectDates = !showObjectDates;}
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

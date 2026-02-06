#include "MENUBAR.HPP"
#include "CONFIG.HPP"
#include "GL.H"
#include "IMGUIGL.HPP"
#include "STL/DOS.HPP"
#include "DIALOGS.HPP"
#include "GAMEOBJ.HPP"
#include "HELP.HPP"
#include "VIEWPORT.HPP"
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
      if (ImGui::MenuItem("Quit")) {
        dialogQuit(true);
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Object")) {
      if (ImGui::MenuItem("Add mesh")) dialogAddObject(true,GAMEOBJECT_MESH);
      if (ImGui::MenuItem("Add camera")) dialogAddObject(true,GAMEOBJECT_CAMERA);
      if (ImGui::MenuItem("Add skybox")) dialogAddObject(true,GAMEOBJECT_SKYBOX);
      if (ImGui::MenuItem("Add path")) dialogAddObject(true,GAMEOBJECT_PATH);
      if (ImGui::MenuItem("Add billboard")) dialogAddObject(true,GAMEOBJECT_BILLBOARD);
        ImGui::Separator();
      if (ImGui::MenuItem("Add section")) dialogAddObject(true,GAMEOBJECT_SECTION);
      if (one) {
        ImGui::Separator();
        if (ImGui::MenuItem("Duplicate")) duplicate();
        if (ImGui::MenuItem("Delete")) dialogDeleteGameObjects(true);
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Select by property")) dialogSelectByProperty(true);
      if (ImGui::MenuItem("Select by layer")) dialogSelectByLayer(true);
      if (ImGui::MenuItem("Select all invisible")) dialogSelectInvisible(true);
      if (one) {
        ImGui::Separator();
        if (ImGui::MenuItem("View object textures")) dialogObjectTextures(true);
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
      if (ImGui::MenuItem("About")) dialogAbout(true);
      if (ImGui::MenuItem("Controls")) dialogControls(true);
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
}

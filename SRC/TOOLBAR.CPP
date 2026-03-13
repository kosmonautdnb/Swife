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
#include "TOOLBAR.HPP"
#include "CONFIG.HPP"
#include "OUTLINER.HPP"
#include "MENUBAR.HPP"
#include "PROPS.HPP"
#include "SVG.HPP"
#include "RENDER.HPP"
#include "VIEWPORT.HPP"
#include "PAINT.HPP"
#include "GAMEOBJ.HPP"
#include "DIALOGS.HPP"
#include "TEXPAINT.HPP"
#include "STL/ARRAY.HPP"
#include <math.h>

int mode = MODE_UNDEFINED;

int toolBar_height = 32; // below 32 seems to not work with this version of ImGui
float cameraSpeed = 1;
unsigned int currentSelectedAltuniqueId = 0;

void drawSeparator(int x, int y, int h) {
  ImVec2 cursor = ImGui::GetCursorScreenPos();
  cursor.x += 3;
  ImGui::GetWindowDrawList()->AddLine(ImVec2(cursor.x+x,cursor.y+y),ImVec2(cursor.x+x,cursor.y+y+h),0xffa0a0a0);
  ImGui::Text(" ");
}

class ToolBarButton {
public:
  ToolBarButton() {this->svgId[0] = NULL; toggleState = false; selected = false; mode = MODE_UNDEFINED; toolTip = NULL;}
  ToolBarButton(const char *svgId0, const char *svgId1 = NULL) {this->svgId[0] = svgId0;this->svgId[1] = svgId1; toggleState = false; selected = false; mode = MODE_UNDEFINED; toolTip = NULL;}
  int mode;
  bool selected;
  const char *svgId[2];
  const char *toolTip;
  bool toggleState;
  bool isSingle() {
    return (svgId[1] == NULL) && (svgId[0] != NULL);
  }
  bool paint(const char *id) {
    if (svgId[0]==NULL) {
      drawSeparator(0,0,23);
      return false;
    }
    SVG *svg = &svgs[svgId[toggleState ? 1 : 0]];
    if (selected)
      ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.4,0.4,0.4,1));
    else
      ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(1,1,1,1));
    ImGui::PushID(id);
    bool ret = false;
    if (ImGui::ImageButton((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h))) {
      if (this->svgId[1]!=NULL) toggleState = !toggleState;
      toolOn = true; // a little bit displaced here
      selectionCounter = 0; // also a little bit displaced here
      renderingUpdated = true;
      ret = true;
    } else if (toolTip != NULL) drawToolTip(toolTip);
    ImGui::PopID();
    ImGui::PopStyleColor();
    return ret;
  }
};

Array<ToolBarButton> toolBarButtons;

#define TOOLBARTOGGLEBUTTON(__val0__,__val1__) toolBarButtons.push_back(ToolBarButton(__val0__,__val1__));
#define TOOLBARSELECTBUTTON(__val0__,__mode__,__toolTip__) {toolBarButtons.push_back(ToolBarButton(__val0__));toolBarButtons.back().mode = __mode__;toolBarButtons.back().toolTip = __toolTip__;}
#define TOOLBARPLACEHOLDER() toolBarButtons.push_back(ToolBarButton());

void initToolBar() {
  TOOLBARTOGGLEBUTTON("play","pause");
  TOOLBARPLACEHOLDER();
  TOOLBARSELECTBUTTON("select",MODE_SELECT,"Select/Place");
  TOOLBARSELECTBUTTON("move",MODE_MOVE,"Move");
  TOOLBARSELECTBUTTON("scale",MODE_SCALE,"Scale");
  TOOLBARSELECTBUTTON("rotate",MODE_ROTATE,"Rotate");
  TOOLBARPLACEHOLDER();
}

unsigned int toolBarTextButton(unsigned int wasSet, unsigned int willBeSet, const char *name) {
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5,0));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);

  ImGui::PushStyleColor(ImGuiCol_Button,wasSet == willBeSet ? color_button_on_bg:color_button_off_bg);
  ImGui::PushStyleColor(ImGuiCol_Text,wasSet == willBeSet ? color_button_on_text:color_button_off_text);
  if (ImGui::Button(name)) {wasSet = willBeSet; renderingUpdated=true;}
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();

  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
  return wasSet;
}

bool toolBarTextButton2(const char *name, bool enabled) {
  bool ret = false;
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5,0));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);

  ImGui::PushStyleColor(ImGuiCol_Button,enabled ? color_button_on_bg : color_button_off_bg);
  ImGui::PushStyleColor(ImGuiCol_Text,enabled ? color_button_on_text : color_button_off_text);
  if (ImGui::Button(name)) {ret = true; renderingUpdated=true;}
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();

  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
  return ret;
}

void drawToolBar() {
  int count = 0; GameObject *first = NULL; for (int ic = 0; ic < gameObjects.size(); ic++) {if (gameObjects[ic]->selected) {first = gameObjects[ic]; count++;}}
  ImGui::SetNextWindowPos(ImVec2(outliner_width,menuBar_height));
  ImGui::Begin("TOOLBAR", NULL, ImVec2(main_window_xres-outliner_width-properties_width,toolBar_height),1.0,FROZENWINDOW);
  if (count == 1 && first->type == GAMEOBJECT_IMAGE) {
    FileBinding *binding = getBindingById(first->fileBindingId);
    if (binding != NULL && toolBarTextButton2("Reload##DRAWTOOLBARRELOADIMAGE", binding->updated)) {
      binding->reload();
    }
    ImGui::SameLine();
    float imageZoomBefore = imageZoom;
    if (imageZoomBefore<0) imageZoomBefore=0;
    ImGui::PushItemWidth(64);
    if (ImGui::SliderFloat("##DRAWTOOLBARIMAGEZOOM",&imageZoom,0,10.0)) {
      if (imageZoom<0) imageZoom=0;
      double zoomBefore = pow(2.0,imageZoomBefore)-1.0;
      double zoom = pow(2.0,imageZoom)-1.0;
      if (zoomBefore<0.0001) zoomBefore = 0.0001;
      Vector oldPos = imageScroll;
      Vector newPos = imageScroll * zoom / zoomBefore;
      imageScroll += (newPos-oldPos);
    }
    drawToolTip("Zoom");
    ImGui::PopItemWidth();
    if (imageZoom<0) imageZoom=0;
    if (imageZoom>10.0) imageZoom=10.0;  
    ImGui::SameLine();
    if (toolBarTextButton2(paintBlended?"Solid##DRAWTOOLBARBLENDING":"Blending##DRAWTOOLBARBLENDING",true)) {paintBlended = !paintBlended;} else drawToolTip("Stamp is solid or with transparency");
    ImGui::SameLine();
    ImGui::Checkbox("##DRAWTOOLBARSTAMPON",&stampOn);
    drawToolTip(stampOn?"Disable stamp":"Enable stamp");
    ImGui::SameLine();
    ImGui::Checkbox("##DRAWTOOLBARALPHAON",&alphaCheckered);
    drawToolTip(alphaCheckered?"Alpha black":"Alpha fading");
    ImGui::SameLine();
    ImGui::Text("f=fill a=fxaa"); drawToolTip("Press <f> to fill or <a> for antialiasing at mouse position");
    ImGui::SameLine();
    if (binding != NULL && toolBarTextButton2("Save image##DRAWTOOLBARSAVEIMAGE",binding->updated)) {
      paintingSaveOn = true;
    }
    ImGui::SameLine();
    if (imageMouse.z>=0) {ImGui::Text("%d %d",(int)floor(imageMouse.z),(int)floor(imageMouse.w));ImGui::SameLine();}
    else if (imageMouse.x>=0) {ImGui::Text("%d %d",(int)floor(imageMouse.x),(int)floor(imageMouse.y));ImGui::SameLine();}
  } else {
    ImVec2 cursorScreen = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(ImVec2(cursorScreen.x, cursorScreen.y + 2));
    drawSeparator(-4,0,23);
    ImGui::SameLine();
    for (int i = 0; i < toolBarButtons.size(); i++) {
      if (i != 0) ImGui::SameLine();
      if (toolBarButtons[i].isSingle()) toolBarButtons[i].selected = mode == toolBarButtons[i].mode;
      if (toolBarButtons[i].paint(IDINDEXED("##DRAWTOOLBARBUTTONID",i))) {
        if (toolBarButtons[i].isSingle()) mode = toolBarButtons[i].mode;
      }
    }
    if (!isEga) ImGui::SameLine();
    if (currentCamera == CAMERA_USER) {
      Camera *c = &cameras[currentCamera];
      c->ortho = (bool)toolBarTextButton((unsigned int)c->ortho,(unsigned int)false,"Persp##ORTHO1"); drawToolTip("Perspective camera");
      ImGui::SameLine();
      c->ortho = (bool)toolBarTextButton((unsigned int)c->ortho,(unsigned int)true,"Ortho##ORTHO2"); drawToolTip("Orthographic camera (needs Ortho scale)");
      ImGui::SameLine();
      SVG *svg = &svgs["cameraspeed"];                        
    
      ImGui::Image((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h));
      ImGui::SameLine();
      ImGui::PushItemWidth(70);
      ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.9,0.9,0.9,1));
      ImGui::InputFloat("##CAMERASPEED",&cameraSpeed,cameraSpeed>1?1:0.1,1,2);
      drawToolTip("Camera speed");
      if (cameraSpeed < 0) cameraSpeed = 0;
      ImGui::PopStyleColor();
      ImGui::PopItemWidth();
      ImGui::SameLine();
      drawSeparator(0,0,23);
      ImGui::SameLine();
    } else {
      int found = -1;
      for (int i = 0; i < gameObjects.size(); i++) {
        if (gameObjects[i]->isCamera[currentCamera]) found = i;
      }
      if (found == -1)
        ImGui::Text("Camera %d",currentCamera);
      else
        ImGui::Text("Camera %d %d\"%s\"",currentCamera,found,gameObjects[found]->name);
      ImGui::SameLine();
      drawSeparator(0,0,23);
      ImGui::SameLine();
    }
    fullScreen = (bool)toolBarTextButton((unsigned int)fullScreen,(unsigned int)true,"View##EDITMODE1"); drawToolTip("Fullscreen view");
    ImGui::SameLine();
    fullScreen  = (bool)toolBarTextButton((unsigned int)fullScreen,(unsigned int)false,"Edit##EDITMODE2"); drawToolTip("Editor view");
    ImGui::SameLine();
    if (ImGui::Checkbox("##DRAWBOUNDINGBOXES",&justDrawBoundingBoxes)) {renderingUpdated = true;}
    drawToolTip(justDrawBoundingBoxes ? "Show normal":"Just show boundingboxes");
    ImGui::SameLine();
    drawSeparator(0,0,23);
  }
  ImGui::End();
}

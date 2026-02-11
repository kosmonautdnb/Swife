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
#include "STL/ARRAY.HPP"

int mode = MODE_UNDEFINED;

int toolBar_height = 32; // below 32 seems to not work with this version of ImGui
float cameraSpeed = 1;

void drawSeparator(int x, int y, int h) {
  ImVec2 cursor = ImGui::GetCursorScreenPos();
  cursor.x += 3;
  ImGui::GetWindowDrawList()->AddLine(ImVec2(cursor.x+x,cursor.y+y),ImVec2(cursor.x+x,cursor.y+y+h),0xffa0a0a0);
  ImGui::Text(" ");
}

class ToolBarButton {
public:
  ToolBarButton() {this->svgId[0] = NULL; toggleState = false; selected = false; mode = MODE_UNDEFINED;}
  ToolBarButton(const char *svgId0, const char *svgId1 = NULL) {this->svgId[0] = svgId0;this->svgId[1] = svgId1; toggleState = false; selected = false; mode = MODE_UNDEFINED;}
  int mode;
  bool selected;
  const char *svgId[2];
  bool toggleState;
  bool isSingle() {
    return (svgId[1] == NULL) && (svgId[0] != NULL);
  }
  bool paint(char *id) {
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
    }
    ImGui::PopID();
    ImGui::PopStyleColor();
    return ret;
  }
};

Array<ToolBarButton> toolBarButtons;

#define TOOLBARTOGGLEBUTTON(__val0__,__val1__) toolBarButtons.push_back(ToolBarButton(__val0__,__val1__));
#define TOOLBARSELECTBUTTON(__val0__,__mode__) {toolBarButtons.push_back(ToolBarButton(__val0__));toolBarButtons.back().mode = __mode__;}
#define TOOLBARPLACEHOLDER() toolBarButtons.push_back(ToolBarButton());

void initToolBar() {
  TOOLBARTOGGLEBUTTON("play","pause");
  TOOLBARPLACEHOLDER();
  TOOLBARSELECTBUTTON("select",MODE_SELECT);
  TOOLBARSELECTBUTTON("move",MODE_MOVE);
  TOOLBARSELECTBUTTON("scale",MODE_SCALE);
  TOOLBARSELECTBUTTON("rotate",MODE_ROTATE);
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

void drawToolBar() {
  ImGui::SetNextWindowPos(ImVec2(outliner_width,menuBar_height));
  ImGui::Begin("TOOLBAR", NULL, ImVec2(main_window_xres-outliner_width-properties_width,toolBar_height),1.0,FROZENWINDOW);
  ImVec2 cursorScreen = ImGui::GetCursorScreenPos();
  ImGui::SetCursorScreenPos(ImVec2(cursorScreen.x, cursorScreen.y + 2));
  drawSeparator(-4,0,23);
  ImGui::SameLine();
  for (int i = 0; i < toolBarButtons.size(); i++) {
    if (i != 0) ImGui::SameLine();
    if (toolBarButtons[i].paint((String("##DRAWTOOLBARBUTTONID")+String::fromInt(i)).c_str())) {
      if (toolBarButtons[i].isSingle()) {
        for (int j = 0; j < toolBarButtons.size(); j++) if (toolBarButtons[j].isSingle()) toolBarButtons[j].selected = false;
        toolBarButtons[i].selected = !toolBarButtons[i].selected;
        if (toolBarButtons[i].selected) mode = toolBarButtons[i].mode;
      }
    }
  }
  ImGui::SameLine();
  Camera *c = &cameras[currentCamera];
  c->ortho = (bool)toolBarTextButton((unsigned int)c->ortho,(unsigned int)false,"Persp##ORTHO1");
  ImGui::SameLine();
  c->ortho = (bool)toolBarTextButton((unsigned int)c->ortho,(unsigned int)true,"Ortho##ORTHO2");
  ImGui::SameLine();
  SVG *svg = &svgs["cameraspeed"];                        

  ImGui::Image((ImTextureID)svg->glHandle,ImVec2(svg->w,svg->h));
  ImGui::SameLine();
  ImGui::PushItemWidth(70);
  ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.9,0.9,0.9,1));
  ImGui::InputFloat("##CAMERASPEED",&cameraSpeed,cameraSpeed>1?1:0.1,1,2);
  if (cameraSpeed < 0) cameraSpeed = 0;
  ImGui::PopStyleColor();
  ImGui::PopItemWidth();
  ImGui::SameLine();
  drawSeparator(0,0,23);
  ImGui::SameLine();
  fullScreen = (bool)toolBarTextButton((unsigned int)fullScreen,(unsigned int)true,"View##EDITMODE1");
  ImGui::SameLine();
  fullScreen  = (bool)toolBarTextButton((unsigned int)fullScreen,(unsigned int)false,"Edit##EDITMODE2");
  ImGui::SameLine();
  drawSeparator(0,0,23);
  ImGui::End();
}

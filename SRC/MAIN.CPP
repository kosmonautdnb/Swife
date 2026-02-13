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
#include "UTIL\LOG.HPP"
#include "CONFIG.HPP"
#include "MENUBAR.HPP"
#include "OUTLINER.HPP"
#include "TOOLBAR.HPP"
#include "VIEWPORT.HPP"
#include "TIMELINE.HPP"
#include "PROPS.HPP"
#include "RENDER.HPP"
#include "GAMEOBJ.HPP"
#include "SVG.HPP"
#include "DIALOGS.HPP"
#include "KEYMTRIX.HPP"
#include "INFO.HPP"
#include "PAINT.HPP"
#include "DOS.HPP"
#include "SERIALZE.HPP"

double lastTime;

bool initVideo() {
  glWatcomPrecisionTimer(GL_TRUE);
  if (!glVesa(main_window_xres,main_window_yres,16)) {
    main_window_xres = 320;
    main_window_yres = 200;
    return glVGA();
  }
  glClearColor(0,0,0,1);
  glClearDepth(1.f);
  glClearStencil(0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
  glRefresh();

  InitImGui(glFrameBufferWidth,glFrameBufferHeight);
  ImGui::GetStyle().WindowRounding = 0.f;
  glFastTexturing = true;

  ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(1,1,1,1);
  ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(0,0,0,1);
  ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = ImVec4(0.5,0.5,1.0,1);
  ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = ImVec4(1,1,1,1);
  ImGui::GetStyle().Colors[ImGuiCol_FrameBg] = ImVec4(0.9,0.9,0.9,1);
  ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImVec4(0,0,0,1.f);
  ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ImVec4(0.2,0.2,0.2,1.f);
  ImGui::GetStyle().Colors[ImGuiCol_SliderGrab] = ImVec4(0.2,0.2,0.2,1.f);
  lastTime = glSeconds();

  return true;
}

void closeVideo() {
  glDone();
}

void freeData() {
  freeBindings();
  freeGameObjects();
}

void perFrame() {
  double time = glSeconds(); timeDelta = time - lastTime; lastTime = time;
  static double fpsTimer = 0; static int fpsCounter = 0; fpsTimer += timeDelta; fpsCounter++; if (fpsTimer>2.0) {fps = fpsCounter/2.0;fpsTimer=0;fpsCounter=0;}
  mouseDX = mouseDeltaX; mouseDY = mouseDeltaY;
  mouseB = 0;
  mouseB |= mouseButtons & GL_MOUSE_BUTTON_LEFT ? 1 : 0;
  mouseB |= mouseButtons & GL_MOUSE_BUTTON_RIGHT ? 2 : 0;
  clearAllUnusedBindings();
}

void renderLoop() {
  while(!quit) {
    ImGuiNewFrame();
    if (currentKey == GL_VK_ESCAPE) quit = true;
    perFrame();
    updateCamera();
    drawToolBar();
    drawViewport();
    if (!fullScreen) drawInfo();
    if (!fullScreen) drawTimeLine();
    if (!fullScreen) drawProperties();
    if (!fullScreen) drawMenuBar();
    if (!fullScreen) drawOutliner(); // outliner has to be processed last (before rendering), because it does all the stuff
    drawScene();
    ImGui::Render();
    glRefresh();
  }
}

void defaultInit() {
  addGameObject("Skybox")->setType(GAMEOBJECT_SKYBOX)->loadSkyBox((appFolder+"DATA/CUBEMAPS/SKYBOX6.PNG").c_str());
  loadBaseSVGs();
  initGameObjects();
  initToolBar();
  initRendering();
  initGizmos();
  initDialogs();
  initPaint();
}

#include "STL/IMAGE.HPP"

int main(int argc, const char *argv[]) {

  //appFolder = getPath(argv[0]);
  appFolder = getCWD();

  initLogging();
  logp("SWIFE - STARTUP");
  
  defaultInit();

  initVideo();
  installKeyboardHandler();

  renderLoop();

  uninstallKeyboardHandler();
  closeVideo();

  freeData();

  return 0;
}

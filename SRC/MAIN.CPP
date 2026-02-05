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
#include "GIZMOS.HPP"
#include "INFO.HPP"

double lastTime;

bool initVideo() {
  glWatcomPrecisionTimer(GL_TRUE);
  if (!glVesa(main_window_xres,main_window_yres,16)) {
    main_window_xres = 320;
    main_window_yres = 200;
    return glVGA();
  }
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

void renderLoop() {
  while(!quit) {
    ImGuiNewFrame();
    if (currentKey == GL_VK_ESCAPE) quit = true;
    double time = glSeconds(); timeDelta = time - lastTime; lastTime = time;
    static double fpsTimer = 0; static int fpsCounter = 0; fpsTimer += timeDelta; fpsCounter++; if (fpsTimer>2.0) {fps = fpsCounter/2.0;fpsTimer=0;fpsCounter=0;}
    mouseDX = mouseDeltaX; mouseDY = mouseDeltaY;
    mouseB = 0;
    mouseB |= mouseButtons & GL_MOUSE_BUTTON_LEFT ? 1 : 0;
    mouseB |= mouseButtons & GL_MOUSE_BUTTON_RIGHT ? 2 : 0;
    updateCamera();
    if (!fullScreen) drawOutliner();
    drawToolBar();
    drawViewport();
    if (!fullScreen) drawInfo();
    if (!fullScreen) drawTimeLine();
    if (!fullScreen) drawProperties();
    if (!fullScreen) drawMenuBar();
    drawScene();
    ImGui::Render();
    glRefresh();
  }
}

void defaultInit() {
  for (int i = 0; i < 20; i++) {
    addGameObject("Aha 1")->setType(rand()&1?GAMEOBJECT_MESH:GAMEOBJECT_CAMERA)->addFloat("abc",1)->addText("abc2a34","abc")->addBool("abc",true)->addInt("abc",127);
    addGameObject("Ehahaha");
    addGameObject("Ehaha2");
  }
  loadBaseSVGs();
  initGameObjects();
  initToolBar();
  initRendering();
  initGizmos();
  initDialogs();
}

int main(int argc, const char *argv[]) {
  initLogging();
  logp("SWIFE - STARTUP");
  
  defaultInit();

  initVideo();
  installKeyboardHandler();

  renderLoop();

  uninstallKeyboardHandler();
  closeVideo();

  return 0;
}

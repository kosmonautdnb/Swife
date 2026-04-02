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
#include "TEXPAINT.HPP"
#include "GAMEOBJ.HPP"
#include "SVG.HPP"
#include "DIALOGS.HPP"
#include "KEYMTRIX.HPP"
#include "INFO.HPP"
#include "PAINT.HPP"
#include "DOS.HPP"
#include "SERIALZE.HPP"
#include "STL/IMAGE.HPP"
#include "STL/GLEGA.HPP"

double lastTime;

bool initVideo() {
  glWatcomPrecisionTimer(GL_TRUE);
  if (isEga || (!glVesa(main_window_xres,main_window_yres,16))) {
    main_window_xres = 640;
    main_window_yres = 480;
    toolBar_height *= 2;
    if (!glEGA(main_window_xres,main_window_yres)) return false;
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
  ImGui::GetStyle().Colors[ImGuiCol_TooltipBg] = ImVec4(1,1,1,0.8);
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

double screenSaverStartTime = 0;

void perFrame() {
  double time = glSeconds(); timeDelta = time - lastTime; lastTime = time;
  static double fpsTimer = 0; static int fpsCounter = 0; fpsTimer += timeDelta; fpsCounter++; if (fpsTimer>2.0) {fps = fpsCounter/2.0;fpsTimer=0;fpsCounter=0;}
  mouseDX = mouseDeltaX; mouseDY = mouseDeltaY;
  mouseB = 0;
  mouseB |= mouseButtons & GL_MOUSE_BUTTON_LEFT ? 1 : 0;
  mouseB |= mouseButtons & GL_MOUSE_BUTTON_RIGHT ? 2 : 0;
  GLboolean shift2,ctrl2,alt2; glSpecialKeys(&shift2,&ctrl2,&alt2); shift = shift2 ? true : false;  ctrl = ctrl2 ? true : false; alt = alt2 ? true : false;
  clearAllUnusedBindings();
  deleteGameObjects();
  if (currentKey != 0 || shift || ctrl || alt || mouseDX != 0 || mouseDY != 0 || mouseB != 0) screenSaverStartTime = glSeconds();
}

void takeScreenShot(const char *fileName) {
  RGBAImage img;
  img.data = glFrameBuffer;
  img.width = glFrameBufferWidth;
  img.height = glFrameBufferHeight;
  int newWidth = img.width * aspectRatio * main_window_yres/main_window_xres;
  img = img.getResized(newWidth,img.height);
  memset(glFrameBuffer,0xff,glFrameBufferWidth*glFrameBufferHeight*sizeof(unsigned int));
  glRefresh();
  img.savePNG(fileName);
  img.free();
}

void paintScreenSaver() {
  if (screenSaverOn && ((glSeconds()-screenSaverStartTime) >= SCREENSAVERTIMEINSECONDS)) {
    static Vector textPos(0,0,0);
    static Vector textDir = normalize(Vector(1,1,0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,1));
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::Begin("SCREENSAVER", NULL, ImVec2(main_window_xres,main_window_yres),0.95,FROZENWINDOW);
    String date = "Date " + timeDescString();float fontSize = 2.0;
    ImVec2 textSize = ImGui::CalcTextSize(date.c_str()); textSize.x *= fontSize; textSize.y *= fontSize;
    ImGui::GetWindowDrawList()->AddText(ImGui::GetWindowFont(),ImGui::GetWindowFontSize()*fontSize,ImVec2(textPos.x,textPos.y),0xffffffff,date.c_str());
    textPos += textDir * timeDelta * main_window_xres / 10;
    mouseX = textPos.x;
    mouseY = textPos.y;
    if (textPos.x > main_window_xres-textSize.x) {
      textDir.x = -(double)rand()/RAND_MAX-1.1;
      textDir.y = (double)rand()/RAND_MAX*2-1;
      textDir = normalize(textDir);
    }
    if (textPos.x < 0) {
      textDir.x = (double)rand()/RAND_MAX+1.1;
      textDir.y = (double)rand()/RAND_MAX*2-1;
      textDir = normalize(textDir);
    }
    if (textPos.y > main_window_yres-textSize.y) {
      textDir.x = (double)rand()/RAND_MAX*2-1;
      textDir.y = -(double)rand()/RAND_MAX-1.1;
      textDir = normalize(textDir);
    }
    if (textPos.y < 0) {
      textDir.x = (double)rand()/RAND_MAX*2-1;
      textDir.y = (double)rand()/RAND_MAX+1.1;
      textDir = normalize(textDir);
    }
    ImGui::End();
    ImGui::PopStyleColor();
  }
}

void renderLoop() {
  while(!quit) {
    ImGuiNewFrame();
    perFrame();
    if (currentKey == GL_VK_ESCAPE && shift) dialogQuit(true);
    if (currentKey == GL_VK_F1) {takeScreenShot("SCRNSHOT.PNG");}
    updateCamera();
    drawToolBar();
    drawTexPaint();
    drawViewport();
    if (!fullScreen) drawInfo();
    if (!fullScreen) drawTimeLine();
    if (!fullScreen) drawProperties();
    if (!fullScreen) drawMenuBar();
    if (!fullScreen) drawOutliner(); // outliner has to be processed last (before rendering), because it does all the stuff
    drawScene();
    paintScreenSaver();
    ImGui::Render();
    if (refreshMasked) glRefreshMasked(); else glRefresh();
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
  createDataPointForNew();
  screenSaverStartTime = glSeconds();
}

int main(int argc, const char *argv[]) {

  appFolder = unixFileName(getPath(argv[0]));
  //appFolder = getCWD();

  printf("Swife version: %d.%02d", version / 100, version % 100);
  printf("\n");
  printf("You can find the newest version of Swife at:\n");
  printf("www.github.com/kosmonautdnb/swife\n");
  printf("\n");

  for (int i = 1; i < argc; i++) {
    String parameter = toLower(String(argv[i]));
    if (parameter == "-ega") isEga = true;
    if (parameter == "-4") {aspectRatio = 4.0/3.0;}
    if (parameter == "-?" || parameter == "-help" || parameter == "--help") {
      printf("Commandline parameters:\n");
      printf("-EGA use EGA 640x480\n");
      printf("-4 use 4:3 monitor aspect ratio (not 16:9)\n");
      exit(0);
    }
  }

  initLogging();
  logp("SWIFE - STARTUP");

  if (!initVideo()) {
    printf("Could not initialize graphics..\n");
    exit(0);
  }
  defaultInit();

  installKeyboardHandler();

  logd("starting..");
  renderLoop();
  logd("finishing..");

  uninstallKeyboardHandler();
  closeVideo();

  freeData();

  logd("finished.");

  return 0;
}

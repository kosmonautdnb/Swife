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
#include "TIMELINE.HPP"
#include "MENUBAR.HPP"
#include "OUTLINER.HPP"
#include "TOOLBAR.HPP"
#include "CONFIG.HPP"
#include "PROPS.HPP"
#include "INFO.HPP"
#include "GAMEOBJ.HPP"
#include "STL/ARRAY.HPP"
#include "SERIALZE.HPP"
#include "RENDER.HPP"

String infoString;
int info_height = 32;

#ifdef __WATCOMC__
#include <i86.h>
#endif
#ifdef __DJGPP__
#include <dos.h>
#endif

struct meminfo {
    unsigned LargestBlockAvail;
    unsigned MaxUnlockedPage;
    unsigned LargestLockablePage;
    unsigned LinAddrSpace;
    unsigned NumFreePagesAvail;
    unsigned NumPhysicalPagesFree;
    unsigned TotalPhysicalPages;
    unsigned FreeLinAddrSpace;
    unsigned SizeOfPageFile;
    unsigned Reserved[3];
} MemInfo;

int getAdditionalMemoryFreeBytes() {
#ifdef __WATCOMC__
  union REGS regs;
  struct SREGS sregs;
  memset( &regs, 0, sizeof(regs) );
  memset( &sregs, 0, sizeof(sregs) );
  regs.w.ax = 0x500;
  sregs.es = FP_SEG( &MemInfo );
  regs.x.edi = FP_OFF( &MemInfo );
  int386x( 0x31, &regs, &regs, &sregs );
  return MemInfo.LargestBlockAvail;
#endif
  return -1;
}

void drawInfo() {

  ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(0,0,0,0));
  ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.25,0.25,0.25,1));

  ImGui::SetNextWindowPos(ImVec2(outliner_width,main_window_yres-timeLine_height-info_height));
  ImGui::Begin("INFO", NULL, ImVec2(main_window_xres-outliner_width-properties_width,info_height),1.0,FROZENWINDOW);
  static bool infoOn = false;
  ImGui::Checkbox("##INFOINFOON",&infoOn);
  if (infoOn) {
    ImGui::SameLine();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    int sceneTriangles = 0;
    for (int j = 0; j < gameObjects.size(); j++) { Array<BakedVertex> *k = gameObjects[j]->getBakedTriangles(); if (k != NULL) sceneTriangles += k->size()/3; }
    for (int i = 0; i < 2; i++) {
      ImGui::SetCursorScreenPos(cursor);
      ImGui::TextWrapped("FPS:%0.2f Mem:%0.2fmb Tris:%d RTris:%d %s", fps, (double)getAdditionalMemoryFreeBytes()/1024.0/1024.0, sceneTriangles, renderedTriangles, infoString.c_str());
      cursor.y += 1;
      ImGui::PopStyleColor();
      ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1,1,1,1));
    }
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.25,0.25,0.25,1));
  }
  ImGui::End();

  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
}









         
    
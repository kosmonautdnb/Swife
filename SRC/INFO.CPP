#include "IMGUIGL.HPP"
#include "TIMELINE.HPP"
#include "MENUBAR.HPP"
#include "OUTLINER.HPP"
#include "TOOLBAR.HPP"
#include "CONFIG.HPP"
#include "PROPS.HPP"
#include "INFO.HPP"

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
    ImGui::TextWrapped("FPS:%0.2f Mem:%0.2fmb %s", fps, (double)getAdditionalMemoryFreeBytes()/1024.0/1024.0, infoString.c_str());
  }
  ImGui::End();

  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
}

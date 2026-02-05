#include "IMGUIGL.HPP"
#include "TIMELINE.HPP"
#include "MENUBAR.HPP"
#include "OUTLINER.HPP"
#include "TOOLBAR.HPP"
#include "CONFIG.HPP"
#include "PROPS.HPP"

int timeLine_height = 32;

void drawTimeLine() {
  ImGui::SetNextWindowPos(ImVec2(outliner_width,main_window_yres-timeLine_height));
  ImGui::Begin("TIMELINE", NULL, ImVec2(main_window_xres-outliner_width-properties_width,main_window_yres-toolBar_height-timeLine_height-menuBar_height),1.0,FROZENWINDOW);
  ImGui::Text("TimeLine");
  ImGui::End();
}

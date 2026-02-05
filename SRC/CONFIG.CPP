#include "CONFIG.HPP"
#include "IMGUI/IMGUI.H"

bool fullScreen = false;
double fps = 30.0;
double timeDelta = 0;
double mouseDX = 0, mouseDY = 0;
unsigned int mouseB = 0;

ImVec4 color_text_off = ImVec4(0.5,0.5,0.5,1);
ImVec4 color_text_on = ImVec4(0.0,0.0,0.0,1);
ImVec4 color_button_off_bg = ImVec4(0.7,0.7,0.7,1);
ImVec4 color_button_on_bg = ImVec4(0.0,0.0,0.0,1);
ImVec4 color_button_off_text = ImVec4(0,0,0,1);
ImVec4 color_button_on_text = ImVec4(1,1,1,1);

char *appName = "SWIFE";
uint32_t main_window_xres = 800;
uint32_t main_window_yres = 600;
double aspectRatio = 16.0/9.0;
char *logFileName = "SWIFE.LOG";

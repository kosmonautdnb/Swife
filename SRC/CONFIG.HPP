#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "STL/TYPES.HPP"

#ifndef PI
#define PI 3.14159265358979323846
#endif // PI

#define FROZENWINDOW (ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoTitleBar)
#define NAMEMAX 16
#define FILENAMEMAX 512
#define NONAME "<no name>"

extern bool fullScreen;
extern double fps;
extern double timeDelta;
extern double mouseDX,mouseDY;
extern unsigned int mouseB;

extern struct ImVec4 color_text_off;
extern struct ImVec4 color_text_on;
extern struct ImVec4 color_button_off_bg;
extern struct ImVec4 color_button_on_bg;
extern struct ImVec4 color_button_off_text;
extern struct ImVec4 color_button_on_text;
extern char *appName;

extern uint32_t main_window_xres;
extern uint32_t main_window_yres;
extern double aspectRatio;
extern char *logFileName;

#endif //__CONFIG_H__

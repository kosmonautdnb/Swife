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
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "STL/TYPES.HPP"
#include "STL/STRING.HPP"

#ifndef PI
#define PI 3.14159265358979323846
#endif // PI

#define FROZENWINDOW (ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoCollapse)
#define NAMEMAX 16
#define FILENAMEMAX 512
#define STRINGMAX 3000 // We have a limit in ImGui currently
#define MAXREPEATEDOBJECTSATONCE 100
#define NONAME "<no name>"
#define ERRORCOLOR ImVec4(1,0,0,1)
#define UNUSED(__p__) (void)(__p__)
#define IDINDEXED(__prefix__,__number__) idIndexed(__prefix__,__number__)
#define IDINDEXED2(__prefix__,__number1__,__number2__) idIndexed2(__prefix__,__number1__,__number2__)
#define IDCOMBINED(__prefix__,__suffix__) idCombined(__prefix__,__suffix__)
#define IDCOMBINED3(__string1__,__string2__,__string3__) idCombined3(__string1__,__string2__,__string3__)
#define MAXTEXTUREWIDTH 875 // 1024 (even 1000 may not work) is too much here maybe due to glMalloc that maybe can't allocate more than 4mb at once (WatcomC11) (there are strange behaviours and  crashes)
#define MAXTEXTUREHEIGHT 875 // 1024 (even 1000 may not work) is too much here maybe due to glMalloc that maybe can't allocate more than 4mb at once (WatcomC11) (there are strange behaviours and  crashes)
#define MAXMEMPERALLOC (MAXTEXTUREWIDTH*MAXTEXTUREHEIGHT*4) // (problems with dos allocations on WatcomC)

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
extern const char *appName;

extern String appFolder;

extern uint32_t main_window_xres;
extern uint32_t main_window_yres;
extern double aspectRatio;
extern bool isEga;
extern const char *logFileName;

extern int defaultRenderTargetWidth;
extern int defaultRenderTargetHeight;

const char *idIndexed(const String &str, int number); // this function is not really safe, it cannot be nested
const char *idIndexed(const char *prefix, int number); // this function is not really safe, it cannot be nested
const char *idIndexed2(const char *prefix, int number1, int number2); // this function is not really safe, it cannot be nested
const char *idCombined(const char *prefix, const char *suffix); // this function is not really safe, it cannot be nested
const char *idCombined3(const char *str1, const char *str2,const char *str3); // this function is not really safe, it cannot be nested

#endif //__CONFIG_H__

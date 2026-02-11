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

String appFolder;

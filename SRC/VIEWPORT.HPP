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
#ifndef __VIEWPORT_HPP__
#define __VIEWPORT_HPP__

#include "STL/VECTOR.HPP"
#include "STL/ARRAY.HPP"
#include "STL/STRING.HPP"

class Overlay {
public:
  float x,y;
  String text;
  unsigned int color;
};

extern bool toolOn;
extern bool showOverlays;
extern Array<Overlay> overlays;
extern Vector cursor;
extern Vector cursorNormal;
extern bool placeCursor;

void setOrthoLookTo(double x, double y, double z);
void centerSelection();
void duplicate();
void drawViewport();

void clearOverlays();
void addTextOverlay(double renderX,double renderY, unsigned int col, const char *text);

#endif // __VIEWPORT_HPP__

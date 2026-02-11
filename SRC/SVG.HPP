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
#ifndef __SVG_HPP__
#define __SVG_HPP__

#include "STL/HASHMAP.HPP"
#include "STL/STRING.HPP"

class SVG {
public:
  SVG() {
    glHandle = 0;
    fileName = NULL;
    w = -1;
    h = -1;
    rgba = NULL;
  }
  const char *fileName;
  unsigned int glHandle;
  int w,h;
  unsigned int *rgba;
  SVG *free();
  SVG *set(const char *fileName, unsigned int *rgba, int w, int h);
};

extern HashMap<String, SVG> svgs;

void loadBaseSVGs();

#endif //__SVG_HPP__

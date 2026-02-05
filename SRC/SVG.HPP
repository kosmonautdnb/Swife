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

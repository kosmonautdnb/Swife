#include "GL.H"
#include "SVG.HPP"

HashMap<String, SVG> svgs;

#define NANOSVG_IMPLEMENTATION
#include "SRC\STL\NANOSVG.HPP"
#define NANOSVGRAST_IMPLEMENTATION
#include "SRC\STL\NSVGRAST.HPP"

#define ICONSCALE (16.0/11.0)
#define ICONSCALEX (20.0/11.0)
#define ADDICON(__a__,__b__) addSVG(__a__,"DATA/ICONS/"##__b__,11*ICONSCALE,11*ICONSCALE,1*ICONSCALE)
#define ADDICONX(__a__,__b__) addSVG(__a__,"DATA/ICONS/"##__b__,11*ICONSCALEX,11*ICONSCALEX,1*ICONSCALEX)

SVG *SVG::free() {
  if (rgba != NULL) {::free(rgba); rgba = NULL;}
  if (glHandle != 0) {glDeleteTextures(1,&glHandle); glHandle = 0;}
  w = -1;
  h = -1;
  fileName = NULL;
  return this;
}

SVG *SVG::set(const char *fileName, unsigned int *rgba, int w, int h) {
  SVG::free();
  this->fileName = fileName;
  this->rgba = rgba;
  this->w = w;
  this->h = h;
  glGenTextures(1,&glHandle);
  glBindTexture(GL_TEXTURE_2D,glHandle);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, w, h, 0, GL_RGBA, GL_BYTE, rgba);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  return this;
}

void addSVG(const char *id, const char *fileName, int width, int height,double scale) {
  NSVGimage* image;
  NSVGrasterizer* rast = nsvgCreateRasterizer();
  image = nsvgParseFromFile(fileName, "px", 96);
  if (image == NULL) {
    printf("some problem with loading the svg\n");
    exit(0);
  }
  int w = width;
  int h = height;
  unsigned int* img = (unsigned int*)malloc(w*h*4);
  nsvgRasterize(rast, image, 0,0,scale, (unsigned char*)img, w, h, w*4);
  nsvgDelete(image);
  nsvgDeleteRasterizer(rast);
  svgs[id].set(fileName,img,w,h);
}

void colorIcon(const char *id, unsigned int color) {
  if (!svgs.has(id)) return;
  SVG *svg = &svgs[id];
  unsigned int* rgba2 = (unsigned int*)malloc(svg->w*svg->h*4);
  for (int i = 0; i < svg->w*svg->h; i++) {
    unsigned int rgba = svg->rgba[i];
    int r = color & 255;
    int g = (color>>8) & 255;
    int b = (color>>16) & 255;
    int a = (rgba>>24) & 255;
    rgba2[i] = (r)|(g<<8)|(b<<16)|(a<<24);
  }
  svg->set(svg->fileName,rgba2,svg->w,svg->h);
}

void loadBaseSVGs() {
  ADDICON("3d","3D.SVG");
  ADDICON("camera","CAMERA.SVG");
  ADDICONX("cameraspeed","CAMERA.SVG");
  ADDICON("close","CLOSE.SVG");
  ADDICON("delete","DELETE.SVG");
  ADDICON("drag","DRAG.SVG");
  ADDICON("edit","EDIT.SVG");
  ADDICONX("move","MOVE.SVG");
  ADDICONX("pause","PAUSE.SVG");
  ADDICONX("play","PLAY.SVG");
  ADDICON("plus","PLUS.SVG");
  ADDICONX("rotate","ROTATE.SVG");
  ADDICONX("scale","SCALE.SVG");
  ADDICON("search","SEARCH.SVG");
  ADDICONX("select","SELECT.SVG");
  ADDICON("settings","SETTINGS.SVG");

  ADDICON("undefined","EDIT.SVG");
  ADDICON("path","ROTATE.SVG");
  ADDICON("skybox","SCALE.SVG");
  ADDICON("billboard","PLUS.SVG");
  ADDICON("appicon","3D.SVG");
  colorIcon("appicon",0x00ffffff);
}

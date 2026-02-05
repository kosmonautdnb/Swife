#include "GL.H"
#include "STL/VECTOR.HPP"

bool USEGRID = true;
double XGRIDSIZE = 100.0;
double ZGRIDSIZE = 100.0;
double GRIDSTEPS = 10.0;
unsigned int gridColor = 0xffe0e0ff;
unsigned int gridTexture;

unsigned int getGridColor(int x, int y) {
  unsigned int a = ((x/16)^(y/16))&1?0xff:0x00;
  unsigned int g = (rand() & 15)+200-15;
  return g|(g<<8)|(g<<16)|(a<<24);
}

void initGizmos() {
  // loading "GRID" texture
  glGenTextures(1, &gridTexture);
  glBindTexture(GL_TEXTURE_2D, gridTexture);
  unsigned int *gridTextureData = new unsigned int[256*256];
  for (int x = 0; x < 256; x++) for (int y = 0; y < 256; y++) gridTextureData[x+y*256] = getGridColor(x,y);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,256,256,0,GL_RGBA,GL_UNSIGNED_BYTE,gridTextureData);
  delete[] gridTextureData;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void draw3DGrid() {
  if (!USEGRID) return;
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);
  glDisable(GL_CULL_FACE);
  glAlphaFunc(GL_GREATER,0.5);
  glBindTexture(GL_TEXTURE_2D, gridTexture);
  double s = GRIDSTEPS/16.0;
  glBegin(GL_QUADS);
  glColor4f( ((gridColor)&255)/255.0, ((gridColor>>8)&255)/255.0, ((gridColor>>16)&255)/255.0, ((gridColor>>24)&255)/255.0);
  glTexCoord2f(s,0);glVertex3f(XGRIDSIZE,0,-ZGRIDSIZE);
  glTexCoord2f(0,0);glVertex3f(-XGRIDSIZE,0,-ZGRIDSIZE);
  glTexCoord2f(0,s);glVertex3f(-XGRIDSIZE,0,ZGRIDSIZE);
  glTexCoord2f(s,s);glVertex3f(XGRIDSIZE,0,ZGRIDSIZE);
  glEnd();
  glEnable(GL_CULL_FACE);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_TEXTURE_2D);
//  glBegin(GL_LINES);
//  glColor4f(gridColor.x,gridColor.y,gridColor.z,gridColor.w);
//  for (double x = -XGRIDSIZE; x <= XGRIDSIZE; x += (double)XGRIDSIZE*2/GRIDSTEPS) {
//    glVertex3f(x,0,-ZGRIDSIZE);
//    glVertex3f(x,0,+ZGRIDSIZE);
//  }
//  for (double z = -ZGRIDSIZE; z <= ZGRIDSIZE; z += (double)ZGRIDSIZE*2/GRIDSTEPS) {
//    glVertex3f(-XGRIDSIZE,0,z);
//    glVertex3f(+XGRIDSIZE,0,z);
//  }
//  glEnd();
}

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
#include "GL.H"
#include "PAINT.HPP"
#include "GAMEOBJ.HPP"
#include "CONFIG.HPP"
#include "RENDER.HPP"
#include "STL/VECTOR.HPP"
#include "STL/QUATERNN.HPP"
#include "STL/MATRIX.HPP"
#include "HITTEST.HPP"
#include "SERIALZE.HPP"
#include <math.h>

#define SKYBOX_LEFT 0
#define SKYBOX_FRONT 1
#define SKYBOX_RIGHT 2
#define SKYBOX_BACK 3
#define SKYBOX_TOP 4
#define SKYBOX_BOTTOM 5

bool USEGRID = true;
double XGRIDSIZE = 100.0;
double ZGRIDSIZE = 100.0;
double GRIDSTEPS = 10.0;
unsigned int gridColor = 0xffe0e0ff;
unsigned int gridTexture;

bool justDrawBoundingBoxes = false;

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

void initPaint() {
}

void drawSkyboxQuad(unsigned int color, unsigned int texture, float size, float quadRot, const Vector &camPos, const Quaternion &rotation, const Quaternion &rotation2, const Vector &pos) {
  Matrix m = normalize(rotation).rotationMatrix();
  Matrix m2 = normalize(rotation2).rotationMatrix();
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture);
  glPushMatrix();
  glTranslatef(camPos.x,camPos.y,camPos.z);
  glMultMatrixd(m2.m);
  glTranslatef(pos.x,pos.y,pos.z);
  glMultMatrixd(m.m);
  glBegin(GL_QUADS);
  glColor4ubv((GLubyte*)&color);
  glTexCoord2f(1,0); glVertex3f(size,size,0);
  glTexCoord2f(0,0); glVertex3f(-size,size,0);
  glTexCoord2f(0,1); glVertex3f(-size,-size,0);
  glTexCoord2f(1,1); glVertex3f(size,-size,0);
  glEnd();
  glPopMatrix();
  glDisable(GL_TEXTURE_2D);
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
}

void paintSkyBox(class GameObject *o) {
  FileBinding *binding = getBindingById(o->fileBindingId);
  if (binding->textureIds.size() != 6) return;
  Camera *c = &cameras[currentCamera];
  float size = fabs(c->farPlane)*0.5; // bigger than 1 because of ortho
  Vector cameraPosition = cameras[currentCamera].position;
  Quaternion r = normalize(o->rotation);
  double k = PI / 180.0;
  Quaternion left; left.yawPitchRoll(k * 90,0,0);
  Quaternion right; right.yawPitchRoll(-k * 90,0,0);
  Quaternion front; front.yawPitchRoll(0,0,0);
  Quaternion back; back.yawPitchRoll(k * 180,0,0);
  Quaternion top; top.yawPitchRoll(0,k * 90,0);
  Quaternion bottom; bottom.yawPitchRoll(0,k * -90,0);
  drawSkyboxQuad(o->color,binding->textureIds[SKYBOX_LEFT], size,0,cameraPosition,left,r,Vector(-size,0,0));
  drawSkyboxQuad(o->color,binding->textureIds[SKYBOX_RIGHT], size,0,cameraPosition,right,r,Vector(size,0,0));
  drawSkyboxQuad(o->color,binding->textureIds[SKYBOX_FRONT], size,0,cameraPosition,front,r,Vector(0,0,-size));
  drawSkyboxQuad(o->color,binding->textureIds[SKYBOX_BACK], size,0,cameraPosition,back,r,Vector(0,0,size));
  drawSkyboxQuad(o->color,binding->textureIds[SKYBOX_TOP], size,0,cameraPosition,top,r,Vector(0,size,0));
  drawSkyboxQuad(o->color,binding->textureIds[SKYBOX_BOTTOM], size,0,cameraPosition,bottom,r,Vector(0,-size,0));
}

bool isInViewport(GameObject *o) {
  Matrix projection; glGetDoublev(GL_PROJECTION_MATRIX, projection.m);
  Matrix modelView; glGetDoublev(GL_MODELVIEW_MATRIX, modelView.m);
  unsigned int allOut = 1|2|4|8; // if all points are out on atleast one screen side
  for (int i = 0; i < 8; i++) {
    Vector t = projection * modelView * o->getRenderTransform() * Vector((i&1)?o->boundingBoxMax.x:o->boundingBoxMin.x,(i&2)?o->boundingBoxMax.y:o->boundingBoxMin.y,(i&4)?o->boundingBoxMax.z:o->boundingBoxMin.z);
    if (t.x >= -t.w) allOut &= ~1;
    if (t.x <= t.w) allOut &= ~2;
    if (t.y >= -t.w) allOut &= ~4;
    if (t.y <= t.w) allOut &= ~8;
  }
  return allOut == 0;
}

void paintTrigger(class GameObject *o, bool showModificators) {
  if (!isInViewport(o)) return;

  glPushMatrix();

  Matrix m = o->getRenderTransform();
  Vector minB = o->boundingBoxMin;
  Vector maxB = o->boundingBoxMax;
  glMultMatrixd(m.m);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,0);
  glBindTexture(GL_TEXTURE_2D,gridTexture);


  glPushMatrix();
  Vector center = (minB+maxB)*0.5;
  Vector siz = (maxB-minB)*0.5;
  glTranslatef(center.x,center.y,center.z);
  glScalef(siz.x,siz.y,siz.z);
  glGetDoublev(GL_MODELVIEW_MATRIX,m.m);
  Vector scal(length(m.getColumn(0)),length(m.getColumn(1)),length(m.getColumn(2)));
  for (int i = 0; i < 2; i++) {
    unsigned int col = o->color;
    if (i == 1) col = ((col>>1)&0x007f7f7f)|(col & 0xff000000);
    glFrontFace(i & 1 ? GL_CW : GL_CCW);
    glBegin(GL_QUADS);
    glColor4f((col & 255)/255.f,((col>>8) & 255)/255.f,((col>>16) & 255)/255.f,((col>>24) & 255)/255.f);
    glNormal3f(0,0,1);
    float tx,ty;
    float ta = 1;
    tx = ta * scal.x;
    ty = ta * scal.y;
    glTexCoord2f(0,ty); glVertex3f(-1,-1,+1);
    glTexCoord2f(tx,ty);  glVertex3f(+1,-1,+1);
    glTexCoord2f(tx,0);  glVertex3f(+1,+1,+1);
    glTexCoord2f(0,0);  glVertex3f(-1,+1,+1);
    glNormal3f(0,1,0);
    tx = ta * scal.z;
    ty = ta * scal.x;
    glTexCoord2f(tx,ty);  glVertex3f(-1,+1,+1);
    glTexCoord2f(tx,0);  glVertex3f(+1,+1,+1);
    glTexCoord2f(0,0);  glVertex3f(+1,+1,-1);
    glTexCoord2f(0,ty);  glVertex3f(-1,+1,-1);
    glNormal3f(1,0,0);
    tx = ta * scal.z;
    ty = ta * scal.y;
    glTexCoord2f(tx,ty);   glVertex3f(+1,-1,-1);
    glTexCoord2f(tx,0);    glVertex3f(+1,+1,-1);
    glTexCoord2f(0,0);   glVertex3f(+1,+1,+1);
    glTexCoord2f(0,ty);  glVertex3f(+1,-1,+1);
    glNormal3f(0,0,-1);
    tx = ta * scal.x;
    ty = ta * scal.y;
    glTexCoord2f(tx,0);    glVertex3f(-1,+1,-1);
    glTexCoord2f(0,0);   glVertex3f(+1,+1,-1);
    glTexCoord2f(0,ty);  glVertex3f(+1,-1,-1);
    glTexCoord2f(tx,ty);   glVertex3f(-1,-1,-1);
    glNormal3f(0,-1,0);
    tx = ta * scal.z;
    ty = ta * scal.x;
    glTexCoord2f(0,0);  glVertex3f(-1,-1,-1);
    glTexCoord2f(0,ty);  glVertex3f(+1,-1,-1);
    glTexCoord2f(tx,ty);  glVertex3f(+1,-1,+1);
    glTexCoord2f(tx,0);  glVertex3f(-1,-1,+1);
    glNormal3f(-1,0,0);
    tx = ta * scal.z;
    ty = ta * scal.y;
    glTexCoord2f(tx,ty);   glVertex3f(-1,-1,+1);
    glTexCoord2f(tx,0);   glVertex3f(-1,+1,+1);
    glTexCoord2f(0,0);  glVertex3f(-1,+1,-1);
    glTexCoord2f(0,ty);  glVertex3f(-1,-1,-1);
    glEnd();
    glFrontFace(GL_CCW);
  }
  glPopMatrix();

  glDisable(GL_ALPHA_TEST);
  glDisable(GL_TEXTURE_2D);

  addHitTestCube(o,0,minB,maxB);

  glTranslatef(o->anchor.x,o->anchor.y,o->anchor.z);

  if (showModificators) {
    Vector minB2 = minB - o->anchor;
    Vector maxB2 = maxB - o->anchor;
    addHitTestMoveGizmo(o,minB2,maxB2);
    addHitTestScaleGizmo(o,minB2,maxB2);
    addHitTestRotateGizmo(o,minB2,maxB2);
  }

  glPopMatrix();
}

void paintCamera(class GameObject *o) {
  if (!isInViewport(o)) return;

  glPushMatrix();

  Matrix m = o->getRenderTransform();
  Vector minB = o->boundingBoxMin;
  Vector maxB = o->boundingBoxMax;
  glMultMatrixd(m.m);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,0);
  glBindTexture(GL_TEXTURE_2D,gridTexture);


  glPushMatrix();
  Vector center = (minB+maxB)*0.5;
  Vector siz = (maxB-minB)*0.5;
  glTranslatef(center.x,center.y,center.z);
  glScalef(siz.x,siz.y,siz.z);
  glGetDoublev(GL_MODELVIEW_MATRIX,m.m);
  Vector scal(length(m.getColumn(0)),length(m.getColumn(1)),length(m.getColumn(2)));
  for (int i = 0; i < 2; i++) {
    unsigned int col = o->color;
    if (i == 1) col = ((col>>1)&0x007f7f7f)|(col & 0xff000000);
    glFrontFace(i & 1 ? GL_CW : GL_CCW);
    glBegin(GL_QUADS);
    glColor4f((col & 255)/255.f,((col>>8) & 255)/255.f,((col>>16) & 255)/255.f,((col>>24) & 255)/255.f);
    glNormal3f(0,0,1);
    float tx,ty;
    float ta = 1;
    tx = ta * scal.x;
    ty = ta * scal.y;
    glTexCoord2f(0,ty); glVertex3f(-1,-1,+1);
    glTexCoord2f(tx,ty);  glVertex3f(+1,-1,+1);
    glTexCoord2f(tx,0);  glVertex3f(+1,+1,+1);
    glTexCoord2f(0,0);  glVertex3f(-1,+1,+1);
    glNormal3f(0,1,0);
    tx = ta * scal.z;
    ty = ta * scal.x;
    glTexCoord2f(tx,ty);  glVertex3f(-1,+1,+1);
    glTexCoord2f(tx,0);  glVertex3f(+1,+1,+1);
    glTexCoord2f(0,0);  glVertex3f(+1,+1,-1);
    glTexCoord2f(0,ty);  glVertex3f(-1,+1,-1);
    glNormal3f(1,0,0);
    tx = ta * scal.z;
    ty = ta * scal.y;
    glTexCoord2f(tx,ty);   glVertex3f(+1,-1,-1);
    glTexCoord2f(tx,0);    glVertex3f(+1,+1,-1);
    glTexCoord2f(0,0);   glVertex3f(+1,+1,+1);
    glTexCoord2f(0,ty);  glVertex3f(+1,-1,+1);
    glNormal3f(0,0,-1);
    tx = ta * scal.x;
    ty = ta * scal.y;
    glTexCoord2f(tx,0);    glVertex3f(-1,+1,-1);
    glTexCoord2f(0,0);   glVertex3f(+1,+1,-1);
    glTexCoord2f(0,ty);  glVertex3f(+1,-1,-1);
    glTexCoord2f(tx,ty);   glVertex3f(-1,-1,-1);
    glNormal3f(0,-1,0);
    tx = ta * scal.z;
    ty = ta * scal.x;
    glTexCoord2f(0,0);  glVertex3f(-1,-1,-1);
    glTexCoord2f(0,ty);  glVertex3f(+1,-1,-1);
    glTexCoord2f(tx,ty);  glVertex3f(+1,-1,+1);
    glTexCoord2f(tx,0);  glVertex3f(-1,-1,+1);
    glNormal3f(-1,0,0);
    tx = ta * scal.z;
    ty = ta * scal.y;
    glTexCoord2f(tx,ty);   glVertex3f(-1,-1,+1);
    glTexCoord2f(tx,0);   glVertex3f(-1,+1,+1);
    glTexCoord2f(0,0);  glVertex3f(-1,+1,-1);
    glTexCoord2f(0,ty);  glVertex3f(-1,-1,-1);
    glEnd();
    glFrontFace(GL_CCW);
  }
  glPopMatrix();

  glDisable(GL_ALPHA_TEST);
  glDisable(GL_TEXTURE_2D);

  addHitTestCube(o,0,minB,maxB);

  glPopMatrix();
}

void paintMesh(class GameObject *o, bool showModificators) {
  Array<BakedVertex> *triangles = o->getBakedTriangles();
  if (triangles == NULL) {
    o->hasDrawProblem = true;
    drawProblems++;
    return;
  }
  if (!isInViewport(o)) return;
  if (justDrawBoundingBoxes) {paintTrigger(o,showModificators); return;}

  glPushMatrix();

  Matrix m = o->getRenderTransform();
  Vector minB = o->boundingBoxMin;
  Vector maxB = o->boundingBoxMax;

  glMultMatrixd(m.m);
  if (texturedOn) glEnable(GL_TEXTURE_2D); else glDisable(GL_TEXTURE_2D);
  if (twoSidedOn) glDisable(GL_CULL_FACE); else glEnable(GL_CULL_FACE);

  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,0);

  glBegin(GL_TRIANGLES);
  for (int i = 0; i < triangles->size(); i++) {
    BakedVertex *v = &(*triangles)[i];
    glBindTexture(GL_TEXTURE_2D,v->textureId); // non standard opengl (in glBegin/glEnd here)
    glColor4ubv((GLubyte*)&v->color);
    glTexCoord2f(v->tx,v->ty);
    glNormal3f(v->nx,v->ny,v->nz);
    glVertex3f(v->x,v->y,v->z);
  }
  glEnd();

  glDisable(GL_ALPHA_TEST);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_CULL_FACE);

  addHitTestTriangles(o,0,triangles,minB,maxB);

  glTranslatef(o->anchor.x,o->anchor.y,o->anchor.z);

  if (showModificators) {
    Vector minB2 = minB - o->anchor;
    Vector maxB2 = maxB - o->anchor;
    addHitTestMoveGizmo(o,minB2,maxB2);
    addHitTestScaleGizmo(o,minB2,maxB2);
    addHitTestRotateGizmo(o,minB2,maxB2);
  }

  glPopMatrix();
}


void paintBillBoard(class GameObject *o, bool showModificators) {
  if (!isInViewport(o)) return;

  FileBinding *binding = getBindingById(o->fileBindingId);
  float tx0=0,ty0=0,tx1=1,ty1=1;
  unsigned int textureId = gridTexture;
  if (binding != NULL) {
    if (binding->billBoardIsTexture) {
      double w = binding->billBoardOriginalWidth;
      double h = binding->billBoardOriginalHeight;
      if (w == 0) w = 1;
      if (h == 0) h = 1;
      tx0 = o->billBoardTX0/w;
      ty0 = o->billBoardTY0/h;
      tx1 = o->billBoardTX1/w;
      ty1 = o->billBoardTY1/h;
      if (binding->textureIds.size()==1) textureId = binding->textureIds[0];
    }
  }

  bool lightingWasOn = glIsEnabled(GL_LIGHTING);
  glDisable(GL_LIGHTING);

  glPushMatrix();

  Matrix m = o->getRenderTransform();
  Vector minB = o->boundingBoxMin;
  Vector maxB = o->boundingBoxMax;

  glMultMatrixd(m.m);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,0);
  glBindTexture(GL_TEXTURE_2D,textureId);

  glPushMatrix();
  Vector center = (minB+maxB)*0.5;
  Vector siz = (maxB-minB)*0.5;
  glTranslatef(center.x,center.y,center.z);
  glScalef(siz.x,siz.y,siz.z);

  glBegin(GL_QUADS);
  unsigned int col = o->color;
  glColor4f((col & 255)/255.f,((col>>8) & 255)/255.f,((col>>16) & 255)/255.f,((col>>24) & 255)/255.f);
  glNormal3f(0,0,1);
  glTexCoord2f(tx0,ty1); glVertex3f(-1,-1,+1);
  glTexCoord2f(tx1,ty1);  glVertex3f(+1,-1,+1);
  glTexCoord2f(tx1,ty0);  glVertex3f(+1,+1,+1);
  glTexCoord2f(tx0,ty0);  glVertex3f(-1,+1,+1);
  glEnd();

  glPopMatrix();

  glDisable(GL_ALPHA_TEST);
  glDisable(GL_TEXTURE_2D);

  addHitTestCube(o,0,minB,maxB);

  glTranslatef(o->anchor.x,o->anchor.y,o->anchor.z);

  if (showModificators) {
    Vector minB2 = minB - o->anchor;
    Vector maxB2 = maxB - o->anchor;
    addHitTestMoveGizmo(o,minB2,maxB2);
  }

  glPopMatrix();

  if (lightingWasOn) glEnable(GL_LIGHTING); else glDisable(GL_LIGHTING);
}

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
#include "IMGUI.H"
#include "PAINT.HPP"
#include "GAMEOBJ.HPP"
#include "CONFIG.HPP"
#include "RENDER.HPP"
#include "STL/VECTOR.HPP"
#include "STL/QUATERNN.HPP"
#include "STL/MATRIX.HPP"
#include "HITTEST.HPP"
#include "SERIALZE.HPP"
#include "VIEWPORT.HPP"
#include "TOOLBAR.HPP"
#include "PROPS.HPP"
#include "STL/SPRTEOBJ.HPP"
#include <math.h>

#define SKYBOX_LEFT 0
#define SKYBOX_FRONT 1
#define SKYBOX_RIGHT 2
#define SKYBOX_BACK 3
#define SKYBOX_TOP 4
#define SKYBOX_BOTTOM 5

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
  if (!showGrids) return;
  Grid *grid = &grids[0];
  if (!grid->displayGridOn) return;
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);
  glDisable(GL_CULL_FACE);
  glAlphaFunc(GL_GREATER,0.5);
  glBindTexture(GL_TEXTURE_2D, gridTexture);
  double s = 1.0;
  float xs = grid->displayGridSize;
  float zs = grid->displayGridSize;
  if (grid->displayGridSteps != 0) s = grid->displayGridSize*2.0/grid->displayGridSteps/16.0;
  Vector gridPos = grid->gridPos;
  glBegin(GL_QUADS);
  glColor4f( ((grid->displayGridColor)&255)/255.0, ((grid->displayGridColor>>8)&255)/255.0, ((grid->displayGridColor>>16)&255)/255.0, ((grid->displayGridColor>>24)&255)/255.0);
  glTexCoord2f(s,0);glVertex3f(xs+gridPos.x,gridPos.y,-zs+gridPos.z);
  glTexCoord2f(0,0);glVertex3f(-xs+gridPos.x,gridPos.y,-zs+gridPos.z);
  glTexCoord2f(0,s);glVertex3f(-xs+gridPos.x,gridPos.y,zs+gridPos.z);
  glTexCoord2f(s,s);glVertex3f(xs+gridPos.x,gridPos.y,zs+gridPos.z);
  glEnd();
  glEnable(GL_CULL_FACE);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_TEXTURE_2D);
}

void initPaint() {
}

void drawSkyboxQuad(unsigned int color, unsigned int texture, float size, float quadRot, const Vector &camPos, const Quaternion &rotation, const Quaternion &rotation2, const Vector &pos) {
  UNUSED(quadRot);
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

  exportObject(o);

  FileBinding *binding = getBindingById(o->fileBindingId); if (binding == NULL) return;
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

void combineForNoRotation(class Matrix *modelViewMatrix, const class Matrix *modelViewMatrixToGetRotationFrom,bool maxScale) {
  double scaleX = length(modelViewMatrix->getColumn(0));
  double scaleY = length(modelViewMatrix->getColumn(1));
  double scaleZ = length(modelViewMatrix->getColumn(2));
  double scale = scaleX; if (scaleY > scale) scale = scaleY; if (scaleZ > scale) scale = scaleZ;
  if (maxScale) scaleX=scaleY=scaleZ=scale;
  Matrix ret = *modelViewMatrix;
  ret.setColumn3(0,modelViewMatrixToGetRotationFrom->getColumn(0)*scaleX);
  ret.setColumn3(1,modelViewMatrixToGetRotationFrom->getColumn(1)*scaleY);
  ret.setColumn3(2,modelViewMatrixToGetRotationFrom->getColumn(2)*scaleZ);
  *modelViewMatrix = ret;
}

void addHitTestMoveGizmo2(const Matrix &cameraMatrix, GameObject *o, const Vector &minB2, const Vector &maxB2) {
  if (moveToolWorldSpace) {
    glPushMatrix();
    Matrix objectMatrix; glGetDoublev(GL_MODELVIEW_MATRIX, objectMatrix.m);
    combineForNoRotation(&objectMatrix,&cameraMatrix,false);
    glLoadMatrixd(objectMatrix.m);
    addHitTestMoveGizmo(o,minB2,maxB2);
    glPopMatrix();
  } else {
    addHitTestMoveGizmo(o,minB2,maxB2);
  }
}

void paintTrigger(class GameObject *o, bool showModificators) {

  exportObject(o);

  if (!isInViewport(o)) return;

  Matrix cameraMatrix; glGetDoublev(GL_MODELVIEW_MATRIX, cameraMatrix.m);

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

  addHitTestCube(o,HITTESTTOOL_NONE,minB,maxB);

  glTranslatef(o->anchor.x,o->anchor.y,o->anchor.z);

  if (showModificators) {
    Vector minB2 = minB - o->anchor;
    Vector maxB2 = maxB - o->anchor;
    addHitTestScaleGizmo(o,minB2,maxB2);
    addHitTestRotateGizmo(o,minB2,maxB2);
    addHitTestMoveGizmo2(cameraMatrix,o,minB2,maxB2);
  }

  glPopMatrix();
}

void paintCamera(class GameObject *o) {

  exportObject(o);

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

  unsigned int col = o->color;
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

  glPopMatrix();

  glDisable(GL_ALPHA_TEST);
  glDisable(GL_TEXTURE_2D);

  addHitTestCube(o,HITTESTTOOL_NONE,minB,maxB);

  glPopMatrix();
}

void paintMesh(class GameObject *o, bool showModificators) {

  exportObject(o);

  Array<BakedVertex> *triangles = o->getBakedTriangles();
  if (triangles == NULL) {
    o->hasDrawProblem = true;
    drawProblems++;
    return;
  }
  if (!isInViewport(o)) return;
  FileBinding *b = getBindingById(o->fileBindingId);
  if (b == NULL || justDrawBoundingBoxes) {paintTrigger(o,showModificators); return;}

  Matrix cameraMatrix; glGetDoublev(GL_MODELVIEW_MATRIX, cameraMatrix.m);

  glPushMatrix();

  Matrix m = o->getRenderTransform();
  Vector minB = o->boundingBoxMin;
  Vector maxB = o->boundingBoxMax;

  glMultMatrixd(m.m);
  if (texturedOn) glEnable(GL_TEXTURE_2D); else glDisable(GL_TEXTURE_2D);
  if (twoSidedOn) glDisable(GL_CULL_FACE); else glEnable(GL_CULL_FACE);
  if (b->forceTwoSided) glDisable(GL_CULL_FACE);

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
    addHitTestScaleGizmo(o,minB2,maxB2);
    addHitTestRotateGizmo(o,minB2,maxB2);
    addHitTestMoveGizmo2(cameraMatrix,o,minB2,maxB2);
  }

  glPopMatrix();
}

void paintBillBoard(class GameObject *o, bool showModificators) {

  exportObject(o);

  FileBinding *binding = getBindingById(o->fileBindingId);

  Matrix cameraMatrix; glGetDoublev(GL_MODELVIEW_MATRIX, cameraMatrix.m);

  float tx0=0,ty0=0,tx1=1,ty1=1;
  unsigned int textureId = gridTexture;
  if (binding != NULL) {
    if (binding->billBoardIsTexture) {
      double w = binding->originalWidth;
      double h = binding->originalHeight;
      if (w == 0) w = 1;
      if (h == 0) h = 1;
      tx0 = o->billBoardTX0/w;
      ty0 = o->billBoardTY0/h;
      tx1 = o->billBoardTX1/w;
      ty1 = o->billBoardTY1/h;
      if (binding->textureIds.size()==1) textureId = binding->textureIds[0];
    }
    if (!binding->billBoardIsTexture && binding->spriteObject != NULL) {
      textureId = binding->spriteObject->frameBufferColor;
      o->boundingBoxMin.set(binding->spriteObject->minx,binding->spriteObject->miny,-0.01);
      o->boundingBoxMax.set(binding->spriteObject->maxx,binding->spriteObject->maxy,0.01);
    }
  }

  if (!isInViewport(o)) return;

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
  glTexCoord2f(tx0,ty1); glVertex3f(-1,-1,0);
  glTexCoord2f(tx1,ty1);  glVertex3f(+1,-1,0);
  glTexCoord2f(tx1,ty0);  glVertex3f(+1,+1,0);
  glTexCoord2f(tx0,ty0);  glVertex3f(-1,+1,0);
  glEnd();

  glPopMatrix();

  glDisable(GL_ALPHA_TEST);
  glDisable(GL_TEXTURE_2D);

  addHitTestCube(o,HITTESTTOOL_NONE,minB,maxB);

  glTranslatef(o->anchor.x,o->anchor.y,o->anchor.z);

  if (showModificators) {
    Vector minB2 = minB - o->anchor;
    Vector maxB2 = maxB - o->anchor;
    Matrix objectMatrix; glGetDoublev(GL_MODELVIEW_MATRIX, objectMatrix.m);
    combineForNoRotation(&objectMatrix,&cameraMatrix,true);
    glLoadMatrixd(objectMatrix.m);
    addHitTestMoveGizmo(o,minB2,maxB2);
  }

  glPopMatrix();

  if (lightingWasOn) glEnable(GL_LIGHTING); else glDisable(GL_LIGHTING);
}

void paintBillBoardContents() {
  for (int i = 0; i < bindings.size(); i++) {
    FileBinding *b = bindings[i];
    if (b->type == FILETYPE_BILLBOARD && (!b->billBoardIsTexture) && b->spriteObject != NULL) {

      startSpriteObjectPainting(b->spriteObject, &b->boundingBoxMin, &b->boundingBoxMax);

      Array<BakedVertex> *triangles = &b->triangles;

      if (triangles != NULL) {
    
        glPushMatrix();
      
        Vector minB = b->boundingBoxMin;
        Vector maxB = b->boundingBoxMax;
      
        if (texturedOn) glEnable(GL_TEXTURE_2D); else glDisable(GL_TEXTURE_2D);
        if (twoSidedOn) glDisable(GL_CULL_FACE); else glEnable(GL_CULL_FACE);
        if (b->forceTwoSided) glDisable(GL_CULL_FACE);
      
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

        glPopMatrix();
      }

      finishSpriteObjectPainting();
    }
  }
}

void paintLink(class GameObject *o1, class GameObject *o2) {
  Vector v1 = o1->getRenderTransform()*((o1->boundingBoxMin+o1->boundingBoxMax)*0.5);
  Vector v2 = o2->getRenderTransform()*((o2->boundingBoxMin+o2->boundingBoxMax)*0.5);
  bool lightingWasOn = glIsEnabled(GL_LIGHTING); glDisable(GL_LIGHTING);
  glBegin(GL_LINES);
  glColor4f(0,1,0,1);
  glVertex3dv(&v1.x);
  glVertex3dv(&v2.x);
  glEnd();
  if (lightingWasOn) glEnable(GL_LIGHTING); else glDisable(GL_LIGHTING);
}

void paintPoint(class GameObject *o, bool showModificators) {

  exportObject(o);

  if (!isInViewport(o)) return;

  Matrix cameraMatrix; glGetDoublev(GL_MODELVIEW_MATRIX, cameraMatrix.m);
  Matrix projectionMatrix; glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix.m);

  glPushMatrix();
  Matrix m = o->getRenderTransform();
  glMultMatrixd(m.m);
  Vector minB = o->boundingBoxMin;
  Vector maxB = o->boundingBoxMax;
  Vector center = (minB+maxB)*0.5;
  Vector siz = (maxB-minB)*0.5;
  glTranslatef(center.x,center.y,center.z);
  glScalef(siz.x,siz.y,siz.z);

  bool lightingWasOn = glIsEnabled(GL_LIGHTING); glDisable(GL_LIGHTING);

  glBegin(GL_QUADS);
  unsigned int col = o->color;
  glColor4f((col & 255)/255.f,((col>>8) & 255)/255.f,((col>>16) & 255)/255.f,((col>>24) & 255)/255.f);
  glNormal3f(0,0,1);
  glVertex3f(-1,-1,+1);
  glVertex3f(+1,-1,+1);
  glVertex3f(+1,+1,+1);
  glVertex3f(-1,+1,+1);
  glNormal3f(0,1,0);
  glVertex3f(-1,+1,+1);
  glVertex3f(+1,+1,+1);
  glVertex3f(+1,+1,-1);
  glVertex3f(-1,+1,-1);
  glNormal3f(1,0,0);
  glVertex3f(+1,-1,-1);
  glVertex3f(+1,+1,-1);
  glVertex3f(+1,+1,+1);
  glVertex3f(+1,-1,+1);
  glNormal3f(0,0,-1);
  glVertex3f(-1,+1,-1);
  glVertex3f(+1,+1,-1);
  glVertex3f(+1,-1,-1);
  glVertex3f(-1,-1,-1);
  glNormal3f(0,-1,0);
  glVertex3f(-1,-1,-1);
  glVertex3f(+1,-1,-1);
  glVertex3f(+1,-1,+1);
  glVertex3f(-1,-1,+1);
  glNormal3f(-1,0,0);
  glVertex3f(-1,-1,+1);
  glVertex3f(-1,+1,+1);
  glVertex3f(-1,+1,-1);
  glVertex3f(-1,-1,-1);
  glEnd();

  if (lightingWasOn) glEnable(GL_LIGHTING); else glDisable(GL_LIGHTING);

  addHitTestCube(o,HITTESTTOOL_NONE,minB,maxB);

  glTranslatef(o->anchor.x,o->anchor.y,o->anchor.z);

  if (showModificators) {
    if (!o->pointHasTransform) {
      Vector minB2 = minB - o->anchor;
      Vector maxB2 = maxB - o->anchor;
      Matrix objectMatrix; glGetDoublev(GL_MODELVIEW_MATRIX, objectMatrix.m);
      combineForNoRotation(&objectMatrix,&cameraMatrix,true);
      glLoadMatrixd(objectMatrix.m);
      addHitTestMoveGizmo(o,minB2,maxB2);
    } else {
      Vector minB2 = minB - o->anchor;
      Vector maxB2 = maxB - o->anchor;
      addHitTestScaleGizmo(o,minB2,maxB2);
      addHitTestRotateGizmo(o,minB2,maxB2);
      addHitTestMoveGizmo2(cameraMatrix,o,minB2,maxB2);
    }
  }

  glPopMatrix();

  Vector pos = projectionMatrix * cameraMatrix * m * center;
  if (pos.w != 0) {pos /= pos.w; pos.w = 1;}
  addTextOverlay(pos.x,pos.y,col,o->caption);
}

Vector textureGridVertexRaw(FileBinding *b, int x, int y, int xa, int ya) {
  float d = b->floatContent[(x+xa)+(y+ya)*(b->textureGridWidth+1)];
  return Vector((float)(x+xa)/b->textureGridWidth*2-1,d,(float)(y+ya)/b->textureGridHeight*2-1);
}

Vector textureGridVertex(FileBinding *b, int x, int y, int xa, int ya) {
  int i = b->uintContent[x+y*b->textureGridWidth];
  int tx = i & 0x3fff; // 16384 in x
  int ty = i / 0x4000;
  float tx0 = ((float)tx * b->textureAtlasTileWidth) / b->originalWidth;
  float ty0 = ((float)ty * b->textureAtlasTileHeight) / b->originalHeight;
  float tx1 = ((float)(tx+1) * b->textureAtlasTileWidth) / b->originalWidth;
  float ty1 = ((float)(ty+1) * b->textureAtlasTileHeight) / b->originalHeight;
  glTexCoord2f(xa==0?tx0:tx1,ya==0?ty0:ty1);
  Vector ret = textureGridVertexRaw(b,x,y,xa,ya);
  glVertex3dv(&ret.x);
  return ret;
}

void paintIntoTextureGrid(class FileBinding *binding, int arrayPos, int toPaint) {
  if (arrayPos<0||arrayPos>=binding->uintContent.size()) return;
  if (binding->textureAtlasTileCountX<=0) return;
  int tx = toPaint % binding->textureAtlasTileCountX;
  int ty = toPaint / binding->textureAtlasTileCountX;
  binding->uintContent[arrayPos] = (tx & 0x3fff)+(ty * 0x4000);
  renderingUpdated = true;
}

int getFromTextureGrid(class FileBinding *binding, int arrayPos) {
  if (arrayPos<0||arrayPos>=binding->uintContent.size()) return -1;
  if (binding->textureAtlasTileCountX<=0) return -1;
  int i = binding->uintContent[arrayPos];
  int tx = i & 0x3fff;
  int ty = i / 0x4000;
  return tx + ty*binding->textureAtlasTileCountX;
}

void paintTextureGrid(class GameObject *o, bool showModificators) {

  FileBinding *binding = getBindingById(o->fileBindingId);

  if (binding != NULL) {
    o->boundingBoxMin = binding->boundingBoxMin;
    o->boundingBoxMax = binding->boundingBoxMax;
  }

  exportObject(o);

  if (!isInViewport(o)) return;

  if (justDrawBoundingBoxes) {paintTrigger(o,showModificators); return;}

  Matrix cameraMatrix; glGetDoublev(GL_MODELVIEW_MATRIX, cameraMatrix.m);

  glPushMatrix();
  Matrix m = o->getRenderTransform();
  glMultMatrixd(m.m);
  Vector minB = o->boundingBoxMin;
  Vector maxB = o->boundingBoxMax;
  Vector center = (minB+maxB)*0.5;
  Vector siz = (maxB-minB)*0.5;
  //glTranslatef(center.x,center.y,center.z);
  //glScalef(siz.x,siz.y,siz.z);

  bool lightingWasOn = glIsEnabled(GL_LIGHTING); //glDisable(GL_LIGHTING);

  glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,0);
  glBindTexture(GL_TEXTURE_2D,binding != NULL && binding->textureIds.size()==1 ? binding->textureIds[0]:0);

  bool noMap = false;
  if (binding != NULL) {
    if (binding->textureGridWidth<=0||binding->textureGridHeight<=0) noMap = true;
    if (binding->originalWidth<=0||binding->originalHeight<=0) noMap = true;
    if (binding->textureAtlasTileWidth<=0||binding->textureAtlasTileHeight<=0||binding->textureAtlasTileCountX<=0||binding->textureAtlasTileCountY<=0) noMap = true;
  } else noMap = true;

  Vector gizmoPos;
  bool hasGizmo = false;

  Matrix modelView; glGetDoublev(GL_MODELVIEW_MATRIX, modelView.m);

  glBegin(GL_QUADS);
  unsigned int col = o->color;
  glColor4f((col & 255)/255.f,((col>>8) & 255)/255.f,((col>>16) & 255)/255.f,((col>>24) & 255)/255.f);
  glNormal3f(0,1,0);
  if (noMap) {
    glVertex3f(-1,0,+1);
    glVertex3f(+1,0,+1);
    glVertex3f(+1,0,-1);
    glVertex3f(-1,0,-1);
  } else {
    for (int z = 0; z < binding->textureGridHeight; z++) {
      for (int x = 0; x < binding->textureGridWidth; x++) {
        glColor4f((col & 255)/255.f,((col>>8) & 255)/255.f,((col>>16) & 255)/255.f,((col>>24) & 255)/255.f);
        if (o->showStructure) {
          bool k = (x + z) & 1 ? true : false;
          glColor4f(k?0.75:0.25,k?0.75:0.25,k?0.75:0.25,1);
        }
        if (binding->index == x + z * binding->textureGridWidth) {
          glColor4f(0.25,0.25,0.25,1); binding->index = -1;
        }
        Vector p[4];
        p[0] = textureGridVertexRaw(binding,x,z,1,0);
        p[1] = textureGridVertexRaw(binding,x,z,0,0);
        p[2] = textureGridVertexRaw(binding,x,z,0,1);
        p[3] = textureGridVertexRaw(binding,x,z,1,1);
        Vector dx = p[3]-p[1];
        Vector dy = p[2]-p[0];
        Vector n = normalize(cross(dy,dx));
        glNormal3dv(&n.x);
        p[0] = textureGridVertex(binding,x,z,1,0);
        p[1] = textureGridVertex(binding,x,z,0,0);
        p[2] = textureGridVertex(binding,x,z,0,1);
        p[3] = textureGridVertex(binding,x,z,1,1);
        if (mode == MODE_SELECT) {
          Vector collisionPoint;
          if (triangleHitTest(p[0],p[1],p[2],collisionPoint)||triangleHitTest(p[2],p[3],p[0],collisionPoint)) {
            if (o->selected) {
              Matrix modelView; glGetDoublev(GL_MODELVIEW_MATRIX, modelView.m);
              Matrix m2 = hitTestInverseModelView*modelView;
              Vector t[4]; t[0] = m2*p[0]; t[1] = m2*p[1]; t[2] = m2*p[2]; t[3] = m2*p[3];
              int k = 0;
              if (length(collisionPoint-t[1])<length(collisionPoint-t[k])) k = 1;
              if (length(collisionPoint-t[2])<length(collisionPoint-t[k])) k = 2;
              if (length(collisionPoint-t[3])<length(collisionPoint-t[k])) k = 3;
              Hit h;
              h.depth = length(collisionPoint - mouseHitStart);
              h.point = collisionPoint;
              h.object = o;
              h.toolId = HITTESTTOOL_TEXTUREGRIDPAINT;
              h.index = x + z * binding->textureGridWidth;
              h.index2 = x+(((k-1)&3)/2) + (z+k/2) * (binding->textureGridWidth+1);
              Vector normal = normalize(inverse(transpose(inverse(cameraMatrix)*modelView))*n);
              h.pointOrientation = normalize(rotationMatrixToQuaternion(rotationMatrixFromNormal(normal)));
              hits.push_back(h);
            }
            Hit h2;
            h2.depth = length(collisionPoint - mouseHitStart);
            h2.point = collisionPoint;
            h2.object = o;
            h2.toolId = HITTESTTOOL_NONE;
            Vector normal = normalize(inverse(transpose(inverse(cameraMatrix)*modelView))*n);
            h2.pointOrientation = normalize(rotationMatrixToQuaternion(rotationMatrixFromNormal(normal)));
            hits.push_back(h2);
          }
        }
      }
    }
  }
  glEnd();

  if (binding != NULL&&binding->index2>=0&&binding->textureGridWidth>0&&binding->index2<binding->floatContent.size()) {
    int px = binding->index2 % (binding->textureGridWidth+1);
    int py = binding->index2 / (binding->textureGridWidth+1);
    Matrix modelView; glGetDoublev(GL_MODELVIEW_MATRIX,modelView.m);
    gizmoPos = inverse(cameraMatrix) * modelView * textureGridVertexRaw(binding,px,py,0,0);
    hasGizmo = true;
  }

  glDisable(GL_ALPHA_TEST);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_CULL_FACE);

  if (lightingWasOn) glEnable(GL_LIGHTING); else glDisable(GL_LIGHTING);

  if (noMap) 
    addHitTestCube(o,HITTESTTOOL_NONE,minB,maxB);

  glTranslatef(o->anchor.x,o->anchor.y,o->anchor.z);

  if (showModificators) {
    Vector minB2 = minB - o->anchor;
    Vector maxB2 = maxB - o->anchor;
    addHitTestScaleGizmo(o,minB2,maxB2);
    addHitTestRotateGizmo(o,minB2,maxB2);
    addHitTestMoveGizmo2(cameraMatrix,o,minB2,maxB2);
  }

  glPopMatrix();

  if (hasGizmo) {
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatedv(&gizmoPos.x);
    glBegin(GL_QUADS);
    unsigned int col = 0xff00ff00;
    glColor4f((col & 255)/255.f,((col>>8) & 255)/255.f,((col>>16) & 255)/255.f,((col>>24) & 255)/255.f);
    glNormal3f(0,0,1);
    glVertex3f(-1,-1,+1);
    glVertex3f(+1,-1,+1);
    glVertex3f(+1,+1,+1);
    glVertex3f(-1,+1,+1);
    glNormal3f(0,1,0);
    glVertex3f(-1,+1,+1);
    glVertex3f(+1,+1,+1);
    glVertex3f(+1,+1,-1);
    glVertex3f(-1,+1,-1);
    glNormal3f(1,0,0);
    glVertex3f(+1,-1,-1);
    glVertex3f(+1,+1,-1);
    glVertex3f(+1,+1,+1);
    glVertex3f(+1,-1,+1);
    glNormal3f(0,0,-1);
    glVertex3f(-1,+1,-1);
    glVertex3f(+1,+1,-1);
    glVertex3f(+1,-1,-1);
    glVertex3f(-1,-1,-1);
    glNormal3f(0,-1,0);
    glVertex3f(-1,-1,-1);
    glVertex3f(+1,-1,-1);
    glVertex3f(+1,-1,+1);
    glVertex3f(-1,-1,+1);
    glNormal3f(-1,0,0);
    glVertex3f(-1,-1,+1);
    glVertex3f(-1,+1,+1);
    glVertex3f(-1,+1,-1);
    glVertex3f(-1,-1,-1);
    glEnd();
    glPopMatrix();
    glEnable(GL_CULL_FACE);
    if (lightingWasOn) glEnable(GL_LIGHTING); else glDisable(GL_LIGHTING);
  }
}

void paintCursor() {
  Vector pos = cursor;
  Matrix modelView; glGetDoublev(GL_MODELVIEW_MATRIX,modelView.m);
  Matrix projection; glGetDoublev(GL_PROJECTION_MATRIX,projection.m);
  Vector screenSpace = projection * modelView * pos;
  if (screenSpace.w != 0) {screenSpace /= screenSpace.w; screenSpace.w = 1;}
  double rad = main_window_xres/50;
  Vector screenSpaceP = inverse(modelView) * inverse(projection) * screenSpace;
  Vector screenSpaceX = inverse(modelView) * inverse(projection) * (screenSpace+Vector(rad/main_window_xres,0,0));
  if (screenSpaceP.w != 0) {screenSpaceP /= screenSpaceP.w; screenSpaceP.w = 1;}
  if (screenSpaceX.w != 0) {screenSpaceX /= screenSpaceX.w; screenSpaceX.w = 1;}
  double radius = length(screenSpaceX-screenSpaceP);
  Matrix invModelView = inverse(modelView);
  Vector tangent = normalize(invModelView.getColumn(0));
  Vector cotangent = normalize(invModelView.getColumn(1));
  int steps = 20;
  glBegin(GL_LINES);
  glColor4f(1,0,0,1);
  for (int i = 0; i < steps; i++) {
    float angle = i * 2.0 * PI / steps;
    Vector p = pos;
    p += sin(angle)*tangent*radius + cos(angle)*cotangent*radius;
    glVertex3dv(&p.x);
  }
  glEnd();
  glBegin(GL_POINTS);
  glColor4f(1,1,1,1);
  glVertex3dv(&pos.x);
  glEnd();
}

double catmulromT(double t, double alpha, const Vector &p0, const Vector &p1)  {
  if (alpha < 0.0001) alpha = 0.0001;
  Vector d = p1 - p0;
  return pow(dot(d,d),alpha*0.5) + t;
}

double divi(double a,double b) {
  return b == 0 ? 0.5 : a/b;
}

Vector catmulrom(const Vector &p0, const Vector &p1, const Vector &p2, const Vector &p3, double t, double alpha) {
  double t0 = 0;
  double t1 = catmulromT(t0,alpha,p0,p1);
  double t2 = catmulromT(t1,alpha,p1,p2);
  double t3 = catmulromT(t2,alpha,p2,p3);
  t = (t2-t1)*t+t1;
  Vector a1 = divi(t1-t,t1-t0)*p0 + divi(t-t0,t1-t0)*p1;
  Vector a2 = divi(t2-t,t2-t1)*p1 + divi(t-t1,t2-t1)*p2;
  Vector a3 = divi(t3-t,t3-t2)*p2 + divi(t-t2,t3-t2)*p3;
  Vector b1 = divi(t2-t,t2-t0)*a1 + divi(t-t0,t2-t0)*a2;
  Vector b2 = divi(t3-t,t3-t1)*a2 + divi(t-t1,t3-t1)*a3;
  return divi(t2-t,t2-t1)*b1+divi(t-t1,t2-t1)*b2;
}

Vector splineAt(GameObject *o,  double slider, Matrix *rotation) {
  static Array<unsigned int> points; points.clear();
  collectSection(o, &points, GAMEOBJECT_POINT);
  if (points.empty()) return Vector(0,0,0);
  double s = slider;
  if (s >= points.size()) s = points.size()-0.001f;
  if (s < 0) s = 0;
  int i2 = (int)floor(s);
  float between = s - (float)i2;
  int i0 = i2;
  int i1 = (i2+1) % points.size();
  Vector p0 = gameObjects[points[i0]]->getRenderTransform() * Vector(0,0,0);
  Vector p1 = gameObjects[points[i1]]->getRenderTransform() * Vector(0,0,0);
  if (p0.w != 0) {p0 /= p0.w; p0.w = 1;}
  if (p1.w != 0) {p1 /= p1.w; p1.w = 1;}
  Vector p;
  if (o->interpolationType == INTERPOLATION_LINEAR || o->interpolationType == INTERPOLATION_STEP) {
    p = (p1 - p0) * between + p0;
  }
  if (o->interpolationType == INTERPOLATION_CATMULROM) {
    int im = o->isLooping ? ((i2 + points.size()-1) % points.size()) : ((i2==0) ? 0 : i2 - 1);
    int ip = o->isLooping ? ((i2 + 2) % points.size()) : (((i2 + 2) > points.size()-1) ? points.size()-1 : i2 + 2);
    Vector pm = gameObjects[points[im]]->getRenderTransform() * Vector(0,0,0);
    Vector pp = gameObjects[points[ip]]->getRenderTransform() * Vector(0,0,0);
    if (pm.w != 0) {pm /= pm.w; pm.w = 1;}
    if (pp.w != 0) {pp /= pp.w; pp.w = 1;}
    double alpha = o->interpolationParam;
    p = catmulrom(pm,p0,p1,pp,between,alpha);
  }

  if (rotation != NULL) {
    Vector qv0 = gameObjects[points[i0]]->additionalRotation * PI / 180.0;
    Vector qv1 = gameObjects[points[i1]]->additionalRotation * PI / 180.0;
    Vector qvd = qv1-qv0;
    Quaternion q;
    q.yawPitchRoll(qvd.x*between+qv0.x,qvd.y*between+qv0.y,qvd.z*between+qv0.z); // no better idea for that,yet
    *rotation = normalize(q).rotationMatrix();
  }

  return p;
}

void getSpline(class GameObject *o, double slider, class Vector *position, class Matrix *rotation) {
  Matrix r2;
  Vector p = splineAt(o,slider,&r2);     
  Vector p_ = splineAt(o,slider+0.01,NULL);     
  Vector _p = splineAt(o,slider-0.01,NULL);     
  Vector n = length(p_-p)>0.0001?p_-p:p-_p;
  Matrix r = rotationMatrixFromNormal(normalize(n))*r2;
  Vector x = normalize(r.getColumn(0));
  Vector y = normalize(r.getColumn(1));
  Vector z = normalize(r.getColumn(2));
  Vector x2 = x;
  Vector y2 = z;
  Vector z2 = y * -1;
  x2.w = y2.w = z2.w = 0;
  r.setColumn(0,x2);
  r.setColumn(1,y2);
  r.setColumn(2,z2);
  if (rotation != NULL) *rotation = r;
  if (position != NULL) *position = p;
}

Hit *splinePart(class GameObject *o, const Vector &p0, const Vector &p1, int index, double t0, double t1) {
  glVertex3dv(&p0.x);
  glVertex3dv(&p1.x);
  Hit *p = addScreenSpaceLineMouseHit(o, HITTESTTOOL_SPLINEPART, p0, p1, o->lineWidth);
  if (p != NULL) {
    p->index = index;                                                                                                                                                                                                               
    p->t0 = t0;
    p->t1 = t1;
    p->p0 = p0;
    p->p1 = p1;
  }
  return p;
}

void paintPath(class GameObject *o, bool showModificators) {
  UNUSED(showModificators);
  static Array<unsigned int> points; points.clear();
  collectSection(o, &points, GAMEOBJECT_POINT);
  if (points.size()<2) return;
  glLineWidth(o->lineWidth);
  glBegin(GL_LINES);
  unsigned int col = o->color;
  glColor4f((col & 255)/255.f,((col>>8) & 255)/255.f,((col>>16) & 255)/255.f,((col>>24) & 255)/255.f);
  for (int i = 0; i < (points.size()-1 + (o->isLooping ? 1 : 0)); i++) {
    int i0 = i;
    int i1 = (i+1) % points.size();
    Vector p0 = gameObjects[points[i0]]->getRenderTransform() * Vector(0,0,0);
    Vector p1 = gameObjects[points[i1]]->getRenderTransform() * Vector(0,0,0);
    if (p0.w != 0) {p0 /= p0.w; p0.w = 1;}
    if (p1.w != 0) {p1 /= p1.w; p1.w = 1;}
    if (o->interpolationType == INTERPOLATION_LINEAR || o->interpolationType == INTERPOLATION_STEP) {
      splinePart(o,p0,p1,i,0,1);
    }
    if (o->interpolationType == INTERPOLATION_CATMULROM) {
      int im = o->isLooping ? ((i + points.size()-1) % points.size()) : ((i==0) ? 0 : i - 1);
      int ip = o->isLooping ? ((i + 2) % points.size()) : (((i + 2) > points.size()-1) ? points.size()-1 : i + 2);
      Vector pm = gameObjects[points[im]]->getRenderTransform() * Vector(0,0,0);
      Vector pp = gameObjects[points[ip]]->getRenderTransform() * Vector(0,0,0);
      if (pm.w != 0) {pm /= pm.w; pm.w = 1;}
      if (pp.w != 0) {pp /= pp.w; pp.w = 1;}
      int steps = 10;
      double alpha = o->interpolationParam;
      for (int j = 0; j < steps; j++) {
        double t0 = (double)j / steps;
        double t1 = (double)(j+1) / steps;
        Vector l0 = catmulrom(pm,p0,p1,pp,t0,alpha);
        Vector l1 = catmulrom(pm,p0,p1,pp,t1,alpha);
        splinePart(o,l0,l1,i,t0,t1);
      }
    }
  }
  glEnd();
  glLineWidth(1);

  if (points.size()>0) {
    Vector p; Matrix r; getSpline(o, o->sliderPos, &p, &r);
 
    glPointSize(o->lineWidth+2);
    glBegin(GL_POINTS);
    glColor4f(0,0,0,1);
    glVertex3dv(&p.x);
    glEnd();
    glPointSize(1);

    glBegin(GL_LINES);
    double l = 1;
    Vector p2;
    glColor4f(1,0,0,1); p2 = p + (r * Vector(1,0,0))*l; glVertex3dv(&p.x); glVertex3dv(&p2.x);
    glColor4f(0,1,0,1); p2 = p + (r * Vector(0,1,0))*l; glVertex3dv(&p.x); glVertex3dv(&p2.x);
    glColor4f(0,0,1,1); p2 = p + (r * Vector(0,0,1))*l; glVertex3dv(&p.x); glVertex3dv(&p2.x);
    glEnd();
  }
}

Vector heightMapVertex(int index, unsigned int *heights, int width, int height) {
  index += height * width;
  int ix = index % width;
  int iy = (index / width) % height;
  unsigned int rgba = heights[ix+iy*width];
  int r = (rgba) & 255;
  int g = (rgba>>8) & 255;
  int b = (rgba>>16) & 255;
  int a = (rgba>>24) & 255;
  double h = (double)(r+g+b)/255.0/3.0+(double)a/255.0/256.0;
  return Vector((double)ix/(width-1)*2-1,h,(double)iy/(height-1)*2-1);
}

bool heightMapIsIndexBorder(int index,int width,int height) {
  index += width * height;
  int ix = index % width;
  int iy = (index / width) % height;
  return (ix == 0)||(iy == 0)||(ix == width-1)||(iy == height-1);
}

void heightMapVertex2(Vector *toUpdate, int completeIndex, int failIndex, unsigned int *heights, int width, int height, unsigned char *pointCount) {

  completeIndex += height * width;
  failIndex += height * width;

  int cx = completeIndex % width;
  int cy = (completeIndex / width) % height;

  int fx = failIndex % width;
  int fy = (failIndex / width) % height;

  int dx = fx-cx;
  int dy = fy-cy;

  int ix = cx + dx * 2;
  int iy = cy + dy * 2;
  if (ix < 0) ix = 0;
  if (iy < 0) iy = 0;
  if (ix > width-1) ix = width-1;
  if (iy > height-1) iy = height-1;

  ix = (ix + width) % width;
  iy = (iy + height) % height;

  int newIndex = ix + iy * width;
  if (pointCount[newIndex]!=4) return;

  toUpdate->y = (heightMapVertex(completeIndex, heights, width, height).y + heightMapVertex(newIndex, heights, width, height).y )*0.5;
}

int heightMapIndex(int i10, int i00, int i01, int i11, int cx, int cy, int width, int height) {
  int ix00 = (i00+width) % width;
  int iy00 = (i00 / width+height) % height;
  int ix01 = (i01+width) % width;
  int iy01 = (i01 / width+height) % height;
  int ix11 = (i11+width) % width;
  int iy11 = (i11 / width+height) % height;
  int ix10 = (i10+width) % width;
  int iy10 = (i10 / width+height) % height;
  if (abs(ix10-ix00)==1) return i00;
  if (abs(iy01-iy00)==1) return i00;
  int newX = (ix10-ix00)*cx/2+ix00;
  int newY = (iy01-iy00)*cy/2+iy00;
  return newX + newY*width;
}

void paintHeightmap(class GameObject *o, bool showModificators) {

  exportObject(o);

  if (!isInViewport(o)) return;
  FileBinding *b = getBindingById(o->fileBindingId);
  if (b == NULL || b->textureIds.empty()) {paintTrigger(o,showModificators); return;}
  int width = glGetTextureWidth(b->textureIds[0]);
  int height = glGetTextureHeight(b->textureIds[0]);
  unsigned int *heights = glGetTexturePointer(b->textureIds[0]);
  if (heights == NULL || width < 2 || height < 2 || justDrawBoundingBoxes) {paintTrigger(o,showModificators); return;}

  Matrix cameraMatrix; glGetDoublev(GL_MODELVIEW_MATRIX, cameraMatrix.m);

  glPushMatrix();

  Matrix m = o->getRenderTransform();
  Vector minB = b->boundingBoxMin;
  Vector maxB = b->boundingBoxMax;

  glMultMatrixd(m.m);
  if (twoSidedOn) glDisable(GL_CULL_FACE); else glEnable(GL_CULL_FACE);  if (b->forceTwoSided) glDisable(GL_CULL_FACE);

  Matrix modelView; glGetDoublev(GL_MODELVIEW_MATRIX, modelView.m);
  Matrix projection; glGetDoublev(GL_PROJECTION_MATRIX, projection.m);
  Matrix m2 = projection * modelView;

  Vector camPos = inverse(cameraMatrix).getColumn(3);
  if (camPos.w != 0) {camPos / camPos.w; camPos.w = 1;}
  double f = b->doubleParameter;
  static Array<unsigned int> recurse; recurse.clear();
  static Array<unsigned int> polys; polys.clear();
  static Array<unsigned char> pointCount; pointCount.resize(width*height); memset(&pointCount[0],0,pointCount.size()*sizeof(unsigned char));
  recurse.push_back(1*(width-1)*1+0*(height-1)*width);
  recurse.push_back(0*(width-1)*1+0*(height-1)*width);
  recurse.push_back(0*(width-1)*1+1*(height-1)*width);
  recurse.push_back(1*(width-1)*1+1*(height-1)*width);
  bool fixBorders = b->boolParameter;

  while(!recurse.empty()) {

    unsigned int i11 = recurse.back(); recurse.pop_back();
    unsigned int i01 = recurse.back(); recurse.pop_back();
    unsigned int i00 = recurse.back(); recurse.pop_back();
    unsigned int i10 = recurse.back(); recurse.pop_back();

    if ((i11==i00)||(i01==i00)|(i10==i00)) continue;

    Vector p00 = heightMapVertex(i00,heights,width,height);
    Vector p10 = heightMapVertex(i10,heights,width,height);
    Vector p01 = heightMapVertex(i01,heights,width,height);
    Vector p11 = heightMapVertex(i11,heights,width,height);

    Vector t00 = m2 * p00;
    Vector t10 = m2 * p10;
    Vector t01 = m2 * p01;
    Vector t11 = m2 * p11;

    int allOutOnSide = 1|2|4|8|16|32;
    double guard = 1.01;
    if (t00.x>=-t00.w*guard||t01.x>=-t01.w*guard||t10.x>=-t10.w*guard||t11.x>=-t11.w*guard) allOutOnSide &= ~1;
    if (t00.y>=-t00.w*guard||t01.y>=-t01.w*guard||t10.y>=-t10.w*guard||t11.y>=-t11.w*guard) allOutOnSide &= ~2;
    if (t00.z>=-t00.w*guard||t01.z>=-t01.w*guard||t10.z>=-t10.w*guard||t11.z>=-t11.w*guard) allOutOnSide &= ~4;
    if (t00.x<=t00.w*guard||t01.x<=t01.w*guard||t10.x<=t10.w*guard||t11.x<=t11.w*guard) allOutOnSide &= ~8;
    if (t00.y<=t00.w*guard||t01.y<=t01.w*guard||t10.y<=t10.w*guard||t11.y<=t11.w*guard) allOutOnSide &= ~16;
    if (t00.z<=t00.w*guard||t01.z<=t01.w*guard||t10.z<=t10.w*guard||t11.z<=t11.w*guard) allOutOnSide &= ~32;
    if (allOutOnSide!=0) continue;

    if (t00.w != 0) {t00 /= t00.w; t00.w = 1;}
    if (t10.w != 0) {t10 /= t10.w; t10.w = 1;}
    if (t01.w != 0) {t01 /= t01.w; t01.w = 1;}
    if (t11.w != 0) {t11 /= t11.w; t11.w = 1;}

    Vector minp=t00,maxp=t00;
    if (t01.x<minp.x) minp.x = t01.x;
    if (t01.y<minp.y) minp.y = t01.y;
    if (t10.x<minp.x) minp.x = t10.x;
    if (t10.y<minp.y) minp.y = t10.y;
    if (t11.x<minp.x) minp.x = t11.x;
    if (t11.y<minp.y) minp.y = t11.y;
    if (t01.x>maxp.x) maxp.x = t01.x;
    if (t01.y>maxp.y) maxp.y = t01.y;
    if (t10.x>maxp.x) maxp.x = t10.x;
    if (t10.y>maxp.y) maxp.y = t10.y;
    if (t11.x>maxp.x) maxp.x = t11.x;
    if (t11.y>maxp.y) maxp.y = t11.y;
    double size = length((maxp-minp).xy());

    bool isLowestLevel = heightMapIndex(i10,i00,i01,i11,1,0,width,height)==i00 || heightMapIndex(i10,i00,i01,i11,0,1,width,height)==i00;
    bool forceRecurse = false;
    if (fixBorders) {
      bool isBorder = heightMapIsIndexBorder(i00,width,height) || heightMapIsIndexBorder(i01,width,height) || heightMapIsIndexBorder(i10,width,height) || heightMapIsIndexBorder(i11,width,height);
      if (isBorder&& !isLowestLevel) forceRecurse = true;
    }

    if ((size>f && !isLowestLevel)||(forceRecurse && !isLowestLevel)) {
      recurse.push_back(heightMapIndex(i10,i00,i01,i11,1,0,width,height));
      recurse.push_back(heightMapIndex(i10,i00,i01,i11,0,0,width,height));
      recurse.push_back(heightMapIndex(i10,i00,i01,i11,0,1,width,height));
      recurse.push_back(heightMapIndex(i10,i00,i01,i11,1,1,width,height));

      recurse.push_back(heightMapIndex(i10,i00,i01,i11,2,0,width,height));
      recurse.push_back(heightMapIndex(i10,i00,i01,i11,1,0,width,height));
      recurse.push_back(heightMapIndex(i10,i00,i01,i11,1,1,width,height));
      recurse.push_back(heightMapIndex(i10,i00,i01,i11,2,1,width,height));

      recurse.push_back(heightMapIndex(i10,i00,i01,i11,2,1,width,height));
      recurse.push_back(heightMapIndex(i10,i00,i01,i11,1,1,width,height));
      recurse.push_back(heightMapIndex(i10,i00,i01,i11,1,2,width,height));
      recurse.push_back(heightMapIndex(i10,i00,i01,i11,2,2,width,height));

      recurse.push_back(heightMapIndex(i10,i00,i01,i11,1,1,width,height));
      recurse.push_back(heightMapIndex(i10,i00,i01,i11,0,1,width,height));
      recurse.push_back(heightMapIndex(i10,i00,i01,i11,0,2,width,height));
      recurse.push_back(heightMapIndex(i10,i00,i01,i11,1,2,width,height));
    } else {
      polys.push_back(i10);
      polys.push_back(i00);
      polys.push_back(i01);
      polys.push_back(i11);
      pointCount[i10]++;
      pointCount[i00]++;
      pointCount[i01]++;
      pointCount[i11]++;
      if (polys.size()>MAXMEMPERALLOC/sizeof(unsigned int)) break;
    }
  }

  if (b->textureIds.size()>1) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,b->textureIds[1]);
  }

  Vector collisionPoint;
  static Array<BakedVertex> tris; tris.resize(3);

  glBegin(GL_QUADS);
  glColor4ubv((GLubyte*)&o->color);
  for (int i = 0; i < polys.size()/4; i++) {
    int i10 = polys[i*4+0];
    int i00 = polys[i*4+1];
    int i01 = polys[i*4+2];
    int i11 = polys[i*4+3];
    Vector p10 = heightMapVertex(i10,heights,width,height);
    Vector p00 = heightMapVertex(i00,heights,width,height);
    Vector p01 = heightMapVertex(i01,heights,width,height);
    Vector p11 = heightMapVertex(i11,heights,width,height);
    Vector n1 = p11-p00;
    Vector n2 = p10-p01;
    // there are still some tiny seams here and there..
    if (pointCount[i01]==4&&pointCount[i00]==2) {heightMapVertex2(&p00,i01,i00,heights,width,height,&pointCount[0]);}
    if (pointCount[i10]==4&&pointCount[i00]==2) {heightMapVertex2(&p00,i10,i00,heights,width,height,&pointCount[0]);}
    if (pointCount[i00]==4&&pointCount[i01]==2) {heightMapVertex2(&p01,i00,i01,heights,width,height,&pointCount[0]);}
    if (pointCount[i11]==4&&pointCount[i01]==2) {heightMapVertex2(&p01,i11,i01,heights,width,height,&pointCount[0]);}
    if (pointCount[i00]==4&&pointCount[i10]==2) {heightMapVertex2(&p10,i00,i10,heights,width,height,&pointCount[0]);}
    if (pointCount[i11]==4&&pointCount[i10]==2) {heightMapVertex2(&p10,i11,i10,heights,width,height,&pointCount[0]);}
    if (pointCount[i01]==4&&pointCount[i11]==2) {heightMapVertex2(&p11,i01,i11,heights,width,height,&pointCount[0]);}
    if (pointCount[i10]==4&&pointCount[i11]==2) {heightMapVertex2(&p11,i10,i11,heights,width,height,&pointCount[0]);}
    Vector n = normalize(cross(n1,n2));
    glNormal3dv(&n.x);
    glTexCoord2f((float)(i10%width)/width,(float)(i10/width)/height); glVertex3dv(&p10.x);
    glTexCoord2f((float)(i00%width)/width,(float)(i00/width)/height); glVertex3dv(&p00.x);
    glTexCoord2f((float)(i01%width)/width,(float)(i01/width)/height); glVertex3dv(&p01.x);
    glTexCoord2f((float)(i11%width)/width,(float)(i11/width)/height); glVertex3dv(&p11.x);
   
    if (doHitTest) {
      if (triangleHitTest(p10, p00, p01,collisionPoint)||triangleHitTest(p01, p11, p10,collisionPoint)) { // this doesn't work, because there are heights below the "quad"
        tris[0].x = p10.x; tris[0].y = p10.y; tris[0].z = p10.z;
        tris[1].x = p00.x; tris[1].y = p00.y; tris[1].z = p00.z;
        tris[2].x = p01.x; tris[2].y = p01.y; tris[2].z = p01.z;
        addHitTestTriangles(o, HITTESTTOOL_NONE, &tris, minB, maxB);
        tris[0].x = p01.x; tris[0].y = p01.y; tris[0].z = p01.z;
        tris[1].x = p11.x; tris[1].y = p11.y; tris[1].z = p11.z;
        tris[2].x = p10.x; tris[2].y = p10.y; tris[2].z = p10.z;
        addHitTestTriangles(o, HITTESTTOOL_NONE, &tris, minB, maxB);
/*
        int sx = i00 % width;
        int sy = (i00/width) % height;
        int ex = i11 % width;
        int ey = (i11/width) % height;
        for (int y = sy; y < ey-1; y++) {
          for (int x = sx; x < ex-1; x++) {
            int i00 = x + y * width;
            int i10 = (x+1) + y * width;
            int i11 = (x+1) + (y+1) * width;
            int i01 = x + (y+1) * width;
            Vector p10 = heightMapVertex(i10,heights,width,height);
            Vector p00 = heightMapVertex(i00,heights,width,height);
            Vector p01 = heightMapVertex(i01,heights,width,height);
            Vector p11 = heightMapVertex(i11,heights,width,height);
            if (triangleHitTest(p10, p00, p01,collisionPoint)) {
              tris[0].x = p10.x; tris[0].y = p10.y; tris[0].z = p10.z;
              tris[1].x = p00.x; tris[1].y = p00.y; tris[1].z = p00.z;
              tris[2].x = p01.x; tris[2].y = p01.y; tris[2].z = p01.z;
              addHitTestTriangles(o, HITTESTTOOL_NONE, &tris, minB, maxB);
            }
            if (triangleHitTest(p01, p11, p10,collisionPoint)) {
              tris[0].x = p01.x; tris[0].y = p01.y; tris[0].z = p01.z;
              tris[1].x = p11.x; tris[1].y = p11.y; tris[1].z = p11.z;
              tris[2].x = p10.x; tris[2].y = p10.y; tris[2].z = p10.z;
              addHitTestTriangles(o, HITTESTTOOL_NONE, &tris, minB, maxB);
            }
          }
        }
*/
      }
    }
  }
  glEnd();

  glEnable(GL_CULL_FACE);
  glDisable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,0);

  glTranslatef(o->anchor.x,o->anchor.y,o->anchor.z);

  if (showModificators) {
    Vector minB2 = minB - o->anchor;
    Vector maxB2 = maxB - o->anchor;
    addHitTestScaleGizmo(o,minB2,maxB2);
    addHitTestRotateGizmo(o,minB2,maxB2);
    addHitTestMoveGizmo2(cameraMatrix,o,minB2,maxB2);
  }

  glPopMatrix();
}

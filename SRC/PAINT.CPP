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
              h.normal = normalize(inverse(transpose(inverse(cameraMatrix)*modelView))*n);
              hits.push_back(h);
            }
            Hit h2;
            h2.depth = length(collisionPoint - mouseHitStart);
            h2.point = collisionPoint;
            h2.object = o;
            h2.toolId = HITTESTTOOL_NONE;
            h2.normal = normalize(inverse(transpose(inverse(cameraMatrix)*modelView))*n);
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
    p->normal = normalize(p1-p0);
  }
  return p;
}

void paintPath(class GameObject *o, bool showModificators) {
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
    Vector p;
    glPointSize(o->lineWidth+2);
    float s = o->sliderPos;
    if (s < 0) s = 0;
    if (s >= points.size()) s = points.size()-0.001f;
    int i2 = (int)floor(s);
    float between = s - (float)i2;
    int i0 = i2;
    int i1 = (i2+1) % points.size();
    Vector p0 = gameObjects[points[i0]]->getRenderTransform() * Vector(0,0,0);
    Vector p1 = gameObjects[points[i1]]->getRenderTransform() * Vector(0,0,0);
    if (p0.w != 0) {p0 /= p0.w; p0.w = 1;}
    if (p1.w != 0) {p1 /= p1.w; p1.w = 1;}
    if (o->interpolationType == INTERPOLATION_LINEAR || o->interpolationType == INTERPOLATION_STEP) {
      p = (p1 - p0) * between + p0;
    }
    if (o->interpolationType == INTERPOLATION_CATMULROM) {
      int im = o->isLooping ? ((i2 + points.size()-1) % points.size()) : ((i2==0) ? 0 : i2 - 1);
      int ip = o->isLooping ? ((i2 + 2) % points.size()) : (((i2 + 2) > points.size()-1) ? points.size()-1 : i + 2);
      Vector pm = gameObjects[points[im]]->getRenderTransform() * Vector(0,0,0);
      Vector pp = gameObjects[points[ip]]->getRenderTransform() * Vector(0,0,0);
      if (pm.w != 0) {pm /= pm.w; pm.w = 1;}
      if (pp.w != 0) {pp /= pp.w; pp.w = 1;}
      double alpha = o->interpolationParam;
      p = catmulrom(pm,p0,p1,pp,between,alpha);
    }
  
    glBegin(GL_POINTS);
    glColor4f(0,0,0,1);
    glVertex3dv(&p.x);
    glEnd();
    glPointSize(1);
  }
}

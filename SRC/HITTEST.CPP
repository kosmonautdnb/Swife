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
#include "HITTEST.HPP"
#include "CONFIG.HPP"
#include "TOOLBAR.HPP"
#include "GAMEOBJ.HPP"
#include "RENDER.HPP"
#include "STL/MATRIX.HPP"
#include <math.h>

Array<Hit> hits;
Vector mouseHitStart, mouseHitEnd;
extern bool doHitTest = false;
Matrix hitTestInverseModelView;
bool startTool = false;
int activatedTool = HITTESTTOOL_NONE;
GameObject *activatedToolObjectBase = NULL;
bool moveToolWorldSpace = false;

void prepareMouseHitTest(int mouseX, int mouseY) {
  doHitTest = false;
  hits.clear();
  mouseHitStart.set(0,0,0);
  mouseHitEnd.set(0,0,0);
  if (mouseX < 0 || mouseY < 0) return;
  int viewport[4]; glGetIntegerv(GL_VIEWPORT,viewport);
  if (viewport[2]==0||viewport[3]==0) return;
  viewport[1] = main_window_yres - viewport[1] - viewport[3];
  viewport[2] = viewport[0] + viewport[2];
  viewport[3] = viewport[1] + viewport[3];
  double mx = (double)(mouseX - viewport[0])/(viewport[2]-viewport[0])*2.0-1.0;
  double my = (double)(mouseY - viewport[1])/(viewport[3]-viewport[1])*2.0-1.0;
  if (mx < -1 || mx > 1 || my < -1 || my > 1) return;
  my = -my;
  Matrix projection; glGetDoublev(GL_PROJECTION_MATRIX, projection.m);
  Matrix modelView; glGetDoublev(GL_MODELVIEW_MATRIX, modelView.m);
  Matrix invProjection = inverse(projection);
  Matrix invModelView = inverse(modelView);
  hitTestInverseModelView = invModelView;
  Vector p0(mx,my,0);
  Vector p1(mx,my,1);
  p0 = invModelView * invProjection * p0;
  p1 = invModelView * invProjection * p1;
  if (p0.w != 0) {p0 /= p0.w; p0.w = 1;}
  if (p1.w != 0) {p1 /= p1.w; p1.w = 1;}
  mouseHitStart = p0;
  mouseHitEnd = p1;
  doHitTest = true;
}

bool hitTestLinePoint(const double fDst1, const double fDst2,const Vector &p1, const Vector &p2, Vector &hit) {
  if ((fDst1*fDst2)>=0.0) return false;
  if (fDst1==fDst2) return false;
  hit = p1 + (p2-p1)*(-fDst1/(fDst2-fDst1));
  return true;
}

bool hitTestInBox(const Vector &hit, const Vector &b1, const Vector &b2, const int axis) {
  switch(axis) {
  case 1: {if (hit.z>=b1.z&&hit.z<b2.z&&hit.y>=b1.y&&hit.y<b2.y) return true;} break;
  case 2: {if (hit.z>=b1.z&&hit.z<b2.z&&hit.x>=b1.x&&hit.x<b2.x) return true;} break;
  case 3: {if (hit.x>=b1.x&&hit.x<b2.x&&hit.y>=b1.y&&hit.y<b2.y) return true;} break;
  }
  return false;
}

bool hitTestCubeAndLine(const Matrix &cube, const Vector &p0,const Vector &p1, Array<Vector> &collisionPoints, bool checkIfInside = true) {
  Matrix im1 = inverse(cube);
  const Vector l1 = im1 * p0;
  const Vector l2 = im1 * p1;
  const Vector b1(-1,-1,-1);
  const Vector b2(1,1,1);
  if (l1.x<b1.x && l2.x<b1.x) return false;
  if (l1.y<b1.y && l2.y<b1.y) return false;
  if (l1.z<b1.z && l2.z<b1.z) return false;
  if (l1.x>=b2.x && l2.x>=b2.x) return false;
  if (l1.y>=b2.y && l2.y>=b2.y) return false;
  if (l1.z>=b2.z && l2.z>=b2.z) return false;
   
  Vector collisionPoint;
  bool ret = false;  
  if (checkIfInside && l1.x>=b1.x && l1.y>=b1.y && l1.z>=b1.z && l1.x<b2.x && l1.y<b2.y && l1.z<b2.z) {collisionPoints.push_back(cube*l1); ret = true;}
  if (hitTestLinePoint(l1.x-b1.x,l2.x-b1.x,l1,l2,collisionPoint)&&hitTestInBox(collisionPoint,b1,b2,1)) {collisionPoints.push_back(cube*collisionPoint); ret = true;}
  if (hitTestLinePoint(l1.y-b1.y,l2.y-b1.y,l1,l2,collisionPoint)&&hitTestInBox(collisionPoint,b1,b2,2)) {collisionPoints.push_back(cube*collisionPoint); ret = true;}
  if (hitTestLinePoint(l1.z-b1.z,l2.z-b1.z,l1,l2,collisionPoint)&&hitTestInBox(collisionPoint,b1,b2,3)) {collisionPoints.push_back(cube*collisionPoint); ret = true;}
  if (hitTestLinePoint(l1.x-b2.x,l2.x-b2.x,l1,l2,collisionPoint)&&hitTestInBox(collisionPoint,b1,b2,1)) {collisionPoints.push_back(cube*collisionPoint); ret = true;}
  if (hitTestLinePoint(l1.y-b2.y,l2.y-b2.y,l1,l2,collisionPoint)&&hitTestInBox(collisionPoint,b1,b2,2)) {collisionPoints.push_back(cube*collisionPoint); ret = true;}
  if (hitTestLinePoint(l1.z-b2.z,l2.z-b2.z,l1,l2,collisionPoint)&&hitTestInBox(collisionPoint,b1,b2,3)) {collisionPoints.push_back(cube*collisionPoint); ret = true;}
  return ret;
}

double hitTestTrianglesSignedVolume(const Vector &a, const Vector &b, const Vector &c, const Vector &d) {return (1.0/6.0)*dot(cross(b-a,c-a),d-a);}

bool hitTestTrianglesCollided(const Vector &l0,const Vector &l1, const Vector &t0, const Vector &t1, const Vector &t2) {
  const double s0 = hitTestTrianglesSignedVolume(l0,t0,t1,t2);
  const double s1 = hitTestTrianglesSignedVolume(l1,t0,t1,t2);
  if (s0*s1>=0) return false;
  const double a0 = hitTestTrianglesSignedVolume(l0,l1,t0,t1);
  const double a1 = hitTestTrianglesSignedVolume(l0,l1,t1,t2);
  if (a0*a1<0) return false;
  const double a2 = hitTestTrianglesSignedVolume(l0,l1,t2,t0);
  if (a1*a2<0) return false;
  return true;
}

bool hitTestTrianglesCollisionPoint(const Vector &l0,const Vector &l1, const Vector &t0, const Vector &t1, const Vector &t2, Vector &collisionPoint) {
  Vector e1 = t1-t0;
  Vector e2 = t2-t0;
  Vector rd = normalize(l1-l0); // ray dir
  Vector n = cross(e1,e2);
  double det = -dot(rd,n);
  double invdet = det!=0?1.0/det:1.0;
  Vector a0 = l0 - t0;
  Vector da0 = cross(a0,rd);
  double u = dot(e2,da0)*invdet;
  double v = -dot(e1,da0)*invdet;
  double t = dot(a0,n)*invdet;
  bool hit = fabs(det)>=1e-6&&t>=0.0&&u>=0.0&&v>=0.0&&(u+v)<=1.0;
  if (hit) collisionPoint = l0 + t * rd;
  return hit;
}

void addHitTestCube(class GameObject *o, int toolId, const Vector &minB, const Vector &maxB) {
  if (!doHitTest) return;

  Matrix modelView; glGetDoublev(GL_MODELVIEW_MATRIX, modelView.m);
  Vector center = (minB+maxB) * 0.5;
  Matrix scale;  scale.translate(center); scale.scale3((maxB-minB)*0.5);
  Array<Vector> collisionPoints;
  if (hitTestCubeAndLine(hitTestInverseModelView*modelView*scale, mouseHitStart, mouseHitEnd, collisionPoints, true)) {
    for (int i = 0; i < collisionPoints.size(); i++) {
      Hit h;
      h.depth = length(collisionPoints[i] - mouseHitStart);
      h.point = collisionPoints[i];
      h.object = o;
      h.toolId = toolId;
      hits.push_back(h);
    }
  }
}


bool isBackFace(const Matrix *m, const Matrix *invModelView, const Vector *v0,const Vector *v1,const Vector *v2) {
  bool ret = false;
  Vector a = *v1 - *v0;
  Vector b = *v2 - *v0;
  Vector n = cross(a,b);
  // check if orthographic projection by x,y,z not influencing w
  if (m->m[0*4+3] == 0 && m->m[1*4+3] == 0 && m->m[2*4+3] == 0) {
    ret = dot(Vector(invModelView->m[4*2+0],invModelView->m[4*2+1],invModelView->m[4*2+2]),n) < 0;
  } else {
    Vector v = *v0 - Vector(invModelView->m[4*3+0],invModelView->m[4*3+1],invModelView->m[4*3+2]);
    ret = dot(v,n) > 0;
  }
  return ret;
}

void addHitTestTriangles(class GameObject *o, int toolId, Array<BakedVertex> *triangles, const Vector &minB, const Vector &maxB) {
  if (!doHitTest) return;

  Array<Vector> collisionPoints;
  Matrix modelView; glGetDoublev(GL_MODELVIEW_MATRIX, modelView.m); 
  Vector center = (minB+maxB) * 0.5; Matrix scale;  scale.translate(center); scale.scale3((maxB-minB)*0.5);
  if (hitTestCubeAndLine(hitTestInverseModelView*modelView*scale, mouseHitStart, mouseHitEnd, collisionPoints, true)) {
    Vector t0,t1,t2,v0,v1,v2;
    Matrix m2 = hitTestInverseModelView*modelView;
    Matrix projection; glGetDoublev(GL_PROJECTION_MATRIX, projection.m); Matrix m3 = projection * modelView;
    Matrix invModelView = inverse(modelView);
    for (int i = 0; i < triangles->size(); i+=3) {
      BakedVertex *t = &(*triangles)[i];
      t0.set(t->x,t->y,t->z); t++;
      t1.set(t->x,t->y,t->z); t++;
      t2.set(t->x,t->y,t->z);      
      if (!twoSidedOn) {
        // this one may be not working fully, since triangles are not clipped on nearplane
        // backface by normal on the other hand may not work well with orthogonal matrices
        //v0 = m3 * t0;
        //v1 = m3 * t1;
        //v2 = m3 * t2;
        //if (v0.w != 0) v0 /= v0.w;
        //if (v1.w != 0) v1 /= v1.w;
        //if (v2.w != 0) v2 /= v2.w;
        //double dx0 = v1.x - v0.x;
        //double dy0 = v1.y - v0.y;
        //double dx1 = v2.x - v0.x;
        //double dy1 = v2.y - v0.y;
        //if ((dx0*dy1 - dy0*dx1) < 0) continue;
        if (isBackFace(&m3,&invModelView,&t0,&t1,&t2)) continue;
      }

      t0 = m2*t0;
      t1 = m2*t1;
      t2 = m2*t2;
      if (hitTestTrianglesCollided(mouseHitStart,mouseHitEnd, t0, t1, t2)) {
        Vector collisionPoint;
        if (hitTestTrianglesCollisionPoint(mouseHitStart,mouseHitEnd, t0, t1, t2, collisionPoint)) {
          Hit h;
          h.depth = length(collisionPoint - mouseHitStart);
          h.point = collisionPoint;
          h.object = o;
          h.toolId = toolId;
          hits.push_back(h);
        }
      }
    }
  }
}

Hit *getNearestHit(unsigned int *counter, double lastDepth) {
  Hit *ret = NULL;
  double currentHitDepth = 0;
  for (int i = 0; i < hits.size(); i++) {
    if (i == 0 || hits[i].depth < currentHitDepth) {
      currentHitDepth = hits[i].depth;
      ret = &hits[i];
    }
  }
  if (currentHitDepth != lastDepth) *counter = 0;
  if (ret != NULL) {
    int count = 0;
    for (int i2 = 0; i2 < hits.size(); i2++) {
      if (hits[i2].depth == currentHitDepth) {
        count++;
      }
    }
    int number = 0;
    for (int i3 = 0; i3 < hits.size(); i3++) {
      if (hits[i3].depth == currentHitDepth) {
        if (number == (*counter % count)) 
          ret = &hits[i3];
        number++;
      }
    }
  } else {
    *counter = 0;
  }
  return ret;
}

Hit *getNearestToolHit() {
  Hit *ret = NULL;
  double currentHitDepth = 0;
  bool first = true;
  for (int i = 0; i < hits.size(); i++) {
    if (hits[i].toolId != HITTESTTOOL_NONE) {
      if (first || hits[i].depth < currentHitDepth) {
        first = false;
        currentHitDepth = hits[i].depth;
        ret = &hits[i];
      }
    }
  }
  return ret;
}

#define CLIPNULL(__v0__,__v1__,__d__) ((__v1__)==0?((__v0__)==0?0:(__d__)):(__v0__)/(__v1__))
void addScreenSpaceLineMouseHit(class GameObject *o, int toolId, const Vector &p0, const Vector &p1, double lineWidth) {
  Matrix projection; glGetDoublev(GL_PROJECTION_MATRIX, projection.m);
  Matrix modelView; glGetDoublev(GL_MODELVIEW_MATRIX, modelView.m);
  Vector t0 = projection*modelView*p0;
  Vector t1 = projection*modelView*p1;
  bool t0Inside = t0.z >= -t0.w;
  bool t1Inside = t1.z >= -t1.w;
  if ((!t0Inside)&&(!t1Inside))
    return;
  if(t0Inside && (!t1Inside)) {
    double f = CLIPNULL((t0.w+t0.z) , ((t0.z-t1.z)+(t0.w-t1.w)), 1.0);
    t1 = (t1-t0)*f+t0;
  }
  if((!t0Inside) && t1Inside) {
    double f = CLIPNULL((t0.w+t0.z) , ((t0.z-t1.z)+(t0.w-t1.w)), 1.0);
    t0 = (t1-t0)*f+t0;
  }
  if (t0.w != 0) {t0/=t0.w;t0.w=1;}
  if (t1.w != 0) {t1/=t1.w;t1.w=1;}
  double dx = t1.x - t0.x;
  double dy = t1.y - t0.y;
  double nx = dx;
  double ny = dy;
  double nl = sqrt(nx*nx+ny*ny);
  if (nl == 0) return;
  nx /= nl; 
  ny /= nl;
  double tx = ny;
  double ty = -nx;

  int viewport[4]; glGetIntegerv(GL_VIEWPORT,viewport);
  if (viewport[2]==0||viewport[3]==0) return;
  viewport[1] = main_window_yres - viewport[1] - viewport[3];
  viewport[2] = viewport[0] + viewport[2];
  viewport[3] = viewport[1] + viewport[3];
  double mx = (double)(ImGui::GetMousePos().x - viewport[0])/(viewport[2]-viewport[0])*2.0-1.0;
  double my = (double)(ImGui::GetMousePos().y - viewport[1])/(viewport[3]-viewport[1])*2.0-1.0;
  my = -my;

  double mpx = mx - (t0.x+t1.x)*0.5;
  double mpy = my - (t0.y+t1.y)*0.5;
  double distX = fabs(tx*mpx*(viewport[2]-viewport[0])*0.5 + ty*mpy*(viewport[3]-viewport[1])*0.5);
  double distY = fabs(nx*mpx + ny*mpy);
  if (distX <= lineWidth && distY <= nl*0.5) {
    double f = distY / nl + 0.5;
    Vector collisionPoint = hitTestInverseModelView*inverse(projection)*((t1-t0)*f+t0);
    Hit h;
    h.depth = length(collisionPoint - mouseHitStart);
    h.point = collisionPoint;
    h.object = o;
    h.toolId = toolId;
    hits.push_back(h);
  }
}

void hitTestLine(class GameObject *o, int toolId, const Vector &p0, const Vector &p1, unsigned int color) {
  Matrix modelView; glGetDoublev(GL_MODELVIEW_MATRIX, modelView.m);
  Vector v0 = inverse(modelView) * p0;
  Vector v1 = inverse(modelView) * p1;
  double lineWidth = 4.f;
  if (activatedTool == toolId) {lineWidth = 8; color |= 0x00808080;}
  glLineWidth(lineWidth);
  glBegin(GL_LINES);
  glColor4ubv((GLubyte*)&color);
  glVertex3f(v0.x,v0.y,v0.z);
  glVertex3f(v1.x,v1.y,v1.z);
  glEnd();
  addScreenSpaceLineMouseHit(o,toolId,v0,v1,lineWidth);
  glLineWidth(1.f);
}

void addHitTestMoveGizmo(class GameObject *o, const Vector &minB, const Vector &maxB) {
  if (!o->selected || mode != MODE_MOVE) return;
  Matrix modelView; glGetDoublev(GL_MODELVIEW_MATRIX, modelView.m);
  Vector p = modelView * Vector(0,0,0);
  Vector xp = modelView * Vector(maxB.x,0,0);
  Vector yp = modelView * Vector(0,maxB.y,0);
  Vector zp = modelView * Vector(0,0,maxB.z);
  Vector xp3 = modelView * Vector(minB.x,0,0);
  Vector yp3 = modelView * Vector(0,minB.y,0);
  Vector zp3 = modelView * Vector(0,0,minB.z);
  Vector xp2 = normalize(modelView * Vector(maxB.x+1,0,0) - xp);
  Vector yp2 = normalize(modelView * Vector(0,maxB.y+1,0) - yp);
  Vector zp2 = normalize(modelView * Vector(0,0,maxB.z+1) - zp);
  float k = 1;
  hitTestLine(o,HITTESTTOOL_MOVE_PLUS_X,p,xp+xp2*k,0xff0000ff);
  hitTestLine(o,HITTESTTOOL_MOVE_PLUS_Y,p,yp+yp2*k,0xff00ff00);
  hitTestLine(o,HITTESTTOOL_MOVE_PLUS_Z,p,zp+zp2*k,0xffff0000);
  hitTestLine(o,HITTESTTOOL_MOVE_MINUS_X,p,xp3-xp2*k,0xff0000ff);
  hitTestLine(o,HITTESTTOOL_MOVE_MINUS_Y,p,yp3-yp2*k,0xff00ff00);
  hitTestLine(o,HITTESTTOOL_MOVE_MINUS_Z,p,zp3-zp2*k,0xffff0000);
}

void addHitTestScaleGizmo(class GameObject *o, const Vector &minB, const Vector &maxB) {
  if (!o->selected || mode != MODE_SCALE) return;
  Matrix modelView; glGetDoublev(GL_MODELVIEW_MATRIX, modelView.m);
  Vector xp = modelView * Vector(maxB.x,0,0);
  Vector yp = modelView * Vector(0,maxB.y,0);
  Vector zp = modelView * Vector(0,0,maxB.z);
  Vector xp3 = modelView * Vector(minB.x,0,0);
  Vector yp3 = modelView * Vector(0,minB.y,0);
  Vector zp3 = modelView * Vector(0,0,minB.z);
  Vector xp2 = normalize(modelView * Vector(maxB.x+1,0,0) - xp);
  Vector yp2 = normalize(modelView * Vector(0,maxB.y+1,0) - yp);
  Vector zp2 = normalize(modelView * Vector(0,0,maxB.z+1) - zp);
  float k = 1;
  hitTestLine(o,HITTESTTOOL_SCALE_PLUS_X,xp,xp+xp2*k,0xff0000ff);
  hitTestLine(o,HITTESTTOOL_SCALE_PLUS_Y,yp,yp+yp2*k,0xff00ff00);
  hitTestLine(o,HITTESTTOOL_SCALE_PLUS_Z,zp,zp+zp2*k,0xffff0000);
  hitTestLine(o,HITTESTTOOL_SCALE_MINUS_X,xp3,xp3-xp2*k,0xff0000ff);
  hitTestLine(o,HITTESTTOOL_SCALE_MINUS_Y,yp3,yp3-yp2*k,0xff00ff00);
  hitTestLine(o,HITTESTTOOL_SCALE_MINUS_Z,zp3,zp3-zp2*k,0xffff0000);
}

void addHitTestRotateGizmo(class GameObject *o, const Vector &minB, const Vector &maxB) {
  if (!o->selected || mode != MODE_ROTATE) return;
  Matrix modelView; glGetDoublev(GL_MODELVIEW_MATRIX, modelView.m);
  Vector xp = modelView * Vector(maxB.x,0,0);
  Vector yp = modelView * Vector(0,maxB.y,0);
  Vector zp = modelView * Vector(0,0,maxB.z);
  Vector xp3 = modelView * Vector(minB.x,0,0);
  Vector yp3 = modelView * Vector(0,minB.y,0);
  Vector zp3 = modelView * Vector(0,0,minB.z);
  Vector xp2 = normalize(modelView * Vector(maxB.x+1,0,0) - xp);
  Vector yp2 = normalize(modelView * Vector(0,maxB.y+1,0) - yp);
  Vector zp2 = normalize(modelView * Vector(0,0,maxB.z+1) - zp);
  float k = 1;
  float k2 = 1;
  hitTestLine(o,HITTESTTOOL_ROTATE_PLUS_X,xp+xp2*k+yp2*k2,xp+xp2*k-yp2*k2,0xff0000ff);
  hitTestLine(o,HITTESTTOOL_ROTATE_PLUS_Y,yp+yp2*k+zp2*k2,yp+yp2*k-zp2*k2,0xff00ff00);
  hitTestLine(o,HITTESTTOOL_ROTATE_PLUS_Z,zp+zp2*k+xp2*k2,zp+zp2*k-xp2*k2,0xffff0000);
  hitTestLine(o,HITTESTTOOL_ROTATE_MINUS_X,xp3-xp2*k+yp2*k2,xp3-xp2*k-yp2*k2,0xff0000ff);
  hitTestLine(o,HITTESTTOOL_ROTATE_MINUS_Y,yp3-yp2*k+zp2*k2,yp3-yp2*k-zp2*k2,0xff00ff00);
  hitTestLine(o,HITTESTTOOL_ROTATE_MINUS_Z,zp3-zp2*k+xp2*k2,zp3-zp2*k-xp2*k2,0xffff0000);
}

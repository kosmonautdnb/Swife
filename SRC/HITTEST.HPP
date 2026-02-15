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
#ifndef __HITTEST_HPP__
#define __HITTEST_HPP__

#include "STL/ARRAY.HPP"
#include "STL/VECTOR.HPP"
#include "SERIALZE.HPP"

enum TOOLID {
  HITTESTTOOL_NONE = 0,
  HITTESTTOOL_MOVE_PLUS_X = 1,
  HITTESTTOOL_MOVE_PLUS_Y = 2,
  HITTESTTOOL_MOVE_PLUS_Z = 3,
  HITTESTTOOL_MOVE_MINUS_X = 4,
  HITTESTTOOL_MOVE_MINUS_Y = 5,
  HITTESTTOOL_MOVE_MINUS_Z = 6,
  HITTESTTOOL_SCALE_PLUS_X = 7,
  HITTESTTOOL_SCALE_PLUS_Y = 8,
  HITTESTTOOL_SCALE_PLUS_Z = 9,
  HITTESTTOOL_SCALE_MINUS_X = 10,
  HITTESTTOOL_SCALE_MINUS_Y = 11,
  HITTESTTOOL_SCALE_MINUS_Z = 12,
  HITTESTTOOL_ROTATE_PLUS_X = 13,
  HITTESTTOOL_ROTATE_PLUS_Y = 14,
  HITTESTTOOL_ROTATE_PLUS_Z = 15,
  HITTESTTOOL_ROTATE_MINUS_X = 16,
  HITTESTTOOL_ROTATE_MINUS_Y = 17,
  HITTESTTOOL_ROTATE_MINUS_Z = 18
};

class Hit {
public:
  Hit() {
    object = NULL;
    toolId = 0;
    depth = 0;
    point.set(0,0,0);
  }
  class GameObject *object;
  int toolId;
  double depth;
  Vector point; 
};

extern Array<Hit> hits;
extern Vector mouseHitStart, mouseHitEnd;
extern bool doHitTest;
extern bool startTool;
extern int activatedTool;
extern bool moveToolWorldSpace;
extern class GameObject *activatedToolObjectBase;

void prepareMouseHitTest(int mouseX, int mouseY);

void addHitTestCube(class GameObject *o, int toolId, const Vector &minB, const Vector &maxB);
void addHitTestTriangles(class GameObject *o, int toolId, Array<BakedVertex> *triangles, const Vector &minB, const Vector &maxB);


Hit *getNearestHit(unsigned int *counter, double lastDepth);
Hit *getNearestToolHit();

void hitTestLine(class GameObject *o, int toolId, const Vector &p0, const Vector &p1, unsigned int color);

void addHitTestMoveGizmo(class GameObject *o, const Vector &minB, const Vector &maxB);
void addHitTestScaleGizmo(class GameObject *o, const Vector &minB, const Vector &maxB);
void addHitTestRotateGizmo(class GameObject *o, const Vector &minB, const Vector &maxB);

#endif //__HITTEST_HPP__

#ifndef __HITTEST_HPP__
#define __HITTEST_HPP__

#include "STL/ARRAY.HPP"
#include "STL/VECTOR.HPP"

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
extern class GameObject *activatedToolObjectBase;

void prepareMouseHitTest(int mouseX, int mouseY);

void addHitTestCube(class GameObject *o, int toolId, const Vector &siz);
Hit *getNearestHit();
Hit *getNearestToolHit();

void hitTestLine(class GameObject *o, int toolId, const Vector &p0, const Vector &p1, unsigned int color);

void addHitTestMoveGizmo(class GameObject *o, const Vector &siz);
void addHitTestScaleGizmo(class GameObject *o, const Vector &siz);
void addHitTestRotateGizmo(class GameObject *o, const Vector &siz);

#endif //__HITTEST_HPP__

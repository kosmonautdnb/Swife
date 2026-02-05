#ifndef __TOOLBAR_HPP__
#define __TOOLBAR_HPP__

enum MODE {
  MODE_UNDEFINED = 0,
  MODE_SELECT = 1,
  MODE_MOVE = 2,
  MODE_SCALE = 3,
  MODE_ROTATE = 4
};

extern int mode;

extern int toolBar_height;
extern float cameraSpeed;

void initToolBar();
void drawToolBar();

#endif //__TOOLBAR_HPP__

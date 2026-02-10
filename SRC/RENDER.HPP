#ifndef __RENDER_HPP__
#define __RENDER_HPP__

#include "STL/VECTOR.HPP"
#include "STL/QUATERNN.HPP"
#include "CONFIG.HPP"

class Camera {
public:
  Camera() {
    fov = 60.f;
    orthoScale = 100.f;
    nearPlane = 0.1f;
    farPlane = 1000.f;
    aspect = aspectRatio;
    ortho = false;
    position.set(0,25,0);
    rotation.yawPitchRoll(-45*PI/180.0,-25*PI/180.0,0.001);
  }
  float fov;
  float orthoScale;
  float nearPlane;
  float farPlane;
  float aspect;
  bool ortho;
  Vector position;
  Quaternion rotation;
  void applyToGL();
  void wasdControls();
};

enum CAMERAS {
  CAMERA_USER = 0,
  CAMERA_CAMERA1 = 1,
  CAMERA_CAMERA2 = 2,
  CAMERA_COUNT = 3
};

extern Camera cameras[CAMERA_COUNT];
extern int currentCamera;

extern unsigned int clearColor;

extern bool renderingUpdated;
extern bool renderingUpdated2; // for continuos / animation
extern unsigned int selectionCounter; // for objects on same depth when hittest
extern int drawProblems;

void disableRenderMouseClicks();

void updateCamera();

void initRendering();

void drawScene();

#endif //__RENDER_HPP__


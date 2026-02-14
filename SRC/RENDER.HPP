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

#define S_CAMERA(__name__,__o__)\
  S_OBJECT_OPEN(__name__);\
  S_FLOAT("fov",(__o__)->fov);\
  S_FLOAT("orthoScale",(__o__)->orthoScale);\
  S_FLOAT("nearPlane",(__o__)->nearPlane);\
  S_FLOAT("farPlane",(__o__)->farPlane);\
  S_FLOAT("aspect",(__o__)->aspect);\
  S_BOOL("ortho",(__o__)->ortho);\
  S_VECTOR("position",(__o__)->position);\
  S_QUATERNION("rotation",(__o__)->rotation);\
  S_OBJECT_CLOSE();

enum CAMERAS {
  CAMERA_USER = 0,
  CAMERA_CAMERA1 = 1,
  CAMERA_CAMERA2 = 2,
  CAMERA_COUNT = 3
};

extern Camera cameras[CAMERA_COUNT];
extern int currentCamera;

extern unsigned int clearColor;
extern unsigned int renderedTriangles;

extern bool renderingUpdated;
extern bool renderingUpdated2; // for continuos / animation
extern unsigned int selectionCounter; // for objects on same depth when hittest
extern int drawProblems;
extern bool lightOn;
extern bool texturedOn;
extern bool twoSidedOn;

void disableRenderMouseClicks();

void updateCamera();

void initRendering();

void drawScene();

#endif //__RENDER_HPP__


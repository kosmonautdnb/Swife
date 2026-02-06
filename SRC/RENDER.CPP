#include "GL.H"
#include "IMGUIGL.HPP"
#include "RENDER.HPP"
#include "OUTLINER.HPP"
#include "PROPS.HPP"
#include "TIMELINE.HPP"
#include "TOOLBAR.HPP"
#include "MENUBAR.HPP"
#include "CONFIG.HPP"
#include "MATRIX.HPP"
#include "GIZMOS.HPP"
#include "GAMEOBJ.HPP"
#include "PAINT.HPP"
#include "STL/KEYMTRIX.HPP"
#include <math.h>

Camera cameras[CAMERA_COUNT];
int currentCamera = 0;
double stretchy = 1.0;

void Camera::wasdControls() {
  if (ImGui::GetIO().WantCaptureKeyboard) return;
  double walkSpeed = cameraSpeed <= 0 ? 0 : pow(2.0,cameraSpeed) * 20.0;
  double mouseRotationSpeed = 0.01;
  double mouseWalkSpeed = 0.01;
  Quaternion q = normalize(rotation);
  if (isKeyPressed(SCANCODE_W)) {position += q.rotateVector(Vector(0,0,-1))*walkSpeed*timeDelta; renderingUpdated=true;}
  if (isKeyPressed(SCANCODE_S)) {position += q.rotateVector(Vector(0,0,1))*walkSpeed*timeDelta; renderingUpdated=true;}
  if (isKeyPressed(SCANCODE_D)) {position += q.rotateVector(Vector(1,0,0))*walkSpeed*timeDelta; renderingUpdated=true;}
  if (isKeyPressed(SCANCODE_A)) {position += q.rotateVector(Vector(-1,0,0))*walkSpeed*timeDelta; renderingUpdated=true;}
  if (isKeyPressed(SCANCODE_Q)) {position += q.rotateVector(Vector(0,-1,0))*walkSpeed*timeDelta; renderingUpdated=true;}
  if (isKeyPressed(SCANCODE_E)) {position += q.rotateVector(Vector(0,1,0))*walkSpeed*timeDelta; renderingUpdated=true;}

  GLboolean alt,shift,ctrl;
  glSpecialKeys(&shift, &ctrl, &alt);

  if (mouseB & 2) {
    if (shift) {
      Vector positionBefore = position;
      position += q.rotateVector(Vector(1,0,0))*walkSpeed*mouseWalkSpeed*mouseDX;
      position += q.rotateVector(Vector(0,-1,0))*walkSpeed*mouseWalkSpeed*mouseDY;
      if (position != positionBefore) renderingUpdated = true;
    } else {
      Quaternion rotationBefore = rotation;
      Quaternion cameraRotation = rotation;
      if (!ctrl) {
        Quaternion ym; ym.yawPitchRoll(0,-mouseDY * mouseRotationSpeed,0); 
        Quaternion xm; xm.yawPitchRoll(-mouseDX * mouseRotationSpeed,0,0); 
        cameraRotation = cameraRotation * ym;
        cameraRotation = cameraRotation * xm;
      } else {
        Quaternion xm; xm.yawPitchRoll(0,0,-mouseDX * mouseRotationSpeed); 
        cameraRotation = cameraRotation * xm;
      }
      cameraRotation = normalize(cameraRotation);
      rotation = cameraRotation;
      if (rotation != rotationBefore) {ortho = false; renderingUpdated=true;}
    }
  }
}

void updateCamera() {
  cameras[currentCamera].wasdControls();
}

unsigned int clearColor = 0xff604020;

bool renderingUpdated = false;

void Camera::applyToGL() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (!ortho) 
    gluPerspective(fov,aspect*stretchy,nearPlane,farPlane);
  else
    glOrtho(-orthoScale*aspect*stretchy,orthoScale*aspect*stretchy,-orthoScale,orthoScale,-100000,100000);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glMultMatrixd(normalize(inverse(rotation)).rotationMatrix().m);
  glTranslatef(-position.x,-position.y,-position.z);
}

const int renderTinyDividerX = 4;
const int renderTinyDividerY = 4;

GLuint renderBigFrameBufferColor;
GLuint renderBigFrameBufferDepth;
GLuint renderBigFrameBufferStencil;
GLuint renderBigFrameBuffer;
GLuint renderTinyFrameBufferColor;
GLuint renderTinyFrameBufferDepth;
GLuint renderTinyFrameBufferStencil;
GLuint renderTinyFrameBuffer;

void initRendering() {

  int w,h;
  w = main_window_xres;
  h = main_window_yres;

  glGenTextures(1,&renderBigFrameBufferColor);
  glBindTexture(GL_TEXTURE_2D,renderBigFrameBufferColor);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, w, h, 0, GL_RGBA, GL_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glGenTextures(1,&renderBigFrameBufferDepth);
  glBindTexture(GL_TEXTURE_2D,renderBigFrameBufferDepth);
  glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glGenTextures(1,&renderBigFrameBufferStencil);
  glBindTexture(GL_TEXTURE_2D,renderBigFrameBufferStencil);
  glTexImage2D(GL_TEXTURE_2D,0,GL_STENCIL_INDEX8, w, h, 0, GL_STENCIL_INDEX8, GL_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D,0);
  glGenFramebuffers(1,&renderBigFrameBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER,renderBigFrameBuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderBigFrameBufferColor, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderBigFrameBufferDepth, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, renderBigFrameBufferStencil, 0);

  w = main_window_xres / renderTinyDividerX;
  h = main_window_yres / renderTinyDividerY;

  glGenTextures(1,&renderTinyFrameBufferColor);
  glBindTexture(GL_TEXTURE_2D,renderTinyFrameBufferColor);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, w, h, 0, GL_RGBA, GL_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glGenTextures(1,&renderTinyFrameBufferDepth);
  glBindTexture(GL_TEXTURE_2D,renderTinyFrameBufferDepth);
  glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glGenTextures(1,&renderTinyFrameBufferStencil);
  glBindTexture(GL_TEXTURE_2D,renderTinyFrameBufferStencil);
  glTexImage2D(GL_TEXTURE_2D,0,GL_STENCIL_INDEX8, w, h, 0, GL_STENCIL_INDEX8, GL_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D,0);
  glGenFramebuffers(1,&renderTinyFrameBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER,renderTinyFrameBuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTinyFrameBufferColor, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderTinyFrameBufferDepth, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, renderTinyFrameBufferStencil, 0);

  glBindFramebuffer(GL_FRAMEBUFFER,0);

  renderingUpdated = true;
}

void enableFrameBuffer(bool tiny) {
  int tx = tiny ? renderTinyDividerX : 1;
  int ty = tiny ? renderTinyDividerY : 1;
  int x0 = (outliner_width)/tx;
  int x1 = (main_window_xres-properties_width)/tx;
  int y0 = (menuBar_height + toolBar_height)/ty;
  int y1 = (main_window_yres-timeLine_height)/ty;
  if (fullScreen) {
    x0 = 0;
    y0 = 0;
    x1 = main_window_xres/tx;
    y1 = main_window_yres/ty;
  }
  glBindFramebuffer(GL_FRAMEBUFFER,tiny ? renderTinyFrameBuffer : renderBigFrameBuffer);
  glViewport(x0,main_window_yres/ty-y1,x1-x0,y1-y0);
  glScissor(x0,main_window_yres/ty-y1,x1-x0,y1-y0);
  stretchy = (double)(x1-x0)/((double)main_window_xres/tx);
  stretchy /= (double)(y1-y0)/((double)main_window_yres/ty);
}

void blitFrameBuffer(bool tiny) {
  glBindFramebuffer(GL_FRAMEBUFFER,0);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  int x0 = outliner_width;
  int x1 = main_window_xres-properties_width;
  int y0 = menuBar_height + toolBar_height;
  int y1 = main_window_yres-timeLine_height;
  if (fullScreen) {
    x0 = 0;
    y0 = 0;
    x1 = main_window_xres;
    y1 = main_window_yres;
  }
  glViewport(x0,main_window_yres-y1,x1-x0,y1-y0);
  glScissor(x0,main_window_yres-y1,x1-x0,y1-y0);
  float tx0 = (float)x0 / main_window_xres;
  float tx1 = (float)x1 / main_window_xres;
  float ty0 = (float)y0 / main_window_yres;
  float ty1 = (float)y1 / main_window_yres;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,tiny ? renderTinyFrameBufferColor : renderBigFrameBufferColor);
  glBegin(GL_QUADS);
  glColor4f(1,1,1,1);
  glTexCoord2f(tx1,ty0); glVertex3f(1,1,-0);
  glTexCoord2f(tx0,ty0); glVertex3f(-1,1,-0);
  glTexCoord2f(tx0,ty1); glVertex3f(-1,-1,-0);
  glTexCoord2f(tx1,ty1); glVertex3f(1,-1,-0);
  glEnd();
  glDisable(GL_TEXTURE_2D);
}

void drawGameObject(GameObject *o) {
  switch(o->type) {
  case GAMEOBJECT_MESH: {} break;
  case GAMEOBJECT_CAMERA: {} break;
  case GAMEOBJECT_SKYBOX: {paintSkyBox(o);} break;
  case GAMEOBJECT_PATH: {} break;
  case GAMEOBJECT_BILLBOARD: {} break;
  }
}

void drawSkyBox() {
  for (int i = 0; i < gameObjects.size(); i++) {
    GameObject *o = gameObjects[i];
    if (!(o->visible1 && o->visible2)) continue;
    if (!((o->layers==0)||(outlinerLayers==0)||(o->layers & outlinerLayers))) continue;
    if (o->type == GAMEOBJECT_SKYBOX) drawGameObject(o);
  }
}

void drawGameObjects() {
  for (int i = 0; i < gameObjects.size(); i++) {
    GameObject *o = gameObjects[i];
    if (!(o->visible1 && o->visible2)) continue;
    if (!((o->layers==0)||(outlinerLayers==0)||(o->layers & outlinerLayers))) continue;
    if (o->type != GAMEOBJECT_SKYBOX) drawGameObject(o);
  }
}

void drawScene() {

  static unsigned int k = 0; if (renderingUpdated) {renderingUpdated = false; k = 0;} k++;

  bool tiny = k == 1;
  if (k < 3) {
    GLboolean f = glFastTexturing; glFastTexturing = GL_FALSE;
    
    enableFrameBuffer(tiny);
  
    glClearDepth(1.f);
    glClearColor((clearColor & 255)/255.0,((clearColor>>8) & 255)/255.0,((clearColor>>16) & 255)/255.0,((clearColor>>24) & 255)/255.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
    cameras[currentCamera].applyToGL();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);      

    drawSkyBox();
    draw3DGrid();
    drawGameObjects();

    glFastTexturing = f;
  }

  blitFrameBuffer(tiny);

  glViewport(0,0,main_window_xres,main_window_yres);
  glScissor(0,0,main_window_xres,main_window_yres);
}

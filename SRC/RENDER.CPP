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
#include "HITTEST.HPP"
#include "STL/KEYMTRIX.HPP"
#include "STL/ARRAY.HPP"
#include "STL/SET.HPP"
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
  if (isKeyPressed(SCANCODE_W)) {position += q.rotateVector(Vector(0,0,-1))*walkSpeed*timeDelta; renderingUpdated2=true;}
  if (isKeyPressed(SCANCODE_S)) {position += q.rotateVector(Vector(0,0,1))*walkSpeed*timeDelta; renderingUpdated2=true;}
  if (isKeyPressed(SCANCODE_D)) {position += q.rotateVector(Vector(1,0,0))*walkSpeed*timeDelta; renderingUpdated2=true;}
  if (isKeyPressed(SCANCODE_A)) {position += q.rotateVector(Vector(-1,0,0))*walkSpeed*timeDelta; renderingUpdated2=true;}
  if (isKeyPressed(SCANCODE_Q)) {position += q.rotateVector(Vector(0,-1,0))*walkSpeed*timeDelta; renderingUpdated2=true;}
  if (isKeyPressed(SCANCODE_E)) {position += q.rotateVector(Vector(0,1,0))*walkSpeed*timeDelta; renderingUpdated2=true;}

  GLboolean alt,shift,ctrl;
  glSpecialKeys(&shift, &ctrl, &alt);

  if (mouseB & 2) {
    if (shift) {
      Vector positionBefore = position;
      position += q.rotateVector(Vector(1,0,0))*walkSpeed*mouseWalkSpeed*mouseDX;
      position += q.rotateVector(Vector(0,-1,0))*walkSpeed*mouseWalkSpeed*mouseDY;
      if (position != positionBefore) renderingUpdated2 = true;
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
      if (rotation != rotationBefore) {ortho = false; renderingUpdated2=true;}
    }
  }
}

void updateCamera() {
  cameras[currentCamera].wasdControls();
}

unsigned int clearColor = 0xff604020;

bool renderingUpdated = false;
bool renderingUpdated2 = false;

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

void paintSimpleGameObject(GameObject *o, bool showModificators) {
  switch(o->type) {
  case GAMEOBJECT_MESH: {} break;
  case GAMEOBJECT_CAMERA: {} break;
  case GAMEOBJECT_PATH: {} break;
  case GAMEOBJECT_BILLBOARD: {} break;
  case GAMEOBJECT_TRIGGER: {paintTrigger(o,showModificators);} break;
  }
}

void drawGameObject(GameObject *o) {
  if (o->selected) {
    glStencilFunc(GL_ALWAYS,0x01,0xffffffff);
    glStencilOp(GL_REPLACE,GL_KEEP,GL_REPLACE);
    glEnable(GL_STENCIL_TEST);
  }
  switch(o->type) {
  case GAMEOBJECT_LINK: {
    // no recursion allowed in WatcomC
    Array<GameObject*> objs;
    Array<GameObject*> links;
    Array<Set<GameObject*> * > circular;
    Array<Matrix> objParentMatrices;
    Array<Matrix> linkMatrices;
    links.push_back(o);
    linkMatrices.push_back(o->getObjectTransform());
    Set<GameObject*> *c = new Set<GameObject*>; c->insert(o); circular.push_back(c);
    while(!links.empty()) {
      GameObject *l = links.back(); links.pop_back();
      Matrix parentMatrix = linkMatrices.back(); linkMatrices.pop_back();
      Set<GameObject*> *c = circular.back(); circular.pop_back();
      l->hasDrawProblem = false;
      for (int i = 0; i < l->uniqueLinkIds.size(); i++) {
        int j = getObjectIdByUniqueId(l->uniqueLinkIds[i]);
        if (j >= 0) {
          GameObject *o = gameObjects[j];
          Matrix m = parentMatrix * o->getObjectTransform();
          if (o->type == GAMEOBJECT_LINK) {
            if (!c->has(o)) {
              links.push_back(o);
              linkMatrices.push_back(m);
              Set<GameObject*> *c2 = new Set<GameObject*>; *c2 = *c; c2->insert(o); circular.push_back(c2);
            } else {
              l->hasDrawProblem = true;
              drawProblems++;
            }
          }
          objs.push_back(o);
          objParentMatrices.push_back(m);
        }
      }
      delete c;
    }
    for (int i = 0; i < objs.size(); i++) {
      Matrix parentBefore = objs[i]->parentTransform;
      objs[i]->parentTransform = objParentMatrices[i];
      paintSimpleGameObject(objs[i], false);
      objs[i]->parentTransform = parentBefore;
    }
  } break;
  default:
    paintSimpleGameObject(o, true);
  }
  glDisable(GL_STENCIL_TEST);
}

void drawSkyBox() {
  for (int i = 0; i < gameObjects.size(); i++) {
    GameObject *o = gameObjects[i];
    if (!(o->visible1 && o->visible2)) continue;
    if (!((o->layers==0)||(outlinerLayers==0)||(o->layers & outlinerLayers))) continue;
    if (o->type == GAMEOBJECT_SKYBOX) {
      if (o->selected) {
        glStencilFunc(GL_ALWAYS,0x01,0xffffffff);
        glStencilOp(GL_REPLACE,GL_KEEP,GL_REPLACE);
        glEnable(GL_STENCIL_TEST);
      }
      paintSkyBox(o);
      glDisable(GL_STENCIL_TEST);
    }
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

void drawSelecting(bool tiny) {
  glStencilFunc(GL_EQUAL,0x01,0xffffffff);
  glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
  glEnable(GL_STENCIL_TEST);
  int tx = tiny ? renderTinyDividerX : 1;
  int ty = tiny ? renderTinyDividerY : 1;
  int x0 = (outliner_width)/tx;
  int x1 = (main_window_xres-properties_width)/tx;
  int y0 = (menuBar_height + toolBar_height)/ty;
  int y1 = (main_window_yres-timeLine_height)/ty;
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glBegin(GL_QUADS);
  glColor4f(1,0,0,0.25);
  glVertex3f(1,1,0);
  glVertex3f(-1,1,0);
  glVertex3f(-1,-1,0);
  glVertex3f(1,-1,0);
  glEnd();
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glDisable(GL_STENCIL_TEST);
  glDisable(GL_BLEND);
}

unsigned int selectionCounter = 0;
int renderMouseClicksOff = 0;
int drawProblems = 0;

void disableRenderMouseClicks() {
  renderMouseClicksOff = 2;
}

void handleHitTest() {
  GLboolean shift,ctrl,alt;
  glSpecialKeys(&shift,&ctrl,&alt);

  if (startTool) {
    Hit *h = getNearestToolHit();
    if (h != NULL) {
      if (activatedToolObjectBase != NULL) {delete activatedToolObjectBase; activatedToolObjectBase = NULL;}
      activatedToolObjectBase = h->object->clone();
      activatedTool = h->toolId;
      renderingUpdated = true;
    }
  }

  switch(mode) {
    case MODE_SELECT: {
      static double lastDepth = 0;
      Hit *h = getNearestHit(&selectionCounter,lastDepth); selectionCounter++;
      lastDepth = h->depth;
      if (h != NULL) {
        if (shift) h->object->selected = true;
        else
        if (ctrl) h->object->selected = false;
        else {
          for (int i = 0; i < gameObjects.size(); i++) gameObjects[i]->selected = false;
          h->object->selected = true;
        }
        h->object->outlinerShouldFocusThis = true;
      } else {
        for (int i = 0; i < gameObjects.size(); i++) gameObjects[i]->selected = false;
      }
      renderingUpdated = true;
    } break;
  }
}

bool mouseNotInRenderView() {
  bool ret = ImGui::GetIO().WantCaptureMouse;
  int x0 = (outliner_width);
  int x1 = (main_window_xres-properties_width);
  int y0 = (menuBar_height + toolBar_height);
  int y1 = (main_window_yres-timeLine_height);
  int mx = ImGui::GetMousePos().x;
  int my = ImGui::GetMousePos().y;
  if (mx >= x0 && mx < x1 && my >= y0 && my < y1) ret = false;
  return ret;
}

void screenSpacePlane(GameObject *o, Vector *d2d, Vector *d3d, double x, double y, double z) {

  *d3d = normalize(normalize(o->rotation).rotationMatrix() * Vector(x,y,z));

  Matrix projection; glGetDoublev(GL_PROJECTION_MATRIX, projection.m);
  Matrix modelView; glGetDoublev(GL_MODELVIEW_MATRIX, modelView.m);

  Vector t0 = projection * modelView * (o->translation);
  if (t0.w != 0) {t0/=t0.w; t0.w = 1;}
  Vector t1 = t0;
  t1.x += 2.0/main_window_xres;
  if (t1.w != 0) {t1/=t1.w; t1.w = 1;}
  Vector d0 = inverse(projection) * t0;
  Vector d1 = inverse(projection) * t1;
  if (d0.w != 0) {d0/=d0.w; d0.w = 1;}
  if (d1.w != 0) {d1/=d1.w; d1.w = 1;}

  *d3d *= length(d1-d0);                                                                         

  Vector p0 = projection * modelView * (o->translation);
  Vector p1 = projection * modelView * (o->translation + normalize(o->rotation).rotateVector(Vector(x,y,z)));

  if (p0.w != 0) {p0/=p0.w; p0.w = 1;}
  if (p1.w != 0) {p1/=p1.w; p1.w = 1;}

  *d2d = normalize(p1-p0);
  d2d->y *= -1;
}

void handleTool() {
  if (activatedToolObjectBase == NULL) return;

  Vector d2d,d3d,scale,rot;

  switch(activatedTool) {
    case HITTESTTOOL_ROTATE_PLUS_X: {rot = Vector(0,1,0);} break;
    case HITTESTTOOL_ROTATE_MINUS_X: {rot = Vector(0,-1,0);} break;
    case HITTESTTOOL_ROTATE_PLUS_Y: {rot = Vector(0,0,1);} break;
    case HITTESTTOOL_ROTATE_MINUS_Y: {rot = Vector(0,0,-1);} break;
    case HITTESTTOOL_ROTATE_PLUS_Z: {rot = Vector(1,0,0);} break;
    case HITTESTTOOL_ROTATE_MINUS_Z:  {rot = Vector(-1,0,0);} break;
    case HITTESTTOOL_SCALE_PLUS_X:  {scale = Vector(1,0,0);} break;
    case HITTESTTOOL_SCALE_MINUS_X: {scale = Vector(-1,0,0);} break;
    case HITTESTTOOL_SCALE_PLUS_Y:  {scale = Vector(0,1,0);} break;
    case HITTESTTOOL_SCALE_MINUS_Y: {scale = Vector(0,-1,0);} break;
    case HITTESTTOOL_SCALE_PLUS_Z:  {scale = Vector(0,0,1);} break;
    case HITTESTTOOL_SCALE_MINUS_Z:  {scale = Vector(0,0,-1);} break;
  }

  switch(activatedTool) {
    case HITTESTTOOL_ROTATE_PLUS_X: 
    case HITTESTTOOL_ROTATE_MINUS_X: 
    case HITTESTTOOL_ROTATE_PLUS_Y: 
    case HITTESTTOOL_ROTATE_MINUS_Y: 
    case HITTESTTOOL_ROTATE_PLUS_Z: 
    case HITTESTTOOL_ROTATE_MINUS_Z: 
      {screenSpacePlane(activatedToolObjectBase,&d2d,&d3d,fabs(rot.x),fabs(rot.y),fabs(rot.z));} break;
    case HITTESTTOOL_SCALE_PLUS_X: 
    case HITTESTTOOL_SCALE_MINUS_X: 
    case HITTESTTOOL_MOVE_PLUS_X: 
    case HITTESTTOOL_MOVE_MINUS_X: 
      {screenSpacePlane(activatedToolObjectBase,&d2d,&d3d,1,0,0);} break;
    case HITTESTTOOL_SCALE_PLUS_Y: 
    case HITTESTTOOL_SCALE_MINUS_Y: 
    case HITTESTTOOL_MOVE_PLUS_Y: 
    case HITTESTTOOL_MOVE_MINUS_Y: 
      {screenSpacePlane(activatedToolObjectBase,&d2d,&d3d,0,1,0);} break;
    case HITTESTTOOL_SCALE_PLUS_Z: 
    case HITTESTTOOL_SCALE_MINUS_Z: 
    case HITTESTTOOL_MOVE_PLUS_Z: 
    case HITTESTTOOL_MOVE_MINUS_Z: 
      {screenSpacePlane(activatedToolObjectBase,&d2d,&d3d,0,0,1);} break;
  }
  scale *= length(d3d);

  bool flip = false;
  switch(activatedTool) {
    case HITTESTTOOL_MOVE_MINUS_X: 
    case HITTESTTOOL_MOVE_MINUS_Y: 
    case HITTESTTOOL_MOVE_MINUS_Z: 
    case HITTESTTOOL_MOVE_PLUS_X: 
    case HITTESTTOOL_MOVE_PLUS_Y: 
    case HITTESTTOOL_MOVE_PLUS_Z: 
    {
      double v = mouseDX * d2d.x + mouseDY * d2d.y;
      if (v == 0) return;
      for (int i = 0; i < gameObjects.size(); i++) {
        GameObject *o = gameObjects[i];
        if (o->selected) {
          gameObjects[i]->translation += d3d * v;
          renderingUpdated2 = true;
        }
      }
    } break;
    case HITTESTTOOL_SCALE_PLUS_X: 
    case HITTESTTOOL_SCALE_MINUS_X: 
    case HITTESTTOOL_SCALE_PLUS_Y: 
    case HITTESTTOOL_SCALE_MINUS_Y: 
    case HITTESTTOOL_SCALE_PLUS_Z: 
    case HITTESTTOOL_SCALE_MINUS_Z: 
    {
      double v = mouseDX * d2d.x + mouseDY * d2d.y;
      if (v == 0) return;
      for (int i = 0; i < gameObjects.size(); i++) {
        GameObject *o = gameObjects[i];
        if (o->selected) {
          gameObjects[i]->scaling += scale * v;
          double minScale = 0.00001;
          if (gameObjects[i]->scaling.x < minScale) gameObjects[i]->scaling.x = minScale;
          if (gameObjects[i]->scaling.y < minScale) gameObjects[i]->scaling.y = minScale;
          if (gameObjects[i]->scaling.z < minScale) gameObjects[i]->scaling.z = minScale;
          renderingUpdated2 = true;
        }
      }
    } break;
    case HITTESTTOOL_ROTATE_PLUS_X: 
    case HITTESTTOOL_ROTATE_MINUS_X: 
    case HITTESTTOOL_ROTATE_PLUS_Y: 
    case HITTESTTOOL_ROTATE_MINUS_Y: 
    case HITTESTTOOL_ROTATE_PLUS_Z: 
    case HITTESTTOOL_ROTATE_MINUS_Z: 
    {
      double v = mouseDX * d2d.x + mouseDY * d2d.y;
      if (v == 0) return;
      double rotScale = 1.0/180.0*5.0;
      v *= rotScale;
      for (int i = 0; i < gameObjects.size(); i++) {
        GameObject *o = gameObjects[i];
        if (o->selected) {
          Quaternion rotation2; rotation2.yawPitchRoll(rot.x*v,rot.z*v,rot.y*v);
          gameObjects[i]->rotation = normalize(normalize(gameObjects[i]->rotation)*normalize(rotation2));
          renderingUpdated2 = true;
        }
      }
    } break;
  }
}

void drawScene() {
  bool leftClicked = ImGui::IsMouseClicked(0);
  bool leftDown = ImGui::IsMouseDown(0);
  if (renderMouseClicksOff>0) {renderMouseClicksOff--; leftClicked = false; leftDown = false;}

  static unsigned int k = 0;
  bool hitTestOn = (mode == MODE_SELECT) && leftClicked;

  static bool lastMouseButtonLeft = false;
  startTool = (!lastMouseButtonLeft) && leftDown;
  bool stopTool = lastMouseButtonLeft && (!leftDown);
  lastMouseButtonLeft = leftDown;
  hitTestOn = hitTestOn || startTool;
  if (activatedTool != HITTESTTOOL_NONE && stopTool) {
    activatedTool = HITTESTTOOL_NONE;
    renderingUpdated = true;
  }

  if (mouseNotInRenderView()) hitTestOn = false;

  if (renderingUpdated) {renderingUpdated = false; k = 1;} 
  if (renderingUpdated2) {renderingUpdated2 = false; k = 0;} 
  if (hitTestOn) k = 1;
  k++;

  bool tiny = k == 1;
  if (k < 3) {
    GLboolean f = glFastTexturing; glFastTexturing = GL_FALSE;
    
    enableFrameBuffer(tiny);
  
    glClearDepth(1.f);
    glClearStencil(0);
    glClearColor((clearColor & 255)/255.0,((clearColor>>8) & 255)/255.0,((clearColor>>16) & 255)/255.0,((clearColor>>24) & 255)/255.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
  
    cameras[currentCamera].applyToGL();
    if (hitTestOn) 
      prepareMouseHitTest(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
    else
      prepareMouseHitTest(-1,-1);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);      

    drawProblems = 0;
    drawSkyBox();
    draw3DGrid();
    drawGameObjects();
    drawSelecting(tiny);

    if (hitTestOn) handleHitTest();

    glFastTexturing = f;
  }

  cameras[currentCamera].applyToGL(); handleTool();

  blitFrameBuffer(tiny);

  glViewport(0,0,main_window_xres,main_window_yres);
  glScissor(0,0,main_window_xres,main_window_yres);
}

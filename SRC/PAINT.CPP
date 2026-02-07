#include "GL.H"
#include "PAINT.HPP"
#include "GAMEOBJ.HPP"
#include "CONFIG.HPP"
#include "RENDER.HPP"
#include "STL/VECTOR.HPP"
#include "STL/QUATERNN.HPP"
#include "STL/MATRIX.HPP"
#include "STL/IMAGE.HPP"
#include "TEXTURES.HPP"
#include "GIZMOS.HPP"

#define SKYBOX_LEFT 0
#define SKYBOX_FRONT 1
#define SKYBOX_RIGHT 2
#define SKYBOX_BACK 3
#define SKYBOX_TOP 4
#define SKYBOX_BOTTOM 5

void initPaint() {
}

void drawSkyboxQuad(unsigned int texture, float size, float quadRot, const Vector &camPos, const Quaternion &rotation, const Quaternion &rotation2, const Vector &pos) {
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
  glColor4f(1,1,1,1);
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

void loadSkyBox(class GameObject *o) {
  o->textureIds.resize(6);
  RGBAImage s = RGBAImage::fromFile("DATA/CUBEMAPS/SKYBOX1.PNG");
  skyBoxFromHorizontalCrossImage(&o->textureIds[0], &s);
  s.free();
}

void paintSkyBox(class GameObject *o) {
  if (o->textureIds.size() != 6) return;
  float size = 1.0;
  float k = PI/180.0;
  Vector cameraPosition = cameras[currentCamera].position;
  Quaternion r;
  double w = PI/180.0;
  r.yawPitchRoll(o->rotation.x*w,o->rotation.y*w,o->rotation.z*w);
  r = normalize(r);
  Quaternion left; left.yawPitchRoll(k * 90,0,0);
  Quaternion right; right.yawPitchRoll(-k * 90,0,0);
  Quaternion front; front.yawPitchRoll(0,0,0);
  Quaternion back; back.yawPitchRoll(k * 180,0,0);
  Quaternion top; top.yawPitchRoll(0,k * 90,0);
  Quaternion bottom; bottom.yawPitchRoll(0,k * -90,0);
  drawSkyboxQuad(o->textureIds[SKYBOX_LEFT], size,0,cameraPosition,left,r,Vector(-size,0,0));
  drawSkyboxQuad(o->textureIds[SKYBOX_RIGHT], size,0,cameraPosition,right,r,Vector(size,0,0));
  drawSkyboxQuad(o->textureIds[SKYBOX_FRONT], size,0,cameraPosition,front,r,Vector(0,0,-size));
  drawSkyboxQuad(o->textureIds[SKYBOX_BACK], size,0,cameraPosition,back,r,Vector(0,0,size));
  drawSkyboxQuad(o->textureIds[SKYBOX_TOP], size,0,cameraPosition,top,r,Vector(0,size,0));
  drawSkyboxQuad(o->textureIds[SKYBOX_BOTTOM], size,0,cameraPosition,bottom,r,Vector(0,-size,0));
}

void paintTrigger(class GameObject *o) {

  glPushMatrix();

  Vector pos = o->translation;
  Vector scal = o->scaling;
  Vector rot = o->rotation;
  Quaternion r;
  double w = PI/180.0;
  r.yawPitchRoll(rot.x*w,rot.y*w,rot.z*w);
  r = normalize(r);
  glTranslatef(pos.x,pos.y,pos.z);
  glMultMatrixd(r.rotationMatrix().m);
  glScalef(scal.x,scal.y,scal.z);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,0);
  glBindTexture(GL_TEXTURE_2D,gridTexture);

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

  glDisable(GL_ALPHA_TEST);
  glDisable(GL_TEXTURE_2D);

  glPopMatrix();
}

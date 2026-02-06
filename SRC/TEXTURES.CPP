#include "GL.H"
#include "IMGUIGL.HPP"
#include "TEXTURES.HPP"
#include "STL/IMAGE.HPP"

bool textureButton(const char *id, unsigned int textureId, int w, int h, bool decoration) {
  ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0,0,0,1));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,ImVec4(0,0,0,1));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(0,0,0,1));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0,0));
  ImVec2 cursor = ImGui::GetCursorScreenPos();
  ImGui::PushID(id);
  int w2 = glGetTextureWidth(textureId);
  int h2 = glGetTextureHeight(textureId);
  double ax = (double)w2/h2;
  double ay = (double)h2/w2;
  if (ax<ay) ay = 1; else ax = 1;
  if (ax != 0) ax = 1.0/ax;
  if (ay != 0) ay = 1.0/ay;
  bool ret = ImGui::ImageButton((ImTextureID)textureId,ImVec2(w,h),ImVec2(0.5-ax*0.5,0.5-ay*0.5),ImVec2(0.5+ax*0.5,0.5+ay*0.5));
  ImGui::PopID();
  if (ax != 0) ax = 1.0/ax;
  if (ay != 0) ay = 1.0/ay;
  ax = 1-ax;
  ay = 1-ay;
  ax *= 0.5;
  ay *= 0.5;
  ImGui::GetWindowDrawList()->AddRectFilled(cursor,ImVec2(cursor.x+ax*w,cursor.y+h),0xff000000);
  ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(cursor.x+w,cursor.y),ImVec2(cursor.x+w-ax*w,cursor.y+h),0xff000000);
  ImGui::GetWindowDrawList()->AddRectFilled(cursor,ImVec2(cursor.x+w,cursor.y+ay*h),0xff000000);
  ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(cursor.x,cursor.y+h),ImVec2(cursor.x+w,cursor.y+h-ay*h),0xff000000);
  if (decoration) {
    char buffer[32];
    sprintf(buffer,"%dx%d",w2,h2);
    ImGui::GetWindowDrawList()->AddText(ImGui::GetWindowFont(),ImGui::GetWindowFontSize(),ImVec2(cursor.x,cursor.y),0xff000000,buffer);
    cursor.x += 1;
    cursor.y += 1;
    ImGui::GetWindowDrawList()->AddText(ImGui::GetWindowFont(),ImGui::GetWindowFontSize(),ImVec2(cursor.x,cursor.y),0xffffffff,buffer);
  }
  ImGui::PopStyleVar();
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  return ret;
}

unsigned int textureFromImageArea(const class RGBAImage *i, int x0, int y0, int x1, int y1) {
  unsigned int t = 0;
  int w = x1 - x0;
  int h = y1 - y0;
  unsigned int *k = (unsigned int*)malloc(w*h*sizeof(unsigned int));
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      unsigned int c = (x^y)|0xff000000;
      int xp = x + x0;
      int yp = y + y0;
      if (xp>=0&&xp<i->width&&yp>=0&&yp<i->height) {
        c = i->data[xp+yp*i->width];
      }
      k[x+y*w] = c;
    }
  }
  glGenTextures(1,&t);
  glBindTexture(GL_TEXTURE_2D,t);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, w, h, 0, GL_RGBA, GL_BYTE, k);
  free(k);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  return t;
}

void skyBoxFromHorizontalCrossImage(unsigned int t[6], const class RGBAImage *s) {
  for (int j = 0; j < 6; j++) {glDeleteTextures(1,&t[j]); t[j] = 0;}
  if (s->data == NULL) return;
  int kx = s->height/3;
  int ky = s->width/4;
  int k = kx;
  for (int i = 0; i < 4; i++) {
    t[i] = textureFromImageArea(s,i*k,k,(i+1)*k,k*2);
  }
  t[4] = textureFromImageArea(s,k,0,k*2,k);
  t[5] = textureFromImageArea(s,k,k*2,k*2,k*3);
}

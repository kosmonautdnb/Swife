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
#include "TEXPAINT.HPP"
#include "CONFIG.HPP"
#include "GL.H"
#include "IMGUIGL.HPP"
#include "VIEWPORT.HPP"
#include "OUTLINER.HPP"
#include "PROPS.HPP"
#include "MENUBAR.HPP"
#include "TOOLBAR.HPP"
#include "TIMELINE.HPP"
#include "GAMEOBJ.HPP"
#include "DIALOGS.HPP"
#include "STL/SET.HPP"
#include "STL/IMAGE.HPP"
#include <math.h>

float imageZoom = 1;
Vector imageScroll(0,0);
Vector imageMouse(0,0);
int selectedTileIndex = -1;
bool paintingSaveOn = false;
bool paintBlended = false;
bool paintFlipX = false;
bool paintFlipY = false;
bool paintRotate = false;

bool stampOn = false;
int stampX0=-1,stampY0=-1,stampX1=-1,stampY1=-1;
unsigned int stampTexture = 0;

void selectImageTile(int tileIndex) {
  selectedTileIndex = tileIndex;
}

void fillImage(int x, int y, unsigned int color, unsigned int *pixels, int w, int h) {
  Set<unsigned int> set;
  int count = 0;
  for (int i = 0; i < w * h; i++) {if ((pixels[i]&0xff000000)==0) if (!set.has(pixels[i])) {set.insert(pixels[i]);count++; if (count>2048) return;}}
  unsigned int found = 0x00ffffff;
  for (int j = 0; j < 0x00ffffff; j++) if (!set.has(j)) {found = j; break;}
  unsigned int colorToReplace = pixels[x+y*w];
  pixels[x+y*w] = found;
  bool some = true;
  while(some) {
    some = false;
    for (int y2 = 0; y2 < h; y2++) {
      for (int x2 = 0; x2 < w; x2++) {
        if (pixels[x2+y2*w] == found) {
          if (x2>0 && pixels[x2-1+y2*w] == colorToReplace) {pixels[x2-1+y2*w] = found; some = true;}
          if (x2<w-1 && pixels[x2+1+y2*w] == colorToReplace) {pixels[x2+1+y2*w] = found; some = true;}
          if (y2>0 && pixels[x2+(y2-1)*w] == colorToReplace) {pixels[x2+(y2-1)*w] = found; some = true;}
          if (y2<h-1 && pixels[x2+(y2+1)*w] == colorToReplace) {pixels[x2+(y2+1)*w] = found; some = true;}
        }
      }
    }
  }
  for (int k = 0; k < w * h; k++) if (pixels[k]==found) pixels[k] = color;
}

void applyPixel(int x, int y, unsigned int color, unsigned int *pixels, int w, int h) {
  if ((x >= 0) && (y >= 0) && (x < w) && (y < h)) {
    if (!paintBlended) 
      pixels[x+y*w] = color;
    else {
      int r1 = color & 255;
      int g1 = (color>>8) & 255;
      int b1 = (color>>16) & 255;
      int a1 = (color>>24) & 255;
      unsigned int rgba = pixels[x+y*w];
      int r2 = rgba & 255;
      int g2 = (rgba>>8) & 255;
      int b2 = (rgba>>16) & 255;
      int a2 = (rgba>>24) & 255;
      double f = a1 / 255.0;
      int r = (r1 - r2)*f+r2;
      int g = (g1 - g2)*f+g2;
      int b = (b1 - b2)*f+b2;
      int a = a1>a2?a1:a2; // erstmal so
      pixels[x+y*w] = (r)|(g<<8)|(b<<16)|(a<<24);
    }
  }
}

void drawTexPaint() {
  int count = 0; GameObject *first = NULL; for (int ic = 0; ic < gameObjects.size(); ic++) {if (gameObjects[ic]->selected) {first = gameObjects[ic]; count++;}}
  bool isOn = (count == 1 && first->type == GAMEOBJECT_IMAGE);

  GLboolean shift,ctrl,alt;
  glSpecialKeys(&shift,&ctrl,&alt);
  static int lastXPos = -1; static int lastYPos = -1;
  static int lastXPos2 = -1; static int lastYPos2 = -1;

  ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(0,0,0,isOn ? 1 : 0));
  ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.25,0.25,0.25,1));
  ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.5,0.5,0.5,1));
  ImGui::SetNextWindowPos(ImVec2(outliner_width,menuBar_height+toolBar_height));

  ImGui::Begin("TEXPAINT", NULL, ImVec2(main_window_xres-outliner_width-properties_width,main_window_yres-menuBar_height-toolBar_height-timeLine_height),1.0,FROZENWINDOW|ImGuiWindowFlags_NoScrollbar);
  ImGui::SetWindowCollapsed(!isOn);

  bool leftClicked = ImGui::IsMouseClicked(0);
  bool leftDown = ImGui::IsMouseDown(0);
  bool rightDown = ImGui::IsMouseDown(1);
  bool off = !ImGui::IsWindowHovered();
  if (renderMouseClicksOff>0) {renderMouseClicksOff--; off = true;}
  if (off) {leftClicked = false; leftDown = false; rightDown = false;}

  if (isOn) {
    FileBinding *binding = getBindingById(first->fileBindingId);
    if (binding != NULL && binding->textureIds.size()==1 && binding->originalWidth>0 && binding->originalHeight>0) {

      int r = paintColor.x * 255.0;
      int g = paintColor.y * 255.0;
      int b = paintColor.z * 255.0;
      int a = paintColor.w * 255.0;
      if (r < 0) r = 0;
      if (g < 0) g = 0;
      if (b < 0) b = 0;
      if (a < 0) a = 0;
      if (r > 255) r = 255;
      if (g > 255) g = 255;
      if (b > 255) b = 255;
      if (a > 255) a = 255;
      unsigned int rgba = r|(g<<8)|(b<<16)|(a<<24);

      int w = binding->originalWidth;
      int h = binding->originalHeight * aspectRatio * main_window_yres / main_window_xres;

      unsigned int *textureData = glGetTexturePointer(binding->textureIds[0]);
      int tw = glGetTextureWidth(binding->textureIds[0]);
      int th = glGetTextureHeight(binding->textureIds[0]);
      bool valid = textureData != NULL && tw > 0 && th > 0;

      ImVec2 cursorPos = ImGui::GetCursorScreenPos();
      cursorPos.x -= 1;
      cursorPos.y -= 4;
      ImVec2 sp;
      sp.x = floor(cursorPos.x - imageScroll.x);
      sp.y = floor(cursorPos.y - imageScroll.y);
      ImGui::SetCursorScreenPos(sp);
      float zoom = imageZoom>0 ? (pow(2.0,imageZoom)-1.0) : 0;
      int imageWidth = (int)floor(w*zoom);
      int imageHeight = (int)floor(h*zoom);
      double txf = floor(ImGui::GetMousePos().x - floor(sp.x))*tw/imageWidth;
      double tyf = floor(ImGui::GetMousePos().y - floor(sp.y))*th/imageHeight;
      int tx = (int)floor(txf);
      int ty = (int)floor(tyf);
      imageMouse.x = tx;
      imageMouse.y = ty;
      imageMouse.z = -1;
      if (tx<0||ty<0||tx>=tw||ty>=th) imageMouse.x=-1;

      static bool lineOn = false;
      bool useStamp = stampOn && stampX0>=0 && stampY0>=0 && stampX1>=0 && stampY1>=0;;
      int stampPX0 = tx-(stampX1-stampX0)/2;
      int stampPY0 = ty-(stampY1-stampY0)/2;
      int stampPX1 = stampPX0 + stampX1-stampX0;
      int stampPY1 = stampPY0 + stampY1-stampY0;

      if (!leftDown) {
        if (valid && lastXPos>=0&&lastYPos>=0&&lastXPos2>=0&&lastYPos2>=0&&abs(lastXPos2-lastXPos)>0&&abs(lastYPos2-lastYPos)>0) {
          stampX0 = lastXPos<lastXPos2?lastXPos:lastXPos2;
          stampY0 = lastYPos<lastYPos2?lastYPos:lastYPos2;
          stampX1 = lastXPos>lastXPos2?lastXPos:lastXPos2;
          stampY1 = lastYPos>lastYPos2?lastYPos:lastYPos2;
          if (stampTexture != 0) glDeleteTextures(1,&stampTexture);
          RGBAImage image;
          image.width = stampX1-stampX0;
          image.height = stampY1-stampY0;
          image.data = (unsigned int*)malloc(image.width*image.height*sizeof(unsigned int));
          for (int y = 0; y < image.height; y++) {
            for (int x = 0; x < image.width; x++) {
              unsigned int rgba = 0x00000000;
              int sx = x + stampX0;
              int sy = y + stampY0;
              if (sx >= 0 && sy >= 0 && sx < tw && sy < th) rgba = textureData[sx+sy*tw];
              image.data[x+y*image.width] = rgba;
            }
          }
          glGenTextures(1,&stampTexture);
          glBindTexture(GL_TEXTURE_2D,stampTexture);
          glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_BYTE, image.data);
          image.free();
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
          glBindTexture(GL_TEXTURE_2D,0);
          stampOn = true;
        }
        if (lineOn && alt) {
          binding->updated = true;
          int deltaX = tx - lastXPos;
          int deltaY = ty - lastYPos;
          int d = abs(deltaX);
          if (abs(deltaY)>d) d = abs(deltaY);              
          d++;
          double dxa = (double)deltaX/d;
          double dya = (double)deltaY/d;
          double xp = lastXPos;
          double yp = lastYPos;
          for (int i = 0; i < d+1; i++) {
            int tx2 = (int)floor(xp);
            int ty2 = (int)floor(yp);
            int sw = glGetTextureWidth(stampTexture);
            int sh = glGetTextureHeight(stampTexture);
            unsigned int *sd = glGetTexturePointer(stampTexture);
            if (useStamp&&sd!=NULL&&sw>0&&sh>0) {
              for (int y = stampPY0; y < stampPY1; y++) {
                for (int x = stampPX0; x < stampPX1; x++) {
                  int sx = x - stampPX0;
                  int sy = y - stampPY0;
                  if (sx >= 0 && sy >= 0 && sx < sw && sy < sh) {
                    unsigned int rgba2 = sd[sx+sy*sw];
                    applyPixel(x-tx+tx2,y-ty+ty2,rgba2,textureData,tw,th);
                  }
                }
              }
            } else {
              applyPixel(tx2,ty2,rgba,textureData,tw,th);
            }
            xp += dxa;
            yp += dya;
          }
        }
        lastXPos = lastYPos = lastXPos2 = lastYPos2 = -1;
        lineOn = false;
      }

      if (valid) {    
        if (paintingSaveOn) {
          paintingSaveOn = false;
          dialogLoadSaveSetFileNameAndPath = binding->fileName; dialogLoadSaveGameObjectUniqueId = first->uniqueId; dialogLoadSave(true, DIALOG_SAVEIMAGE);
        }
        if (rightDown && ctrl) {
          imageScroll.x -= mouseDX;
          imageScroll.y -= mouseDY;
        }

        ImGui::Image((ImTextureID)binding->textureIds[0],ImVec2(imageWidth,imageHeight));

        if (useStamp) {
          ImVec2 one = ImVec2(floor(sp.x)+floor(stampPX0*imageWidth/tw),floor(sp.y)+floor(stampPY0*imageHeight/th));
          ImVec2 zwo = ImVec2(floor(sp.x)+floor(stampPX1*imageWidth/tw),floor(sp.y)+floor(stampPY1*imageHeight/th));
          ImVec2 tex0 = ImVec2(0,0);
          ImVec2 tex1 = ImVec2(1,1);
          if (!paintBlended) {
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)0,one,zwo,ImVec2(0,0),ImVec2(1,1),0xff000000);
          }
          ImGui::GetWindowDrawList()->AddImage((ImTextureID)stampTexture,one,zwo,tex0,tex1);
          ImGui::GetWindowDrawList()->AddRect(one,zwo,0xff00ff00);
        }

        if (binding->textureAtlasTileWidth>0&&binding->textureAtlasTileHeight>0) {
          int kx = tx/binding->textureAtlasTileWidth*binding->textureAtlasTileWidth;
          int ky = ty/binding->textureAtlasTileHeight*binding->textureAtlasTileHeight;
          ImVec2 zwo = ImVec2(floor(sp.x)+floor((kx+binding->textureAtlasTileWidth)*imageWidth/tw),floor(sp.y)+floor((ky+binding->textureAtlasTileHeight)*imageHeight/th));
          zwo.x += 1; zwo.y += 1;
          ImGui::GetWindowDrawList()->AddRect(ImVec2(floor(sp.x)+floor(kx*imageWidth/tw),floor(sp.y)+floor(ky*imageHeight/th)),zwo,0xff0000ff);
        }

        ImVec2 one = ImVec2(floor(sp.x)+floor(tx*imageWidth/tw),floor(sp.y)+floor(ty*imageHeight/th));
        ImVec2 zwo = ImVec2(floor(sp.x)+floor((tx+1)*imageWidth/tw),floor(sp.y)+floor((ty+1)*imageHeight/th)); zwo.x += 1; zwo.y += 1;
        ImGui::GetWindowDrawList()->AddRect(one,zwo,0xffffffff);
     
        if (rightDown && !ctrl) {
          if (tx>=0&&ty>=0&&tx<tw&&ty<th) {
            unsigned int rgba = textureData[tx+ty*tw];
            paintColor.set(((rgba)&255)/255.0,((rgba>>8)&255)/255.0,((rgba>>16)&255)/255.0,((rgba>>24)&255)/255.0);
            addPreviousColor(paintColor);
          }
        }
        if (leftDown && shift) {
          if (tx>=0&&ty>=0&&tx<tw&&ty<th) {
            binding->updated = true;
            if (lastXPos<0||lastXPos>=tw) lastXPos = tx;
            if (lastYPos<0||lastYPos>=th) lastYPos = ty;
            int deltaX = tx - lastXPos;
            int deltaY = ty - lastYPos;
            int d = abs(deltaX);
            if (abs(deltaY)>d) d = abs(deltaY);              
            d++;
            double dxa = (double)deltaX/d;
            double dya = (double)deltaY/d;
            double xp = lastXPos;
            double yp = lastYPos;
            for (int i = 0; i < d+1; i++) {
              int tx2 = (int)floor(xp);
              int ty2 = (int)floor(yp);
              int sw = glGetTextureWidth(stampTexture);
              int sh = glGetTextureHeight(stampTexture);
              unsigned int *sd = glGetTexturePointer(stampTexture);
              if (useStamp&&sd!=NULL&&sw>0&&sh>0) {
                for (int y = stampPY0; y < stampPY1; y++) {
                  for (int x = stampPX0; x < stampPX1; x++) {
                    int sx = x - stampPX0;
                    int sy = y - stampPY0;
                    if (sx >= 0 && sy >= 0 && sx < sw && sy < sh) {
                      unsigned int rgba2 = sd[sx+sy*sw];
                      applyPixel(x-tx+tx2,y-ty+ty2,rgba2,textureData,tw,th);
                    }
                  }
                }
              } else {
                applyPixel(tx2,ty2,rgba,textureData,tw,th);
              }
              xp += dxa;
              yp += dya;
            }
            lastXPos = tx; lastYPos = ty;
          }
        }
        if (leftDown && alt && (!lineOn)) {
          if (tx>=0&&ty>=0&&tx<tw&&ty<th) {
            if (lastXPos<0||lastXPos>=tw) lastXPos = tx;
            if (lastYPos<0||lastYPos>=th) lastYPos = ty;
            lineOn = true;
          }
        }
        if (leftDown && alt && lineOn) {
          ImVec2 one = ImVec2(floor(sp.x)+floor((lastXPos+0.5)*imageWidth/tw),floor(sp.y)+floor((lastYPos+0.5)*imageHeight/th));
          ImVec2 zwo = ImVec2(floor(sp.x)+floor((tx+0.5)*imageWidth/tw),floor(sp.y)+floor((ty+0.5)*imageHeight/th));
          ImGui::GetWindowDrawList()->AddLine(one,zwo,0xff0000ff);
        }

        if (leftDown && ctrl) {
          if (tx>=0&&ty>=0&&tx<tw&&ty<th) {
            if (lastXPos>=0&&lastYPos>=0) {
              lastXPos2 = tx;
              lastYPos2 = ty;
            }
            if (lastXPos<0||lastXPos>=tw) lastXPos = tx;
            if (lastYPos<0||lastYPos>=th) lastYPos = ty;
          }
          if (lastXPos>=0&&lastYPos>=0&&lastXPos2>=0&&lastYPos2>=0) {
            ImVec2 one = ImVec2(floor(sp.x)+floor(lastXPos*imageWidth/tw),floor(sp.y)+floor(lastYPos*imageHeight/th));
            ImVec2 zwo = ImVec2(floor(sp.x)+floor(lastXPos2*imageWidth/tw),floor(sp.y)+floor(lastYPos2*imageHeight/th)); zwo.x += 1; zwo.y += 1;
            ImGui::GetWindowDrawList()->AddRect(one,zwo,0xffffffff);
          }
          imageMouse.z = abs(lastXPos2-lastXPos);
          imageMouse.w = abs(lastYPos2-lastYPos);
        }
        if (selectedTileIndex>=0 && binding->textureAtlasTileCountX > 0) {
          int x = selectedTileIndex % binding->textureAtlasTileCountX;
          int y = selectedTileIndex / binding->textureAtlasTileCountX;
          int posX = x * binding->textureAtlasTileWidth;
          int posY = y * binding->textureAtlasTileHeight;
          imageScroll.x = posX * zoom * w / binding->originalWidth;
          imageScroll.y = posY * zoom * h / binding->originalHeight;
          selectedTileIndex=-1;
        }
        if (useStamp&&paintFlipX) {
          int sw = glGetTextureWidth(stampTexture);
          int sh = glGetTextureHeight(stampTexture);
          unsigned int *sd = glGetTexturePointer(stampTexture);
          if (sd != NULL)
          for (int y = 0; y < sh; y++) {
            for (int x = 0; x < sw/2; x++) {
              unsigned int r1 = sd[x+y*sw];
              unsigned int r2 = sd[(sw-1-x)+y*sw];
              sd[(sw-1-x)+y*sw] = r1;
              sd[x+y*sw] = r2;
            }
          }
          paintFlipX = false;
        }
        if (useStamp&&paintFlipY) {
          int sw = glGetTextureWidth(stampTexture);
          int sh = glGetTextureHeight(stampTexture);
          unsigned int *sd = glGetTexturePointer(stampTexture);
          if (sd != NULL)
          for (int y = 0; y < sh/2; y++) {
            for (int x = 0; x < sw; x++) {
              unsigned int r1 = sd[x+y*sw];
              unsigned int r2 = sd[x+(sh-1-y)*sw];
              sd[x+(sh-1-y)*sw] = r1;
              sd[x+y*sw] = r2;
            }
          }
          paintFlipY = false;
        }
        if (useStamp&&paintRotate) {
          int sw = glGetTextureWidth(stampTexture);
          int sh = glGetTextureHeight(stampTexture);
          unsigned int *sd = glGetTexturePointer(stampTexture);
          if (sd != NULL) {
            RGBAImage image;
            image.width = sh;
            image.height = sw;
            image.data = (unsigned int*)malloc(image.width*image.height*sizeof(unsigned int));
            int s;
            s = stampX0; stampX0 = stampY0; stampY0 = s; // a little bit suboptimal
            s = stampX1; stampX1 = stampY1; stampY1 = s;
            for (int y = 0; y < image.height; y++) {
              for (int x = 0; x < image.width; x++) {
                unsigned int rgba = 0x00000000;
                int sx = y;
                int sy = sh-1-x;
                if (sx >= 0 && sy >= 0 && sx < sw && sy < sh) rgba = sd[sx+sy*sw];
                image.data[x+y*image.width] = rgba;
              }
            }
            if (stampTexture != 0) glDeleteTextures(1,&stampTexture);
            glGenTextures(1,&stampTexture);
            glBindTexture(GL_TEXTURE_2D,stampTexture);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_BYTE, image.data);
            image.free();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D,0);
          }
          paintRotate = false;
        }
        if (!ImGui::GetIO().WantCaptureKeyboard && ImGui::IsKeyPressed('f')) {
          if (tx>=0&&ty>=0&&tx<tw&&ty<th) {
            fillImage(tx,ty,rgba,textureData,tw,th);
          }
        }
      }
    }
  }
  ImGui::End();
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
}

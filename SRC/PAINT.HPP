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
#ifndef __PAINT_HPP__
#define __PAINT_HPP__

extern bool justDrawBoundingBoxes;

void initGizmos();
void draw3DGrid();

void initPaint();

void paintSkyBox(class GameObject *o);
void paintTrigger(class GameObject *o, bool showModificators);
void paintMesh(class GameObject *o, bool showModificators);
void paintCamera(class GameObject *o);
void paintBillBoard(class GameObject *o, bool showModificators);
void paintLink(class GameObject *o1,class GameObject *o2);
void paintPoint(class GameObject *o, bool showModificators);
void paintTextureGrid(class GameObject *o, bool showModificators);
void paintCursor();
void paintPath(class GameObject *o, bool showModificators);
void paintHeightmap(class GameObject *o, bool showModificators);

void paintBillBoardContents();

void combineForNoRotation(class Matrix *modelViewMatrix, const class Matrix *modelViewMatrixToGetRotationFrom, bool maxScale);
void getSpline(class GameObject *o, double slider, class Vector *position, class Matrix *rotation);
                                                                                                                                                                                                        
void paintIntoTextureGrid(class FileBinding *binding, int arrayPos, int toPaint);
int getFromTextureGrid(class FileBinding *binding, int arrayPos);

#endif //__PAINT_HPP__

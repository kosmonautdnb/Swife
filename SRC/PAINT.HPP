#ifndef __PAINT_HPP__
#define __PAINT_HPP__

void initPaint();

void loadSkyBox(class GameObject *o, const char *fileName);
void paintSkyBox(class GameObject *o);

void paintTrigger(class GameObject *o, bool showModificators);

#endif //__PAINT_HPP__

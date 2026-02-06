#ifndef __OUTLINER_HPP__
#define __OUTLINER_HPP__

#include "CONFIG.HPP"

extern int outliner_width;
extern unsigned int outlinerLayers;
extern char selectedGroupName[NAMEMAX];

unsigned int layerSelection(unsigned int a, const char *dcl);

void drawOutliner();

#endif //__OUTLINER_HPP__

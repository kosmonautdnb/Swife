#ifndef __GIZMOS_HPP__
#define __GIZMOS_HPP__

extern bool USEGRID;
extern double XGRIDSIZE;
extern double ZGRIDSIZE;
extern double GRIDSTEPS;
extern unsigned int gridColor;
extern unsigned int gridTexture;

void initGizmos();
void draw3DGrid();

#endif //__GIZMOS_HPP__

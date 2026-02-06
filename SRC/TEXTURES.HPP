#ifndef __TEXTURES_HPP__
#define __TEXTURES_HPP__

bool textureButton(const char *id, unsigned int textureId, int w, int h, bool decoration);

unsigned int textureFromImageArea(const class RGBAImage *i, int x0, int y0, int x1, int y1);
void skyBoxFromHorizontalCrossImage(unsigned int t[6], const class RGBAImage *s);

#endif // __TEXTURES_HPP__

#include <i86.h>

int main(int argc, const char **argv) {
  union REGS regs;
  regs.w.ax = 0x02;
  int386(0x10, &regs, &regs);
  return 0;
}
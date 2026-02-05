#include <stdio.h>
#include <stdlib.h>
#include <i86.h>
#include <string.h>


int main(int argc, const char **argv) {
  if (argc != 4) {
    printf("usage: prepend <srcfile> <dstfile> <string>\n");
    return 0;
  }

  FILE *in = fopen(argv[1],"rb");
  if (in == NULL) {
    printf("couldn't open file for input [%s]\n",argv[1]);
    return 0;
  }
  fseek(in,0,SEEK_END);
  int fileSize = ftell(in);
  fseek(in,0,SEEK_SET);
  char *file = (char*)malloc(fileSize);
  fread(file,1,fileSize,in);
  fclose(in);

  FILE *out = fopen(argv[2],"w");
  if (out == NULL) {
    printf("couldn't open file for output [%s]\n",argv[2]);
    free(file);
    return 0;
  }

  bool lastWasLineFeed=true;
  for (int i = 0; i < fileSize; i++) {
    if (lastWasLineFeed) {
      lastWasLineFeed=false;
      fprintf(out,"%s",argv[3]);
    }
    if (file[i]!=0x0d)
      fprintf(out, "%c", file[i]); // 0x0a gets expanded to 0x0d 0a
    if (file[i] == 0x0a) {
      lastWasLineFeed = true;
    }
  }
  fclose(out);

  free(file);
  return 1;
}
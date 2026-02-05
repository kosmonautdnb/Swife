#include "LOG.HPP"
#include "../CONFIG.HPP"
#include "DOS.HPP"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef __WATCOMC__
#include <i86.h>
#endif
#ifdef __DJGPP__
#include <dos.h>
#endif

void log_setBiosCursor(int x, int y) {
    union REGS r;
    memset(&r,0,sizeof(r));
    r.w.ax = 0x200;
    r.h.bh = 0;
    r.h.dl = (unsigned char)(x&255);
    r.h.dh = (unsigned char)(y&255);
    int386(0x10, &r, &r);
}

static char temp_buffer1[4096]; // not thread safe (done this way due to WatcomC)
static char temp_buffer2[4096]; // not thread safe (done this way due to WatcomC)
static FILE *f_logging = NULL;

#define TOBUFFER(__buffer__) va_list ap;\
    va_start(ap, fmt);\
    vsprintf(__buffer__, fmt, ap);\
    va_end(ap);

void closeLogging() {
  if (f_logging != NULL) {
    fclose(f_logging);
    f_logging = NULL;
  }
}

#define GETLOGID ("["+currentTimeString()+"]").c_str()
void logIt(const char *str, bool printIt) {
  if (printIt) {
    log_setBiosCursor(0,0);
    printf(str);
  }
  if (f_logging != NULL) {fprintf(f_logging,"%s %s",GETLOGID,str); fflush(f_logging);}
}

void initLogging() {
  closeLogging();

  f_logging = fopen(logFileName,"wb"); // create new
  if (f_logging != NULL) fclose(f_logging);

  f_logging = fopen(logFileName,"a"); // append
  if (f_logging != NULL) {
    static bool first = true;
    if (first) atexit(closeLogging);
    first = false;
  }
  logIt("\n",false);
  logIt(">>>Started new log\n",false);
  logIt("---------------------------\n",false);
}

void log(const char *fmt, ...) {
  TOBUFFER(temp_buffer1);
  sprintf(temp_buffer2,"%s\n",temp_buffer1);
  logIt(temp_buffer2,true);
}

void logp(const char *fmt, ...) {
  TOBUFFER(temp_buffer1);
  sprintf(temp_buffer2,"%s\n",temp_buffer1);
  logIt(temp_buffer2,true);
}

void logi(const char *fmt, ...) {
  TOBUFFER(temp_buffer1);
  sprintf(temp_buffer2,"INFO: %s\n",temp_buffer1);
  logIt(temp_buffer2,true);
}

void logd(const char *fmt, ...) {
  TOBUFFER(temp_buffer1);
  sprintf(temp_buffer2,"DEBUG: %s\n",temp_buffer1);
  logIt(temp_buffer2,true);
}

void loge(const char *fmt, ...) {
  TOBUFFER(temp_buffer1);
  sprintf(temp_buffer2,"ERROR: %s\n",temp_buffer1);
  logIt(temp_buffer2,true);
}


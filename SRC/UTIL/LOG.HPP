#ifndef __LOG_HPP__
#define __LOG_HPP__

void initLogging();

void log(const char *fmt, ...);
void logp(const char *fmt, ...);
void logi(const char *fmt, ...);
void logd(const char *fmt, ...);
void loge(const char *fmt, ...);

#endif //__LOG_HPP__

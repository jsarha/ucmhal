#ifndef LOG_H
#define LOG_H


#include <stdio.h>
#include <stdarg.h>

static inline void debug_print(const char *file, int line, const char *tag, 
							   char lvl, const char *format, ...) {
  va_list ap;
  char buf[2048];
  int len;
  len = snprintf(buf, sizeof(buf), "%s:%d: %s %c: ", file, line, tag, lvl); 
  va_start(ap, format);
  vsnprintf(&buf[len], sizeof(buf)-len*sizeof(buf[0]), format, ap);
  va_end(ap);
  buf[sizeof(buf)-1] = '\0';
  fprintf(stderr,"%s\n", buf);
}

#define LOGV(...) debug_print(__FILE__, __LINE__, LOG_TAG, 'V',  __VA_ARGS__)
#define LOGD(...) debug_print(__FILE__, __LINE__, LOG_TAG, 'D',  __VA_ARGS__)
#define LOGW(...) debug_print(__FILE__, __LINE__, LOG_TAG, 'W',  __VA_ARGS__)
#define LOGE(...) debug_print(__FILE__, __LINE__, LOG_TAG, 'E',  __VA_ARGS__)

#endif

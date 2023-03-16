#ifndef __AC_TIME_H__
#define __AC_TIME_H__

extern "C" {
#include <libavutil/time.h>
}

/**
 * av_msleep sleep 毫秒
 * 对av_usleep 的补充
 */
inline void av_msleep(const int n) {
  av_usleep(n * 1000);
}

inline int64_t av_gettime_ms() {
  return av_gettime() / 1000;
}

#endif

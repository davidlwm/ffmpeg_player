#ifndef __AVAPIS_DASA_H__
#define __AVAPIS_DASA_H__

#include "AVAPIs/DASA.h"

#define DASA_HIGH_FPS           30
#define DASA_BTWHIGHNORMAL_FPS  20
#define DASA_NORMAL_FPS         15
#define DASA_BTWNORMALLOW_FPS   10
#define DASA_LOW_FPS            5

void *thread_DASACheck(void *arg);
int GetDASASuggestFrameRate(int *frame_rate);
int SetCleanBufferTakeAction(int sid);
bool IsCleanBufferInProgress(int sid);
int InitDasaSetting(int av_index);
int CreateDasaCheckThread();

#endif
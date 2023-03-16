//
// Created by apple on 29/12/2022.
//

#ifndef P2PPLAYER_RENDER_H
#define P2PPLAYER_RENDER_H

#include <stdlib.h>
#include "FrameInfo.h"
#include <mutex>

extern "C" {
#include <libavutil/frame.h>
}

//#ifdef __ANDROID__
//extern "C" {
//#include <SDL.h>
//#include <SDL_opengl.h>
//}
//#else
//extern "C" {
//#include <SDL2/SDL.h>
//#include <SDL2/SDL_opengl.h>
//}
//#endif

class AudioPcmSource {
public:
    virtual int FillAudio(uint8_t *buf, int len) = 0;
};

class SourceCtl {
public:
    virtual void Stop() = 0;
};


typedef void (*OnFrame)(void*, AVFrame *);

//typedef int (*FillAudio)(uint8_t *buf, int len);
typedef void (*OnAudioInfo)(void*,AudioFrameInfo_t *info, AudioPcmSource* source);


//class RenderCtl {
//public:
//    virtual void InitAudioRender(AudioFrameInfo_t *info, AudioPcmSource *source) = 0;
//
//    virtual void FillVideo(AVFrame *frame) = 0;
//};

#endif //P2PPLAYER_RENDER_H


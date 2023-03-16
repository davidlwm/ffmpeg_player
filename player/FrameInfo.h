//
// Created by apple on 29/12/2022.
//

#ifndef P2PPLAYER_FRAMEINFO_H
#define P2PPLAYER_FRAMEINFO_H
#include <stdlib.h>
typedef struct VideoFrameInfo
{
    unsigned int codec_id; //code id of ffmpeg
    unsigned char flags;	//
    unsigned int timestamp;	// Timestamp of the frame, in milliseconds
    int extradata_size;
    uint8_t extradata[256];
}VideoFrameInfo_t;

typedef struct AudioFrameInfo
{
    unsigned int codec_id; //code id of ffmpeg
    unsigned char flags;	//
    unsigned int timestamp;	// Timestamp of the frame, in milliseconds
    int channel;
    int sample_rate;
    int sample_fmt;
    int extradata_size;
    uint8_t extradata[64];
}AudioFrameInfo_t;

#endif //P2PPLAYER_FRAMEINFO_H

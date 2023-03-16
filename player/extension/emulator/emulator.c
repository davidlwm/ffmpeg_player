#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "AVAPIs_Server.h"
#include "dasa/dasa.h"
#include "emulator.h"

static char gEmulatorVideoStreamFile[EMULATOR_STREAM_NUM][128] = {
    "video_multi/beethoven_1080.multi",
    "video_multi/beethoven_720.multi",
    "video_multi/beethoven_480.multi",
    "video_multi/beethoven_360.multi",
    "video_multi/beethoven_240.multi"};

static char gEmulatorAudioStreamFile[128] = {
    "audio_raw/beethoven_8k_16bit_mono.raw"};

typedef enum
{
    WALLCLOCK_INIT,
    WALLCLOCK_START,
    WALLCLOCK_PAUSE,
    WALLCLOCK_GET,
    WALLCLOCK_SPEED_1X,
    WALLCLOCK_SPEED_2X,
    WALLCLOCK_SPEED_HALFX,
    WALLCLOCK_SPEED_QUALX
} WallClockStatus;

typedef struct _WallClock_t_
{
    unsigned int ms;
    int speed;
    int slow;
    struct timeval sysTime;
    WallClockStatus status;
} WallClock_t;

typedef struct _EmulatorVideoStream
{
    char stream_bin[128];
    char stream_info[128];
    FILE *streamBin_fp;
    FILE *streamInfo_fp;
    int fps;
}EmulatorVideoStream;

typedef struct _EmulatorAudioStream
{
    char stream_bin[128];
    FILE *streamBin_fp;
    int fps;
}EmulatorAudioStream;

typedef struct _EmulatorInfo
{
    int mode;
    int selectStream;
    int selectStreamOld;
    int selectChange;
    int threadRun;
    WallClock_t clock;
    EmulatorVideoStream videoStream[EMULATOR_STREAM_NUM];
    EmulatorAudioStream audioStream;

    Emulator_SendVideoFunc sendVideoFunc;
    Emulator_SendAudioFunc sendAudioFunc;
}EmulatorInfo;


static int gInit = 0;
static EmulatorInfo gEmulatorInfo;

int getTimevalDiff(struct timeval x , struct timeval y)
{
    int x_ms , y_ms , diff;

    x_ms = x.tv_sec*1000 + x.tv_usec/1000;
    y_ms = y.tv_sec*1000 + y.tv_usec/1000;

    diff = y_ms - x_ms;

    return diff;
}

unsigned int WallClock(WallClock_t *clock, WallClockStatus status)
{
    struct timeval now;
    unsigned int pauseTime = 0;
    gettimeofday(&now, NULL);
    
    if(status == WALLCLOCK_INIT){
        memset(clock, 0, sizeof(WallClock_t));
        clock->status = WALLCLOCK_INIT;
        clock->speed = 1;
        clock->slow = 1;
    }
    else if(status == WALLCLOCK_START){
        if(clock->status == WALLCLOCK_INIT){
            memcpy(&clock->sysTime, &now, sizeof(struct timeval));
            clock->status = WALLCLOCK_START;
            clock->ms = 0;
        }
        else if(clock->status == WALLCLOCK_PAUSE){
            pauseTime = (unsigned int)getTimevalDiff(clock->sysTime, now);
            memcpy(&clock->sysTime, &now, sizeof(struct timeval));
            clock->status = WALLCLOCK_START;
            return pauseTime;
        }
    }
    else if(status == WALLCLOCK_PAUSE){
        clock->ms += (((unsigned int)getTimevalDiff(clock->sysTime, now)*clock->speed))/clock->slow;
        memcpy(&clock->sysTime, &now, sizeof(struct timeval));
        clock->status = WALLCLOCK_PAUSE;
        return clock->ms;
    }
    else if(status == WALLCLOCK_GET){
        if(clock->status == WALLCLOCK_START){
            clock->ms += (((unsigned int)getTimevalDiff(clock->sysTime, now)*clock->speed))/clock->slow;
            memcpy(&clock->sysTime, &now, sizeof(struct timeval));
        }
        return clock->ms;
    }
    else if(status == WALLCLOCK_SPEED_1X){
        clock->speed = 1;
        clock->slow = 1;
    }
    else if(status == WALLCLOCK_SPEED_2X){
        clock->speed = 2;
        clock->slow = 1;
    }
    else if(status == WALLCLOCK_SPEED_HALFX){
        clock->speed = 1;
        clock->slow = 2;
    }
    else if(status == WALLCLOCK_SPEED_QUALX){
        clock->speed = 1;
        clock->slow = 4;
    }
    return 0;
}

int Emulator_Open(EmulatorInfo *pEmulatorInfo, int mode)
{
    int index = 0;
    char line[128] = {0}, *read = NULL;

    memset(pEmulatorInfo, 0, sizeof(EmulatorInfo));
    WallClock(&pEmulatorInfo->clock, WALLCLOCK_INIT);

    pEmulatorInfo->mode = mode;

    for(index = 0 ; index < EMULATOR_STREAM_NUM ; index++){
        sprintf(pEmulatorInfo->videoStream[index].stream_bin, "%s/frames.bin", gEmulatorVideoStreamFile[index]);
        sprintf(pEmulatorInfo->videoStream[index].stream_info, "%s/frames.info", gEmulatorVideoStreamFile[index]);

        pEmulatorInfo->videoStream[index].streamBin_fp = fopen(pEmulatorInfo->videoStream[index].stream_bin, "rb");
        if(pEmulatorInfo->videoStream[index].streamBin_fp == NULL){
            printf("[%s:%d]: Video Bin \'%s\' open error!!\n", __FUNCTION__, __LINE__, pEmulatorInfo->videoStream[index].stream_bin);
            goto EMULATOR_OPEN_ERROR;
        }
        pEmulatorInfo->videoStream[index].streamInfo_fp = fopen(pEmulatorInfo->videoStream[index].stream_info, "rb");
        if(pEmulatorInfo->videoStream[index].streamInfo_fp == NULL){
            printf("[%s:%d]: Video Info \'%s\' open error!!\n", __FUNCTION__, __LINE__, pEmulatorInfo->videoStream[index].stream_info);
            goto EMULATOR_OPEN_ERROR;
        }

        if((read = fgets(line, sizeof(line), pEmulatorInfo->videoStream[index].streamInfo_fp)) == NULL){
            printf("[%s:%d]: Read Video Info \'%s\' error!!\n", __FUNCTION__, __LINE__, pEmulatorInfo->videoStream[index].stream_info);
            goto EMULATOR_OPEN_ERROR;
        }
        sscanf(line, "FPS %d\n", &pEmulatorInfo->videoStream[index].fps);
    }

    sprintf(pEmulatorInfo->audioStream.stream_bin, "%s", gEmulatorAudioStreamFile);
    pEmulatorInfo->audioStream.streamBin_fp = fopen(pEmulatorInfo->audioStream.stream_bin, "rb");
    if(pEmulatorInfo->audioStream.streamBin_fp == NULL){
        printf("[%s:%d]: Audio Bin \'%s\' open error!!\n", __FUNCTION__, __LINE__, pEmulatorInfo->audioStream.stream_bin);
        goto EMULATOR_OPEN_ERROR;
    }

    return 0;

EMULATOR_OPEN_ERROR:
    for(index = 0 ; index < EMULATOR_STREAM_NUM ; index++){
        if(pEmulatorInfo->videoStream[index].streamBin_fp != NULL){
            fclose(pEmulatorInfo->videoStream[index].streamBin_fp);
            pEmulatorInfo->videoStream[index].streamBin_fp = NULL;
        }
        if(pEmulatorInfo->videoStream[index].streamInfo_fp != NULL){
            fclose(pEmulatorInfo->videoStream[index].streamInfo_fp);
            pEmulatorInfo->videoStream[index].streamInfo_fp = NULL;
        }
    }
    if(pEmulatorInfo->audioStream.streamBin_fp != NULL){
        fclose(pEmulatorInfo->audioStream.streamBin_fp);
        pEmulatorInfo->audioStream.streamBin_fp = NULL;
    }
    return -1;
}

int Emulator_Close(EmulatorInfo *pEmulatorInfo)
{
    int index = 0;

    for(index = 0 ; index < EMULATOR_STREAM_NUM ; index++){
        if(pEmulatorInfo->videoStream[index].streamBin_fp != NULL){
            fclose(pEmulatorInfo->videoStream[index].streamBin_fp);
            pEmulatorInfo->videoStream[index].streamBin_fp = NULL;
        }
        if(pEmulatorInfo->videoStream[index].streamInfo_fp != NULL){
            fclose(pEmulatorInfo->videoStream[index].streamInfo_fp);
            pEmulatorInfo->videoStream[index].streamInfo_fp = NULL;
        }
    }
    if(pEmulatorInfo->audioStream.streamBin_fp != NULL){
        fclose(pEmulatorInfo->audioStream.streamBin_fp);
        pEmulatorInfo->audioStream.streamBin_fp = NULL;
    }

    return -1;
}

int Emulator_Rewind(EmulatorInfo *pEmulatorInfo)
{
    int index = 0;
    char line[128] = {0}, *read = NULL;

    for(index = 0 ; index < EMULATOR_STREAM_NUM ; index++){
        rewind(pEmulatorInfo->videoStream[index].streamBin_fp);
        rewind(pEmulatorInfo->videoStream[index].streamInfo_fp);

        if((read = fgets(line, sizeof(line), pEmulatorInfo->videoStream[index].streamInfo_fp)) == NULL){
            printf("[%s:%d]: Read Video Info \'%s\' error!!\n", __FUNCTION__, __LINE__, pEmulatorInfo->videoStream[index].stream_info);
            return -1;
        }
        sscanf(line, "FPS %d\n", &pEmulatorInfo->videoStream[index].fps);
    }
    rewind(pEmulatorInfo->audioStream.streamBin_fp);

    return 0;
}

int Emulator_ReadVideo(EmulatorInfo *pEmulatorInfo, int streamID, char* buf, int size, int* frameType)
{
    EmulatorVideoStream* pVideoStream = &(pEmulatorInfo->videoStream[streamID]);
    char line[128] = {0}, *read = NULL, frame_type[2] = {0};
    int frame_pos = 0, frame_size = 0, nRet = 0;

    if((read = fgets(line, sizeof(line), pVideoStream->streamInfo_fp)) == NULL) {
        //TODO : Rewind
        return 0;
    }

    sscanf(line, "%c %d %d\n", frame_type, &frame_pos, &frame_size);

    if(frame_size > size){
        return -1;
    }
    *frameType = (strcmp(frame_type, "I") == 0 ? IPC_FRAME_FLAG_IFRAME : IPC_FRAME_FLAG_PBFRAME);

    fseek(pVideoStream->streamBin_fp, frame_pos*sizeof(char), SEEK_SET);
    nRet = fread(buf, 1, frame_size, pVideoStream->streamBin_fp);

    return nRet;
}

int Emulator_ReadVideoOnlyInfo(EmulatorInfo *pEmulatorInfo, int streamID, int* frameType)
{
    EmulatorVideoStream* pVideoStream = &(pEmulatorInfo->videoStream[streamID]);
    char line[128] = {0}, *read = NULL, frame_type[2] = {0};
    int frame_pos = 0, frame_size = 0;

    if((read = fgets(line, sizeof(line), pVideoStream->streamInfo_fp)) == NULL) {
        //TODO : Rewind
        return 0;
    }

    sscanf(line, "%c %d %d\n", frame_type, &frame_pos, &frame_size);
    *frameType = (strcmp(frame_type, "I") == 0 ? IPC_FRAME_FLAG_IFRAME : IPC_FRAME_FLAG_PBFRAME);

    return 1;
}

int Emulator_ReadAudio(EmulatorInfo *pEmulatorInfo, char* buf, int size)
{
    EmulatorAudioStream* pAudioStream = &(pEmulatorInfo->audioStream);
    int nRet = 0;

    if(feof(pAudioStream->streamBin_fp))
        return 0;

    nRet = fread(buf, 1, size, pAudioStream->streamBin_fp);

    return nRet;
}

void *Emulator_StreamoutThread(void *arg)
{
    EmulatorInfo *pEmulatorInfo = (EmulatorInfo *)arg;
    struct timeval tv, tv2;
    int recycle = 1, index = 0, mode = pEmulatorInfo->mode, sleepTime = 0, roundtime = 0, frameType = 0, ret = 0;
    int readVideo = 0, readAudio = 0, audioTimestamp = 0, videoTimestamp = 0, playTime = 0, videofps = 0, audiofps = 0, videoCount = 0;
    int audioSleepTime = 0, videoSleepTime = 0, selectStreamOld = 0;
    char vBuf[VIDEO_BUF_SIZE] = {0};
    char aBuf[AUDIO_BUF_SIZE] = {0};
    unsigned int totalPlayTime = 0;

    videofps = pEmulatorInfo->videoStream[0].fps;
    audiofps = AUDIO_FPS;

    //printf("Emulator_StreamoutThread: Start\n");
    WallClock(&pEmulatorInfo->clock, WALLCLOCK_SPEED_1X);
    WallClock(&pEmulatorInfo->clock, WALLCLOCK_START);

    while(pEmulatorInfo->threadRun)
    {
        gettimeofday(&tv, NULL);
        if(recycle){
            printf("[%s:%d] Emulator_Rewind playTime[%d]\n", __FUNCTION__, __LINE__, playTime);
            if(Emulator_Rewind(pEmulatorInfo) < 0){
                printf("[%s:%d] rewind error, exit thread\n", __FUNCTION__, __LINE__);
                goto EXIT_STREAMOUT_EMULATOR;
            }
            totalPlayTime += WallClock(&pEmulatorInfo->clock, WALLCLOCK_GET);

            WallClock(&pEmulatorInfo->clock, WALLCLOCK_INIT);
            WallClock(&pEmulatorInfo->clock, WALLCLOCK_SPEED_1X);
            WallClock(&pEmulatorInfo->clock, WALLCLOCK_START);
            audioTimestamp = 0;
            videoTimestamp = 0;
            recycle = 0;
        }

        playTime = WallClock(&pEmulatorInfo->clock, WALLCLOCK_GET);
        //printf("playTime[%d] videoTimestamp[%d] audioTimestamp[%d]\n", playTime, videoTimestamp, audioTimestamp);
        readVideo = readAudio = 0;
        if(videoTimestamp <= playTime){
            readVideo = 1;
            videoTimestamp += (1000/videofps);
            //handle sleep 33.333
            if(++videoCount == 3){
                videoTimestamp++;
                videoCount = 0;
            }
        }
        videoSleepTime = (videoTimestamp - playTime);
        
        if(audioTimestamp <= playTime){
            readAudio = 1;
            audioTimestamp += (1000/audiofps);
        }
        audioSleepTime = (audioTimestamp - playTime);

        sleepTime = audioSleepTime <= videoSleepTime ? audioSleepTime : videoSleepTime;

        if(readVideo){
            //Read Video
            if(mode == EMULATOR_MODE_MULTISTREAM){
                for(index = 0; index < EMULATOR_STREAM_NUM ; index++){
                    ret = Emulator_ReadVideo(pEmulatorInfo, index, vBuf, VIDEO_BUF_SIZE, &frameType);
                    if(ret == 0){
                        recycle = 1;
                    }
                    else if(ret == -1){
                        printf("[%s:%d] video buffer too small\n", __FUNCTION__, __LINE__);
                    }
                    else if(ret > 0){
                        //Send Video
                        if(pEmulatorInfo->sendVideoFunc != NULL){
                            pEmulatorInfo->sendVideoFunc(videoTimestamp+totalPlayTime, vBuf, ret, frameType);
                        }
                        if(frameType == IPC_FRAME_FLAG_IFRAME){
                            Streamout_SetCacheIFrame(vBuf, ret);
                        }
                    }
                    else{
                        printf("[%s:%d] read error ret[%d], exit thread\n", __FUNCTION__, __LINE__, ret);
                        goto EXIT_STREAMOUT_EMULATOR;
                    }
                }
            }
            else{
                for(index = 0; index < EMULATOR_STREAM_NUM ; index++){
                    if(pEmulatorInfo->selectStream != index && pEmulatorInfo->selectStreamOld != index){
                        ret = Emulator_ReadVideoOnlyInfo(pEmulatorInfo, index, &frameType);
                        if(ret == 0){
                            recycle = 1;
                        }
                        continue;
                    }

                    ret = Emulator_ReadVideo(pEmulatorInfo, index, vBuf, VIDEO_BUF_SIZE, &frameType);
                    if(ret == 0){
                        recycle = 1;
                    }
                    else if(ret == -1){
                        printf("[%s:%d] video buffer too small\n", __FUNCTION__, __LINE__);
                    }
                    else if(ret > 0){
                        if(pEmulatorInfo->selectChange){
                            if(frameType == IPC_FRAME_FLAG_IFRAME){
                                selectStreamOld = pEmulatorInfo->selectStreamOld;
                                pEmulatorInfo->selectStreamOld = pEmulatorInfo->selectStream;
                                pEmulatorInfo->selectChange = 0;

                                if(index == selectStreamOld)
                                    continue;
                            }
                            else{
                                if(index == pEmulatorInfo->selectStream)
                                    continue;
                            }
                        }

                        //Send Video
                        if(pEmulatorInfo->sendVideoFunc != NULL)
                            pEmulatorInfo->sendVideoFunc(videoTimestamp+totalPlayTime, vBuf, ret, frameType);
                    }
                    else{
                        printf("[%s:%d] read error ret[%d], exit thread\n", __FUNCTION__, __LINE__, ret);
                        goto EXIT_STREAMOUT_EMULATOR;
                    }
                }
            }
        }

        //Read Audio
        if(readAudio){
            ret = Emulator_ReadAudio(pEmulatorInfo, aBuf, AUDIO_FRAME_SIZE);
            if(ret == 0){
                recycle = 1;
            }
            else if(ret > 0){
                //Send Audio
                if(pEmulatorInfo->sendAudioFunc != NULL)
                    pEmulatorInfo->sendAudioFunc(audioTimestamp+totalPlayTime, aBuf, ret);
            }
            else{
                printf("[%s:%d] read error ret[%d], exit thread\n", __FUNCTION__, __LINE__, ret);
                goto EXIT_STREAMOUT_EMULATOR;
            }
        }
        gettimeofday(&tv2, NULL);
        if(recycle == 0){
            roundtime = getTimevalDiff(tv, tv2);
            sleepTime -= roundtime;
            if(sleepTime > 0){
                usleep((sleepTime-1)*1000);
            }
        }
    }

EXIT_STREAMOUT_EMULATOR:
    pEmulatorInfo->threadRun = 2;
    pthread_exit(0);
}

int Emulator_Initialize(int mode, int initStream, Emulator_SendVideoFunc sendVideoFunc, Emulator_SendAudioFunc sendAudioFunc)
{
    int nRet = 0;
    pthread_t ThreadEmulator_ID;

    if(mode < EMULATOR_MODE_SINGLESTREAM || mode > EMULATOR_MODE_MULTISTREAM)
        return -1;

    if(gInit)
        return 0;

    //Open File for Read
    if(Emulator_Open(&gEmulatorInfo, mode) < 0){
        printf("[%s:%d] Emulator_Open error!!\n", __FUNCTION__, __LINE__);
        return -1;
    }
    gEmulatorInfo.mode = mode;
    gEmulatorInfo.selectStream = initStream;
    gEmulatorInfo.selectStreamOld = initStream;
    gEmulatorInfo.selectChange = 0;
    gEmulatorInfo.sendVideoFunc = sendVideoFunc;
    gEmulatorInfo.sendAudioFunc = sendAudioFunc;

    gEmulatorInfo.threadRun = 1;
    if((nRet = pthread_create(&ThreadEmulator_ID, NULL, &Emulator_StreamoutThread, (void*)&gEmulatorInfo))){
        printf("[%s:%d] pthread_create ThreadEmulator_ID nRet[%d]\n", __FUNCTION__, __LINE__, nRet);
        return -1;
    }
    pthread_detach(ThreadEmulator_ID);

    gInit = 1;

    return 0;
}

int Emulator_DeInitialize()
{
    if(gInit)
        return 0;

    gEmulatorInfo.threadRun = 0;
    while(gEmulatorInfo.threadRun != 2){
        usleep(100000);
    }

    Emulator_Close(&gEmulatorInfo);

    gInit = 0;

    return 0;
}

int Emulator_ChangeStream(int selectStream)
{
    gEmulatorInfo.selectStreamOld = gEmulatorInfo.selectStream;
    gEmulatorInfo.selectStream = selectStream;
    gEmulatorInfo.selectChange = 1;

    return 0;
}


int gCacheSize = 0;
char gIFrameCache[VIDEO_BUF_SIZE] = {0};

int Streamout_SetCacheIFrame(char* buf, int size)
{
    if(size > VIDEO_BUF_SIZE){
        printf("Streamout_SetCacheIFrame gIFrameCache too small\n");
        return -1;
    }

    gCacheSize = size;
    memcpy(gIFrameCache, buf, size);

    return 0;
}

int Streamout_GetCacheIFrame(char* buf)
{
    if(gCacheSize == 0)
        return -1;

    buf = gIFrameCache;

    return gCacheSize;
}

int CreateEmulatorStream(Emulator_SendVideoFunc sendVideoFunc, Emulator_SendAudioFunc sendAudioFunc)
{
    int ret = 0;
    ret = Emulator_Initialize(EMULATOR_MODE_SINGLESTREAM, AV_DASA_LEVEL_QUALITY_LOW, sendVideoFunc, sendAudioFunc);
    return ret;
}

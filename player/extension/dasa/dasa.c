#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "AVAPIs_Server.h"
#include "dasa.h"

#if ENABLE_DASA

extern AV_Client gClientInfo[MAX_CLIENT_NUMBER];
extern int gProcessRun;
extern int gOnlineNum;

static int gDASALevel = AV_DASA_LEVEL_QUALITY_HIGH;

int GetDASASuggestFrameRate(int *frame_rate) {
    if(gDASALevel == AV_DASA_LEVEL_QUALITY_HIGH){
        *frame_rate = DASA_HIGH_FPS;
    } else if(gDASALevel == AV_DASA_LEVEL_QUALITY_BTWHIGHNORMAL) {
        *frame_rate = DASA_BTWHIGHNORMAL_FPS;
    } else if(gDASALevel == AV_DASA_LEVEL_QUALITY_NORMAL) {
        *frame_rate = DASA_NORMAL_FPS;
    } else if(gDASALevel == AV_DASA_LEVEL_QUALITY_BTWNORMALLOW) {
        *frame_rate = DASA_BTWNORMALLOW_FPS;
    } else if(gDASALevel == AV_DASA_LEVEL_QUALITY_LOW) {
        *frame_rate = DASA_LOW_FPS;
    }
    return 0;
}


void *thread_DASACheck(void *arg)
{
    int ret = 0, lock_ret = 0, i = 0, doCleanBuffer = 0, avIndex = 0, clientEnableVideoCount = 0;
    int DASALevel = 0, DASALevelMAX = AV_DASA_LEVEL_QUALITY_HIGH;
    unsigned char bEnableVideo = 0;

    while(gProcessRun)
    {
        for(i = 0; i < MAX_CLIENT_NUMBER ; i++){
            lock_ret = pthread_rwlock_rdlock(&gClientInfo[i].sLock);
		    if(lock_ret)
			    printf("Acquire SID %d rdlock error, ret = %d\n", i, lock_ret);

            bEnableVideo = gClientInfo[i].bEnableVideo;
            doCleanBuffer = gClientInfo[i].doCleanBuffer;
            avIndex = gClientInfo[i].avIndex;

            lock_ret = pthread_rwlock_unlock(&gClientInfo[i].sLock);
            if(lock_ret)
                printf("Acquire SID %d rdlock error, ret = %d\n", i, lock_ret);

            if(bEnableVideo){
                clientEnableVideoCount++;
                DASALevel = avDASACheck(avIndex);
                if(DASALevel >= DASALevelMAX)
                    DASALevelMAX = DASALevel;
        	}

            if(doCleanBuffer){
                printf("[%s:%d] SID[%d] avIndex[%d] avServResetBuffer\n", __FUNCTION__, __LINE__, i, avIndex);
                avServResetBuffer(avIndex, RESET_ALL, 0);
                printf("[%s:%d] SID[%d] avIndex[%d] ret[%d] avServResetBuffer done\n", __FUNCTION__, __LINE__, i, avIndex, ret);

                lock_ret = pthread_rwlock_wrlock(&gClientInfo[i].sLock);
				if(lock_ret)
					printf("Acquire SID %d rwlock error, ret = %d\n", i, lock_ret);

                gClientInfo[i].doCleanBuffer = 0;
				gClientInfo[i].doCleanBufferDone = 1;

				//release lock
				lock_ret = pthread_rwlock_unlock(&gClientInfo[i].sLock);
				if(lock_ret)
					printf("Release SID %d rwlock error, ret = %d\n", i, lock_ret);
            }
        }

        if((gOnlineNum == 0 || clientEnableVideoCount == 0) && gDASALevel != AV_DASA_LEVEL_QUALITY_LOW){
            gDASALevel = AV_DASA_LEVEL_QUALITY_LOW;
#if ENABLE_EMULATOR
            Emulator_ChangeStream(gDASALevel);
#endif
            printf("No client connect, set gDASALevel to AV_DASA_LEVEL_QUALITY_LOW\n");
        }

        if((gOnlineNum > 0 && clientEnableVideoCount > 0) && gDASALevel != DASALevelMAX){
            if(DASALevelMAX > gDASALevel)
                printf("Decrease Quality to DASALevelMAX[%d]\n", DASALevelMAX);
            else if(DASALevelMAX < gDASALevel)
                printf("Increase Quality to DASALevelMAX[%d]\n", DASALevelMAX);
            gDASALevel = DASALevelMAX;
#if ENABLE_EMULATOR
            Emulator_ChangeStream(gDASALevel);
#endif
        }

        DASALevelMAX = AV_DASA_LEVEL_QUALITY_HIGH;
        clientEnableVideoCount = 0;

        sleep(1);
    }

    pthread_exit(0);
}


int SetCleanBufferTakeAction(int sid) {
    int lock_ret = 0;
    lock_ret = pthread_rwlock_wrlock(&gClientInfo[sid].sLock);
    if(lock_ret)
        printf("Acquire SID %d rwlock error, ret = %d\n", sid, lock_ret);

    gClientInfo[sid].doCleanBuffer = 1;
    gClientInfo[sid].doCleanBufferDone = 0;
    gClientInfo[sid].waitKeyFrame = 1;

    //release lock
    lock_ret = pthread_rwlock_unlock(&gClientInfo[sid].sLock);
    if(lock_ret)
        printf("Release SID %d rwlock error, ret = %d\n", sid, lock_ret);

    return 0;
}

bool IsCleanBufferInProgress(int sid) {
    int lock_ret = 0;
    bool ret = false;

    lock_ret = pthread_rwlock_rdlock(&gClientInfo[sid].sLock);
    if(lock_ret)
        printf("Acquire SID %d rdlock error, ret = %d\n", sid, lock_ret);

    int doCleanBuffer = gClientInfo[sid].doCleanBuffer;
    int doCleanBufferDone = gClientInfo[sid].doCleanBufferDone;

    if(doCleanBuffer && !doCleanBufferDone){
        printf("SID[%d] avIndex[%d] doCleanBuffer[%d] doCleanBufferDone[%d]\n", sid, gClientInfo[sid].avIndex, doCleanBuffer, doCleanBufferDone);
        ret = true;
    }
    
    lock_ret = pthread_rwlock_unlock(&gClientInfo[sid].sLock);
    if(lock_ret)
        printf("Acquire SID %d rdlock error, ret = %d\n", sid, lock_ret);

    return ret;
}

int InitDasaSetting(int av_index) {
    int ret = 0;
    avServSetResendSize(av_index, 1024*4);
    int nCleanBufferCondition = 5, nCleanBufferRatio = 70, nAdjustmentKeepTime = 5, nIncreaseQualityCond = 10, nDecreaseRatio = 50;
    ret = avDASASetting(av_index, 1, nCleanBufferCondition, nCleanBufferRatio, nAdjustmentKeepTime, nIncreaseQualityCond, nDecreaseRatio, AV_DASA_LEVEL_QUALITY_LOW);
    if(ret < 0)
        printf("avDASASetting Error avIndex[%d] ret[%d]\n", av_index, ret);

    return 0;
}

int CreateDasaCheckThread() {
    int ret = 0;
    pthread_t ThreadDASACheck_ID;
	if((ret = pthread_create(&ThreadDASACheck_ID, NULL, &thread_DASACheck, NULL)))
	{
		printf("pthread_create failed ret=%d\n", ret);
        return -1;
	}
	pthread_detach(ThreadDASACheck_ID);
    return 0;
}

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "AVAPIs_Server.h"

extern AV_Client gClientInfo[MAX_CLIENT_NUMBER];
extern char gVideoFn[128];
extern char gAudioFn[128];

extern int ExPasswordAuthCallBackFn(const char *account, char *pwd, unsigned int pwd_buf_size);

static int gbSearchEvent = 0;
static SMsgAVIoctrlListEventResp *gEventList = NULL;

void *thread_PlayBack(void *arg)
{
    int SID = *((int *)arg);
    free(arg);
    AV_Client *p = &gClientInfo[SID];

    AVServStartInConfig avStartInConfig;
    AVServStartOutConfig avStartOutConfig;

    avStartInConfig.cb               = sizeof(AVServStartInConfig);
    avStartInConfig.iotc_session_id  = SID;
    avStartInConfig.iotc_channel_id  = p->playBackCh;
    avStartInConfig.timeout_sec      = 30;
    avStartInConfig.password_auth    = &ExPasswordAuthCallBackFn;
    avStartInConfig.server_type      = SERVTYPE_STREAM_SERVER;
    avStartInConfig.resend           = ENABLE_RESEND;
#if ENABLE_DTLS
    avStartInConfig.security_mode = AV_SECURITY_DTLS; // Enable DTLS, otherwise use AV_SECURITY_SIMPLE
#else
    avStartInConfig.security_mode = AV_SECURITY_SIMPLE;
#endif

    avStartOutConfig.cb              = sizeof(AVServStartOutConfig);

    int avIndex = avServStartEx(&avStartInConfig, &avStartOutConfig);

    if(avIndex < 0)
    {
        printf("avServStart2 failed SID[%d] code[%d]!!!\n", SID, avIndex);
        pthread_exit(0);
    }
    printf("[%s:%d] SID[%d] avIndex[%d] start\n", __FUNCTION__, __LINE__, SID, avIndex);

    /*
     *  search play back by p->playRecord.stTimeDay
     */
    FILE *vFp = fopen(gVideoFn, "rb");
    if(vFp == NULL)
    {
        printf("[%s:%d] fopen %s error\n", __FUNCTION__, __LINE__, gVideoFn);
        pthread_exit(0);
    }
    FILE *aFp = fopen(gAudioFn, "rb");
    if(aFp == NULL)
    {
        printf("[%s:%d] fopen %s error\n", __FUNCTION__, __LINE__, gAudioFn);
        pthread_exit(0);
    }

    char videoBuf[VIDEO_BUF_SIZE];
    char audioBuf[AUDIO_BUF_SIZE];
    int videoSize = fread(videoBuf, 1, VIDEO_BUF_SIZE, vFp);
    int sendCnt = 0, ret;
    FRAMEINFO_t videoInfo, audioInfo;
    memset(&videoInfo, 0, sizeof(FRAMEINFO_t));
    memset(&audioInfo, 0, sizeof(FRAMEINFO_t));
    videoInfo.codec_id = MEDIA_CODEC_VIDEO_H264;
    videoInfo.flags = IPC_FRAME_FLAG_IFRAME;
    audioInfo.codec_id = AUDIO_CODEC;
    audioInfo.flags = (AUDIO_SAMPLE_8K << 2) | (AUDIO_DATABITS_16 << 1) | AUDIO_CHANNEL_MONO;
    SMsgAVIoctrlPlayRecordResp resp;
    resp.command = AVIOCTRL_RECORD_PLAY_START;
    int lock_ret;

	while(1)
	{
		//get reader lock
		lock_ret = pthread_rwlock_rdlock(&gClientInfo[SID].sLock);
		if(lock_ret)
			printf("Acquire SID %d rdlock error, ret = %d\n", SID, lock_ret);
		if(p->bStopPlayBack == 1)
		{
			//release reader lock
			lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Acquire SID %d rdlock error, ret = %d\n", SID, lock_ret);
			printf("SID %d:%d bStopPlayBack bStopPlayBack\n", SID, avIndex );
			break;
		}
		if(gClientInfo[SID].bPausePlayBack)
		{
			//release reader lock
			lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Acquire SID %d rdlock error, ret = %d\n", SID, lock_ret);
			usleep(50000);
			continue;
		}
		if(sendCnt++ % 20 == 0) // by fps
		{
			ret = avSendFrameData(avIndex, videoBuf, videoSize, (void *)&videoInfo, sizeof(FRAMEINFO_t));
			//release reader lock
			lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Acquire SID %d rdlock error, ret = %d\n", SID, lock_ret);
			if(ret < 0 && ret != AV_ER_EXCEED_MAX_SIZE)
			{
				printf("avSendFrameData error ret[%d]\n", ret);
				break;
			}
		}
		else
		{
			int size = fread(audioBuf, 1, AUDIO_FRAME_SIZE, aFp);
			if(size <= 0)
			{
				printf("fread end\n");
				//release reader lock
				lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
				if(lock_ret)
					printf("Acquire SID %d rdlock error, ret = %d\n", SID, lock_ret);
				break;
			}
			ret = avSendAudioData(avIndex, audioBuf, size, (void *)&audioInfo, sizeof(FRAMEINFO_t));
			//release reader lock
			lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Acquire SID %d rdlock error, ret = %d\n", SID, lock_ret);
			if(ret < 0)
			{
				printf("avSendAudioData error ret[%d]\n", ret);
				break;
			}
		}

		if(sendCnt == 500)
		{
			printf("[%s:%d] SID[%d] avIndex[%d] playback end\n", __FUNCTION__, __LINE__, SID, avIndex);
			memset(&resp, 0, sizeof(SMsgAVIoctrlPlayRecordResp));
			resp.command = AVIOCTRL_RECORD_PLAY_END;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP, (char *)&resp, sizeof(SMsgAVIoctrlPlayRecordResp));
			//get write lock
			lock_ret = pthread_rwlock_wrlock(&gClientInfo[SID].sLock);
			if(lock_ret)
				printf("Acquire SID %d rwlock error, ret = %d\n", SID, lock_ret);

			gClientInfo[SID].bStopPlayBack = 1;
			lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
			break;
		}

		usleep(20000); // by fps
	}

	avServStop(avIndex);
	fclose(vFp);
	fclose(aFp);
	lock_ret = pthread_rwlock_wrlock(&gClientInfo[SID].sLock);
	if(lock_ret)
		printf("Acquire SID %d rwlock error, ret = %d\n", SID, lock_ret);
	gClientInfo[SID].playBackCh = -1;
	lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
	printf("[%s:%d] SID[%d] avIndex[%d] exit\n", __FUNCTION__, __LINE__, SID, avIndex);
	pthread_exit(0);
}

static int generateEventList(STimeDay* start_time, int count)
{
    int i = 0;
    time_t rawtime;
    struct tm timeinfo;

    if(gEventList != NULL){
        free(gEventList);
        gEventList = NULL;
    }

    gEventList = (SMsgAVIoctrlListEventResp *)malloc(sizeof(SMsgAVIoctrlListEventResp)+sizeof(SAvEvent)*count);
    if(gEventList == NULL)
        return -1;

    memset(gEventList, 0, sizeof(SMsgAVIoctrlListEventResp));
    gEventList->total = count;
    gEventList->index = 0;
    gEventList->endflag = 1;
    gEventList->count = count;

    timeinfo.tm_year = start_time->year - 1900;
    timeinfo.tm_mon = start_time->month - 1;
    timeinfo.tm_mday = start_time->day;
    timeinfo.tm_hour = start_time->hour;
    timeinfo.tm_min = start_time->minute;
    timeinfo.tm_sec = start_time->second;
    rawtime = mktime(&timeinfo);

    for(i = 0 ; i < gEventList->count ; i++)
    {
        rawtime += 60;
        localtime_r(&rawtime, &timeinfo);
        gEventList->stEvent[i].stTime.year = timeinfo.tm_year+1900;
        gEventList->stEvent[i].stTime.month = timeinfo.tm_mon+1;
        gEventList->stEvent[i].stTime.day = timeinfo.tm_mday;
        gEventList->stEvent[i].stTime.wday = timeinfo.tm_wday;
        gEventList->stEvent[i].stTime.hour = timeinfo.tm_hour;
        gEventList->stEvent[i].stTime.minute = timeinfo.tm_min;
        gEventList->stEvent[i].stTime.second = timeinfo.tm_sec;
        gEventList->stEvent[i].event = AVIOCTRL_EVENT_MOTIONDECT;
        gEventList->stEvent[i].status = 0;
    }

    return 0;
}

int Handle_IOTYPE_USER_IPCAM_LISTEVENT_REQ(int av_index, char* req_buf)
{
    int i = 0, count = 0, packet_index = 0, resp_size = 0;
    unsigned char end_flag = 0;
    char buf[AV_MAX_IOCTRL_DATA_SIZE] = {0};
    SMsgAVIoctrlListEventReq *req = (SMsgAVIoctrlListEventReq *)req_buf;
    SMsgAVIoctrlListEventResp *resp = (SMsgAVIoctrlListEventResp*)buf;

    printf("[%s:%d] av_index[%d] Receive IOCTRL command: IOTYPE_USER_IPCAM_LISTEVENT_REQ\n", __FUNCTION__, __LINE__, av_index);
    printf("Request list from stStartTime[%04d:%02d:%02d-%02d:%02d:%02d] to stEndTime[%04d:%02d:%02d-%02d:%02d:%02d]\n",
        req->stStartTime.year, req->stStartTime.month, req->stStartTime.day, req->stStartTime.hour, req->stStartTime.minute, req->stStartTime.second,
        req->stEndTime.year, req->stEndTime.month, req->stEndTime.day, req->stEndTime.hour, req->stEndTime.minute, req->stEndTime.second);

    /*
     *  Example : Sample code just do search event list once, actually must renew when got this command
     *            Generate 100 events into list
     */
    if(gbSearchEvent == 0)
    {
        if(generateEventList(&(req->stStartTime), 100) < 0)
        {
            printf("[%s:%d] av_index[%d] generateEventList error\n", __FUNCTION__, __LINE__, av_index);
            return -1;
        }
        gbSearchEvent = 1;
    }

    /*
     *  At most 84 events in a response, because AV_MAX_IOCTRL_DATA_SIZE is 1024
     */
    while(i < gEventList->total)
    {
        if(gEventList->total - i > 84)
        {
            count = 84;
        }
        else
        {
            count = gEventList->total-i;
            end_flag = 1;
        }

        resp->total = gEventList->total;
        resp->index = packet_index;
        resp->endflag = end_flag;
        resp->count = count;
        memcpy(&resp->stEvent[0], &gEventList->stEvent[i], sizeof(SAvEvent)*count);

        resp_size = sizeof(SMsgAVIoctrlListEventResp)+sizeof(SAvEvent)*(count-1);
        if(avSendIOCtrl(av_index, IOTYPE_USER_IPCAM_LISTEVENT_RESP, (char *)resp, resp_size) < 0)
            return -1;

        printf("[%s:%d] av_index[%d] send resp : packet index[%d] with data count[%d]\n", __FUNCTION__, __LINE__, av_index, resp->index, resp->count);

        i += count;
        packet_index++;
    }

    return 0;
}

int Handle_IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL(int sid, int av_index, char* req_buf)
{
    int i = 0, *tmp_sid = NULL, ret = 0, lock_ret = 0;
    pthread_t ThreadID;
    SMsgAVIoctrlPlayRecord *req = (SMsgAVIoctrlPlayRecord *)req_buf;
    SMsgAVIoctrlPlayRecordResp resp;

    printf("[%s:%d] av_index[%d] Receive IOCTRL command: IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL cmd[%d]\n", __FUNCTION__, __LINE__, av_index, req->command);
    printf("stTimeDay[%04d:%02d:%02d-%02d:%02d:%02d]\n", req->stTimeDay.year, req->stTimeDay.month, req->stTimeDay.day,
                                                         req->stTimeDay.hour, req->stTimeDay.minute, req->stTimeDay.second);

    if(req->command == AVIOCTRL_RECORD_PLAY_START)
    {
        memcpy(&gClientInfo[sid].playRecord, req, sizeof(SMsgAVIoctrlPlayRecord));
        //get writer lock
        lock_ret = pthread_rwlock_wrlock(&gClientInfo[sid].sLock);
        if(lock_ret)
            printf("Acquire SID %d rwlock error, ret = %d\n", sid, lock_ret);

        resp.command = AVIOCTRL_RECORD_PLAY_START;
        printf("playback on playBackCh[%d]\n",gClientInfo[sid].playBackCh);
        if(gClientInfo[sid].playBackCh < 0)
        {
            gClientInfo[sid].bPausePlayBack = 0;
            gClientInfo[sid].bStopPlayBack = 0;
            gClientInfo[sid].playBackCh = IOTC_Session_Get_Free_Channel(sid);
            resp.result = gClientInfo[sid].playBackCh;
        }
        else
        {
            resp.result = -1;
        }

        //release lock
        lock_ret = pthread_rwlock_unlock(&gClientInfo[sid].sLock);
        if(lock_ret)
            printf("Release SID %d rwlock error, ret = %d\n", sid, lock_ret);

        if(resp.result >= 0)
        {
            printf("start playback result[%d]\n", resp.result);
            for(i = 0 ; i < gEventList->count ; i++)
            {
                if(req->stTimeDay.year == gEventList->stEvent[i].stTime.year &&
                   req->stTimeDay.month == gEventList->stEvent[i].stTime.month &&
                   req->stTimeDay.day == gEventList->stEvent[i].stTime.day &&
                   req->stTimeDay.hour == gEventList->stEvent[i].stTime.hour &&
                   req->stTimeDay.minute == gEventList->stEvent[i].stTime.minute &&
                   req->stTimeDay.second == gEventList->stEvent[i].stTime.second)
                gEventList->stEvent[i].status = 1;
            }

            tmp_sid = (int *)malloc(sizeof(int));
            *tmp_sid = sid;
            if((ret = pthread_create(&ThreadID, NULL, &thread_PlayBack, (void *)tmp_sid)))
            {
                printf("pthread_create thread_PlayBack ret[%d]\n", ret);
                resp.result = -1;
            }
            else
            {
                pthread_detach(ThreadID);
            }
        }
        else
        {
            printf("Playback on SID %d is still functioning\n", sid);
        }

        printf("Sending resp.result[%d]\n", resp.result);
        if(avSendIOCtrl(av_index, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP, (char *)&resp, sizeof(SMsgAVIoctrlPlayRecordResp)) < 0)
            return -1;
    }
    else if(req->command == AVIOCTRL_RECORD_PLAY_PAUSE)
    {
        resp.command = AVIOCTRL_RECORD_PLAY_PAUSE;
        resp.result = 0;
        if(avSendIOCtrl(av_index, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP, (char *)&resp, sizeof(SMsgAVIoctrlPlayRecordResp)) < 0)
            return -1;

        //get writer lock
        lock_ret = pthread_rwlock_wrlock(&gClientInfo[sid].sLock);
        if(lock_ret)
            printf("Acquire SID %d rwlock error, ret = %d\n", sid, lock_ret);
        gClientInfo[sid].bPausePlayBack = !gClientInfo[sid].bPausePlayBack;
        //release lock
        lock_ret = pthread_rwlock_unlock(&gClientInfo[sid].sLock);
        if(lock_ret)
            printf("Release SID %d rwlock error, ret = %d\n", sid, lock_ret);
    }
    else if(req->command == AVIOCTRL_RECORD_PLAY_STOP)
    {
        //get writer lock
        lock_ret = pthread_rwlock_wrlock(&gClientInfo[sid].sLock);
        if(lock_ret)
            printf("Acquire SID %d rwlock error, ret = %d\n", sid, lock_ret);
        gClientInfo[sid].bStopPlayBack = 1;
        //release lock
        lock_ret = pthread_rwlock_unlock(&gClientInfo[sid].sLock);
        if(lock_ret)
            printf("Release SID %d rwlock error, ret = %d\n", sid, lock_ret);
    }

    return 0;
}


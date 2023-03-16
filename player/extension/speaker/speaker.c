#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "IOTCAPIs/IOTCClient.h"
#include "AVAPIs/AVClient.h"
#include "AVAPIs_Server.h"

extern AV_Client gClientInfo[MAX_CLIENT_NUMBER];
extern char avID[32];
extern char avPass[32];

void close_dsp(int fd)
{
	close(fd);
}

int open_dsp()
{
	unlink("audio.in");
	return open("audio.in", O_WRONLY | O_CREAT, 644);
}

void audio_playback(int fd, char *buf, int size)
{
	int ret = write(fd, buf, size);
	if (ret < 0)
	{
		printf("audio_playback::write , ret=[%d]\n", ret);
	}
}

void *thread_ReceiveAudio(void *arg)
{
	int SID = *((int *)arg);
	free(arg);
	int recordCnt = 0, recordFlag = 0;
	AV_Client *p = &gClientInfo[SID];
    int avIndex = 0;

    if(1 == p->bTwoWayStream) {
        avIndex = p->avIndex;
        printf("\n[thread_ReceiveAudio] Client support two way stream\n");
    }
    else {
        AVClientStartInConfig avClientInConfig;
        AVClientStartOutConfig avClientOutConfig;

        avClientInConfig.cb               = sizeof(AVClientStartInConfig);
        avClientInConfig.iotc_channel_id  = p->speakerCh;
        avClientInConfig.iotc_session_id  = SID;
        avClientInConfig.timeout_sec      = 30;
        avClientInConfig.account_or_identity = "";
        avClientInConfig.password_or_token = "";
        avClientInConfig.resend           = 0;
        avClientInConfig.security_mode    = AV_SECURITY_SIMPLE;
        avClientInConfig.auth_type        = AV_AUTH_PASSWORD;
        avClientOutConfig.cb = sizeof(AVClientStartOutConfig);
        //IOTC_Session_Channel_ON(SID, p->speakerCh);
        avIndex = avClientStartEx(&avClientInConfig, &avClientOutConfig);
        printf("\navClientStartEx idx[%d], resend[%d] two_way_streaming[%d]\n", avIndex, avClientOutConfig.resend, avClientOutConfig.two_way_streaming);
    }
	printf("\n\n[thread_ReceiveAudio] start ok idx[%d]\n\n", avIndex);

	if(avIndex > -1)
	{
		char buf[AUDIO_BUF_SIZE];
		FRAMEINFO_t frameInfo;
		unsigned int frmNo = 0;
		int dspFd = open_dsp();
        int fpsCnt = 0;

		if(dspFd < 0)
		{
			printf("open_dsp error[%d]!\n", dspFd);
			avClientStop(avIndex);
			pthread_exit(0);
		}
        if(1 != p->bTwoWayStream)
            avClientCleanAudioBuf(p->speakerCh);
        struct timeval tv1, tv2;
        gettimeofday(&tv1, NULL);
		while(p->bEnableSpeaker)
		{
			int ret = avRecvAudioData(avIndex, buf, AUDIO_BUF_SIZE, (char *)&frameInfo, sizeof(FRAMEINFO_t), &frmNo);
			//printf("avRecvAudioData[%d]\n", ret);
			if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
			{
				printf("avRecvAudioData AV_ER_SESSION_CLOSE_BY_REMOTE\n");
				break;
			}
			else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
			{
				printf("avRecvAudioData AV_ER_REMOTE_TIMEOUT_DISCONNECT\n");
				break;
			}
			else if(ret == IOTC_ER_INVALID_SID)
			{
				printf("avRecvAudioData Session[%d] cant be used anymore\n", SID);
				break;
			}
			else if(ret == AV_ER_LOSED_THIS_FRAME)
			{
				printf("Audio LOST[%d] ", frmNo);fflush(stdout);
				continue;
			}
			else if (ret == AV_ER_DATA_NOREADY)
            {
                usleep(5000);
                continue;
            }
			else if(ret < 0)
			{
				printf("Other error[%d]!!!\n", ret);
				break;
			}
			else
			{
                fpsCnt++;
				//printf("avRecvAudioData[%d]\n", ret);
			}

			if(recordFlag)
			{
				if(recordCnt++ > 2000)
				{
					recordFlag = 0;
					printf("Record finish\n");
					close(dspFd);
					break;
				}
				audio_playback(dspFd, buf, ret);
			}

            gettimeofday(&tv2, NULL);
            long sec = tv2.tv_sec-tv1.tv_sec, usec = tv2.tv_usec-tv1.tv_usec;
			if(usec < 0)
			{
				sec--;
				usec += 1000000;
			}
			usec += (sec*1000000);

            if(usec >= 1000000)
            {
				printf("[AVAPIs_Servr] Recv Spaeker Audio FPS = %d\n", fpsCnt);
				fpsCnt = 0;
                gettimeofday(&tv1, NULL);
            }
		}
	}

	printf("[thread_ReceiveAudio] exit\n");
	if(1 != p->bTwoWayStream) {
	    avClientStop(avIndex);
	}
	pthread_exit(0);
}

int Handle_IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_REQ(int av_index, char* req)
{
    SMsgAVIoctrlGetAudioOutFormatReq *p = (SMsgAVIoctrlGetAudioOutFormatReq *)req;
    SMsgAVIoctrlGetAudioOutFormatResp res;

    printf("[%s:%d] av_index[%d] Receive IOCTRL command: IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_REQ\n", __FUNCTION__, __LINE__, av_index);

    memset(&res, 0, sizeof(SMsgAVIoctrlGetAudioOutFormatResp));

    res.channel = p->channel;
    res.codecId = MEDIA_CODEC_AUDIO_G711U;
    res.sample_rate = AUDIO_SAMPLE_8K;
    res.bitdata = AUDIO_DATABITS_16;
    res.channels = AUDIO_CHANNEL_MONO;
    if(p->channel == 0)
        res.avservchannel = 1;    // set avservchannel = 1 for Kalay App using two way stream

    if(avSendIOCtrl(av_index, IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetAudioOutFormatResp)) < 0)
        return -1;

    return 0;
}

int Handle_IOTYPE_USER_IPCAM_SPEAKERSTART(int sid, int av_index, char* req)
{
    SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)req;
    int ret = 0, *tmp_sid = NULL;
    pthread_t Thread_ID;

    printf("[%s:%d] sid[%d] av_index[%d] Receive IOCTRL command: IOTYPE_USER_IPCAM_SPEAKERSTART channel[%d]\n", __FUNCTION__, __LINE__, sid, av_index, p->channel);

	//get writer lock
	int lock_ret = pthread_rwlock_wrlock(&gClientInfo[sid].sLock);
	if(lock_ret)
		printf("Acquire SID %d rwlock error, ret = %d\n", sid, lock_ret);
	gClientInfo[sid].speakerCh = p->channel;
	gClientInfo[sid].bEnableSpeaker = 1;
	//release lock
	lock_ret = pthread_rwlock_unlock(&gClientInfo[sid].sLock);
	if(lock_ret)
		printf("Release SID %d rwlock error, ret = %d\n", sid, lock_ret);
	// use which channel decided by client
	tmp_sid = (int *)malloc(sizeof(int));
	*tmp_sid = sid;

	if((ret = pthread_create(&Thread_ID, NULL, &thread_ReceiveAudio, (void *)tmp_sid))){
		printf("pthread_create thread_ReceiveAudio ret[%d]\n", ret);
		return -1;
	}
	pthread_detach(Thread_ID);

    return 0;
}

int Handle_IOTYPE_USER_IPCAM_SPEAKERSTOP(int sid, int av_index, char* req)
{
    printf("[%s:%d] sid[%d] av_index[%d] Receive IOCTRL command: IOTYPE_USER_IPCAM_SPEAKERSTOP\n", __FUNCTION__, __LINE__, sid, av_index);

	//get writer lock
	int lock_ret = pthread_rwlock_wrlock(&gClientInfo[sid].sLock);
	if(lock_ret)
		printf("Acquire SID %d rwlock error, ret = %d\n", sid, lock_ret);
	gClientInfo[sid].bEnableSpeaker = 0;
	//release lock
	lock_ret = pthread_rwlock_unlock(&gClientInfo[sid].sLock);
	if(lock_ret)
		printf("Release SID %d rwlock error, ret = %d\n", sid, lock_ret);

    return 0;
}


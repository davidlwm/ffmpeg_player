#ifndef __SPEAKER_H__
#define __SPEAKER_H__

int Handle_IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_REQ(int av_index, char* req);
int Handle_IOTYPE_USER_IPCAM_SPEAKERSTART(int sid, int av_index, char* req);
int Handle_IOTYPE_USER_IPCAM_SPEAKERSTOP(int sid, int av_index, char* req);

#endif


#ifndef __PLAYBACK_H__
#define __PLAYBACK_H__

int Handle_IOTYPE_USER_IPCAM_LISTEVENT_REQ(int av_index, char* req);
int Handle_IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL(int SID, int av_index, char* req);

#endif

#ifndef _AVAPI_EMULATOR_H_
#define _AVAPI_EMULATOR_H_

#define EMULATOR_STREAM_NUM 5

typedef enum
{
    EMULATOR_MODE_SINGLESTREAM  = 0,
    EMULATOR_MODE_MULTISTREAM   = 1
}EMULATOR_MODE;

typedef int (*Emulator_SendVideoFunc)(unsigned int timestamp, char* buf, int size, int frameType);

typedef int (*Emulator_SendAudioFunc)(unsigned int timestamp, char* buf, int size);

int Emulator_Initialize(int mode, int initStream, Emulator_SendVideoFunc sendVideoFunc, Emulator_SendAudioFunc sendAudioFunc);
int Emulator_DeInitialize();
int Emulator_ChangeStream(int selectStream);

int Streamout_SetCacheIFrame(char* buf, int size);
int Streamout_GetCacheIFrame(char* buf);
int CreateEmulatorStream(Emulator_SendVideoFunc sendVideoFunc, Emulator_SendAudioFunc sendAudioFunc);

#endif //_AVAPI_EMULATOR_H_

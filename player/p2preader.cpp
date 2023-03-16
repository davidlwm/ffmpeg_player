//
// Created by apple on 28/12/2022.
//

#include "p2preader.h"
#include <memory>
#include <thread>
#include "FrameInfo.h"
#include "log.h"
#include "util/ac_time.h"
#include "error.h"
extern "C"{
#include "IOTCAPIs.h"
#include "AVAPIs.h"

typedef struct
{
    char sourceId[20]; // Camera Index
} SMsgAVIoctrlUser;

}

int InitLicense(){
    const char *license_key = "AQAAAFN1pQ7o+sRwhzzbSPpFkOaDwBToCSQ14FSSKRidnONzdeKaBHSUhS/rHJECkbmhaXGnKN4lMqjQP9nz6SFOclVRQTLYbdAPbLdsJO7/FiqfOhgp3CQ1Oaqdz5alo0DUr4fAhPTdE+JA/M+5opVrVLGTB1RcWuIXEwJ2QHeO71Ycalu1/RKCCIu0GWWPbvgyTc5Uzx06eIKfntwbE55dunQb";
    int ret = TUTK_SDK_Set_License_Key(license_key);
    if (ret != TUTK_ER_NoERROR)
    {
        LOGE("TUTK_SDK_Set_License_Key()=%d ,exit...!!", ret);
    }
    ret = IOTC_Initialize2(0);
    if (ret != IOTC_ER_NoERROR && ret != IOTC_ER_ALREADY_INITIALIZED) {
        LOGE("IOTC_Initialize2 failed ret[%d]", ret);
        return ret;
    }
    avInitialize(MX_CHANNELS);
    LOGI("InitLicense OK");
    return ret;
}

P2PReader::P2PReader(std::string source):reading(true),avIndex_(-1),sourceId_(source),sid_(-1) {
    video_decoder = new FFDecoder();
    audio_decoder = new FFDecoder();
}

P2PReader::~P2PReader() {
//    avDeInitialize();
    if (avIndex_ >= 0){
        avClientStop(avIndex_);
    }
    if(video_decoder != nullptr){
        delete video_decoder;
    }
    if(audio_decoder != nullptr){
        delete audio_decoder;
    }
    LOGI("~P2PReader()");
}

void P2PReader::Init(const char *uid) {
    memset(&this->config,0, sizeof(DemoConfig));
    DemoConfig  *demo_config = &this->config;
    strcpy(demo_config->iotc_auth_key, "00000000");
    strcpy(demo_config->av_account, "admin");
    strcpy(demo_config->av_password, "888888");
    strcpy(demo_config->av_identity, "admin");
    strcpy(demo_config->av_token, "888888");

    LOGI("IOTCAPI version[%s] AVAPI version[%s]", IOTC_Get_Version_String(), avGetAVApiVersionString());

    strncpy(this->UID, uid, 21);
}



void P2PReader::ReadDataFunc(){
    int video_buf_size = VIDEO_BUF_SIZE;
    char *buf = (char*)malloc(video_buf_size);
    int ret;

    LOGI("ReadDataFunc start");

//    FRAMEINFO_t frameInfo;
    unsigned int frmNo;

    int vFPS = 0, aFPS = 0, vLostCnt = 0, aLostCnt = 0;
    int outBufSize = 0;
    int outFrmSize = 0;
    int outFrmInfoSize = 0;
    int vbps = 0, abps = 0;
    int video_not_ready = 1, audio_not_ready = 1;
    int firstVideoFrame_ready = 0;

    VideoFrameInfo_t videoFrameInfo;
    AudioFrameInfo_t audioFrameInfo;

    int64_t  last_ts = av_gettime_ms();
    int vMaxFrameLen = 0;
    int aMaxFrameLen = 0;

    while(reading) {
        ret = avRecvFrameData2(avIndex_, buf, video_buf_size, &outBufSize, &outFrmSize, (char *) &videoFrameInfo,
                               sizeof(VideoFrameInfo_t), &outFrmInfoSize, &frmNo);

        if(ret == AV_ER_DATA_NOREADY) {
//            video_not_ready = 1;
        } else if(ret == AV_ER_LOSED_THIS_FRAME) {
            LOGI("Recv video, Lost video frame NO[%d]", frmNo);
            vLostCnt++;
        } else if(ret == AV_ER_INCOMPLETE_FRAME) {
#if 1
            if(outFrmInfoSize > 0){
                LOGI("Incomplete video frame NO[%d] ReadSize[%d] FrmSize[%d] FrmInfoSize[%u]  Flag[%d]",
                     frmNo, outBufSize, outFrmSize, outFrmInfoSize, videoFrameInfo.flags);
            }
            else
                LOGI("Incomplete video frame NO[%d] ReadSize[%d] FrmSize[%d] FrmInfoSize[%u]", frmNo, outBufSize, outFrmSize, outFrmInfoSize);
#endif
            vLostCnt++;
        } else if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE) {
            LOGE("Recv video, AV_ER_SESSION_CLOSE_BY_REMOTE");
            break;
        } else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT) {
            LOGE("Recv video, AV_ER_REMOTE_TIMEOUT_DISCONNECT");
            break;
        } else if(ret == IOTC_ER_INVALID_SID) {
            LOGE("Recv video, Session cant be used anymore");
            break;
        } else if(ret == AV_ER_BUFPARA_MAXSIZE_INSUFF) {
            LOGE("Recv video, video buffer too small");
            video_buf_size = video_buf_size+(5*1024);
            buf = (char*)realloc(buf, video_buf_size);
            LOGI("increase buffer size to %d\n", video_buf_size);
            continue;
        } else if(ret < 0){
            LOGE("Recv video, error[%d]!!!", ret);
            continue;
        } else {
            if(vMaxFrameLen < outBufSize){
                vMaxFrameLen = outBufSize;
            }
            vbps += outBufSize;
            vFPS++;
            video_not_ready = 0;
            if (!this->video_decoder->IsInit()){
                this->video_decoder->InitDecoder(AVCodecID(videoFrameInfo.codec_id),
                                                 videoFrameInfo.extradata,
                                                 videoFrameInfo.extradata_size);
            }
            int64_t  now1 = av_gettime_ms();
            this->video_decoder->DecoderVideo((uint8_t*) buf,outBufSize, videoFrameInfo.timestamp);
            if (av_gettime_ms() -now1 > 300){
                printf("DecoderVideo use %lld\n",av_gettime_ms() -now1);
            }
            firstVideoFrame_ready = 1;

            int64_t  now = av_gettime_ms();
            int64_t diff = now - last_ts;
            if (diff > 1000){

                printf("framerate(%f %f) bitrate(%3.1f %3.1f) lost(%d %d) max(%d %d)\n",
                     (double)vFPS*1000 / diff,(double)aFPS*1000 / diff,
                     vbps*1.0 / (now - last_ts),
                     abps*1.0 / (now - last_ts),
                     vLostCnt,aLostCnt,
                     vMaxFrameLen,aMaxFrameLen);
                last_ts = now;
                vbps = 0;
                vFPS = 0;
                aFPS = 0;
                abps = 0;
            }
            fflush(stdout);
        }

        ret = avRecvAudioData(avIndex_, buf, AUDIO_BUF_SIZE, (char *)&audioFrameInfo, sizeof(AudioFrameInfo_t), &frmNo);
        if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE) {
            LOGE("Recv audio, AV_ER_SESSION_CLOSE_BY_REMOTE");
            break;
        } else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT) {
            LOGE("Recv audio, AV_ER_REMOTE_TIMEOUT_DISCONNECT");
            break;
        } else if(ret == IOTC_ER_INVALID_SID) {
            LOGE("Recv audio, Session cant be used anymore");
            break;
        } else if(ret == AV_ER_LOSED_THIS_FRAME) {
            LOGW("Recv audio, AV_ER_LOSED_THIS_FRAME[%d]", frmNo);
            aLostCnt++;
        } else if (ret == AV_ER_DATA_NOREADY) {
            audio_not_ready = 1;
        } else if(ret < 0) {
            LOGE("Recv audio, error[%d]!!!", ret);
            continue;
        } else {
            abps += ret;
            aFPS++;
            audio_not_ready = 0;
            if(aMaxFrameLen < ret){
                aMaxFrameLen = ret;
            }
            if(firstVideoFrame_ready == 0){
//                LOGI("DROP audio \n");
                continue;
            }
            if (! audio_decoder->IsInit()){
                audio_decoder->InitAudioDecoder(AVCodecID(audioFrameInfo.codec_id),
                                          audioFrameInfo.extradata,
                                          audioFrameInfo.extradata_size,
                                          &audioFrameInfo);
            }
            this->audio_decoder->DecoderAudio((uint8_t*) buf,ret, audioFrameInfo.timestamp);
        }
        if(1 == video_not_ready && 1 == audio_not_ready){
            av_msleep(10);
        }

    }
    if(buf) {
        free(buf);
    }
    avClientStop(avIndex_);
    LOGI("avClientStop avIndex_[%d]",  avIndex_);
    avIndex_ = -1;
    LOGW("thread exit");
}
int P2PReader::StartIPcam() const{
    int ret;
    if (sourceId_.empty()){
        LOGE("sourceId.empty()");
        return -1;
    }
//    std::string sourceId = "ccc";
    SMsgAVIoctrlUser ioMsg;
    memset(&ioMsg, 0, sizeof(SMsgAVIoctrlUser));
    memcpy(ioMsg.sourceId, sourceId_.c_str(), sourceId_.length());
    if((ret = avSendIOCtrl(avIndex_, IOTYPE_USER_IPCAM_START, (char *)&ioMsg, sizeof(SMsgAVIoctrlUser))) < 0)
    {
        LOGI("StartIPcam IOTYPE_USER_IPCAM_START failed[%d]", ret);
        return ret;
    }
    LOGI("StartIPcam send IOTYPE_USER_IPCAM_START OK");

    if((ret = avSendIOCtrl(avIndex_, IOTYPE_USER_IPCAM_AUDIOSTART, (char *)&ioMsg, sizeof(SMsgAVIoctrlUser))) < 0)
	{
		LOGI("StartIPcam IOTYPE_USER_IPCAM_AUDIOSTART failed[%d]", ret);
		return ret;
	}
	LOGI("StartIPcam send  IOTYPE_USER_IPCAM_AUDIOSTART, OK");
    return 0;
}

void P2PReader::ConnectFunc(){
    int   tmpSID;
    tmpSID = IOTC_Get_SessionID();
    if(tmpSID < 0) {
        LOGE("IOTC_Get_SessionID failed ret[%d]\n",  tmpSID);
        return ;
    }

    IOTCConnectInput connect_input;
    connect_input.cb = sizeof(connect_input);
    connect_input.authentication_type = AUTHENTICATE_BY_KEY;
    connect_input.timeout = 20; //sec
    memcpy(connect_input.auth_key, this->config.iotc_auth_key, IOTC_AUTH_KEY_LENGTH);

    sid_ = IOTC_Connect_ByUIDEx(this->UID, tmpSID, &connect_input);
    if (sid_ < 0) {
        LOGE("Connect failed SID[%d] %s", sid_, GetConnectError(sid_));
        return ;
    }

    {//check Sinfo
        struct st_SInfoEx Sinfo;
        memset(&Sinfo, 0, sizeof(struct st_SInfoEx));
        Sinfo.size = sizeof(struct st_SInfoEx);
        if(IOTC_Session_Check_Ex(sid_, &Sinfo) == IOTC_ER_NoERROR) {
            if(Sinfo.Mode == 0)
                printf("Device is from %s:%d[%s] Mode=P2P\n",Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);
            else if(Sinfo.Mode == 1)
                printf("Device is from %s:%d[%s] Mode=RLY\n",Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);
            else if(Sinfo.Mode == 2)
                printf("Device is from %s:%d[%s] Mode=LAN\n",Sinfo.RemoteIP, Sinfo.RemotePort, Sinfo.UID);
            printf("Device info VER[%X] NAT[%d]\n", Sinfo.IOTCVersion, Sinfo.RemoteNatType);
        }
    }
    //----------

    AVClientStartInConfig avClientInConfig;
    AVClientStartOutConfig avClientOutConfig;
    memset(&avClientInConfig, 0, sizeof(avClientInConfig));
    avClientInConfig.cb               = sizeof(AVClientStartInConfig);
    avClientInConfig.iotc_channel_id  = 0;
    avClientInConfig.iotc_session_id  = sid_;
    avClientInConfig.timeout_sec      = 30;
    avClientInConfig.resend           = 1;
    avClientInConfig.security_mode = AV_SECURITY_SIMPLE;
    avClientInConfig.account_or_identity    = this->config.av_account;
    avClientInConfig.password_or_token      = this->config.av_password;
    avClientInConfig.auth_type              = AV_AUTH_PASSWORD;
    avClientOutConfig.cb = sizeof(AVClientStartOutConfig);
    avIndex_ = avClientStartEx(&avClientInConfig, &avClientOutConfig);
    if (avIndex_ < 0) {
        LOGE("avClientStartEx failed avIndex_[%d]\n", avIndex_);
        return;
    }
    LOGI("avClientStartEx success avIndex_[%d] resend[%d] two_way_streaming[%d] security_mode[%s]",
           avIndex_, avClientOutConfig.resend, avClientOutConfig.two_way_streaming, (avClientOutConfig.security_mode==AV_SECURITY_SIMPLE)?"AV_SECURITY_SIMPLE":"AV_SECURITY_DTLS");

    //----------

    read_thread = std::thread(&P2PReader::ReadDataFunc, this);
    if (StartIPcam() < 0){
    }
    if (read_thread.joinable()){
        read_thread.join();
        LOGI("read_thread.join() end");
    }
    if(sid_ >= 0){
        IOTC_Session_Close(sid_);
        sid_ = -1;
    }
    LOGI("IOTC_Session_Close SID[%d]",sid_);
}

void P2PReader::Start()  {
    demux_thread = std::thread(&P2PReader::ConnectFunc, this);
}

void P2PReader::Stop(){
    this->reading = false;
    avClientExit(sid_,0);
    this->video_decoder->Stop();
    this->audio_decoder->Stop();
    if (demux_thread.joinable()){
        LOGI("P2PReader::join()");
        demux_thread.join();
    }
    LOGI("P2PReader::Stop()");
}

void P2PReader::SetCallback(void *obj, OnFrame cb1, OnAudioInfo cb2) {
    video_decoder->SetCallback(obj,cb1,cb2);
    audio_decoder->SetCallback(obj,cb1,cb2);
}

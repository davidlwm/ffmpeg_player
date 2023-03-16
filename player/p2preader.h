//
// Created by apple on 28/12/2022.
//

#ifndef P2PPLAYER_DEMUXER_H
#define P2PPLAYER_DEMUXER_H
#include <thread>
extern "C"{
#include "demoOption/demoOption.h"
#include "P2PCam/AVFRAMEINFO.h"
#include "P2PCam/AVIOCTRLDEFs.h"
};
#include "decoder.h"
#include "render.h"

#define VIDEO_BUF_SIZE	500000
#define AUDIO_BUF_SIZE	1024*2
#define MX_CHANNELS 30

int InitLicense();

class P2PReader : public SourceCtl{
    DemoConfig  config;
    char UID[21];
    int avIndex_;
    int sid_;
    FFDecoder *video_decoder;
    FFDecoder *audio_decoder;
    std::atomic_bool reading;
    std::string sourceId_;

public:
    explicit P2PReader(std::string sourceId);
     ~P2PReader();

    void Init(const char *uid);
    void SetCallback(void * obj ,OnFrame cb1,OnAudioInfo cb2);
    void Start();
    void Stop() override;

private:
    void ConnectFunc();
    void ReadDataFunc();
    int StartIPcam() const;
//    int StopIPcam(int avIndex );

    std::thread demux_thread;
    std::thread read_thread;
};


#endif //P2PPLAYER_DEMUXER_H

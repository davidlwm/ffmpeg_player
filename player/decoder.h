//
// Created by apple on 29/12/2022.
//

#ifndef P2PPLAYER_DECODER_H
#define P2PPLAYER_DECODER_H
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
}
#include "util/ac_queue.h"
#include "ac_buf.h"
#include "FrameInfo.h"
#include "render.h"
#include "resample_audio.h"

class FFDecoder : public AudioPcmSource{
protected:
    bool init_ = false;
    AVCodec* codec_ = nullptr;
    AVCodecContext* codec_ctx_ = nullptr;
    AVPacket avpkt_;
    AVFrame* frame_ = 0;
    bool Stop_ = false;
//    RenderCtl* render_;
    void *cb_obj_;
    OnFrame frame_cb_;
    OnAudioInfo audio_cb_;

    ACQueue<std::shared_ptr<ACAudioFrameS16>> audio_pcm_buf_;
    bool init_audio_render_ = false;
    ResampleAudio resample_;

    int64_t last_;
    int n_;

public:
    FFDecoder();
    virtual ~FFDecoder();
    bool IsInit();

public:
    int InitDecoder(AVCodecID codec_id,
                    uint8_t* extra_data,
                    int extra_size);

    int InitAudioDecoder(AVCodecID codec_id,
                    uint8_t* extra_data,
                    int extra_size,
                    AudioFrameInfo_t *info);

    int DecoderVideo(uint8_t* pFrame,
                     int uiLen,
                     int64_t pts);
    int DecoderAudio(uint8_t* pFrame,
                     int uiLen,
                     int64_t pts);

    int FillAudio(uint8_t* buf, int len);
    void Stop();

    void SetCallback(void * obj ,OnFrame cb1,OnAudioInfo cb2);

private:
    int FeedPcm(uint8_t* pFrame,
                int uiLen,
                int64_t pts,
                AudioFrameInfo_t *info);
};


#endif //P2PPLAYER_DECODER_H

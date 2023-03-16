//
// Created by apple on 29/12/2022.
//

#include "decoder.h"
#include "render.h"
#include "util/ac_time.h"
#include "log.h"
extern "C"{
#include "libavcodec/avcodec.h"
}
FFDecoder::FFDecoder():init_(false),frame_cb_(nullptr),audio_cb_(nullptr),cb_obj_(nullptr) {
    last_ = av_gettime_ms();
    n_ = 0;
}

FFDecoder::~FFDecoder() {
    if (codec_ctx_) {
        int ret = avcodec_close(codec_ctx_);
        LOGI("~FFDecoder() avcodec_close ret: %d.", ret);
        codec_ctx_ = nullptr;
    }
    av_frame_free(&frame_);
}

bool FFDecoder::IsInit() {
    return init_;
}
int FFDecoder::InitDecoder(AVCodecID codec_id,
                           uint8_t* extra_data,
                           int extra_size) {
    if (init_ != false) {
        LOGE("decoder inited fail");
        return -1;
    }
    //        avcodec_register_all();
    codec_ = avcodec_find_decoder(codec_id);
    if (!codec_) {
        LOGE("avcodec_find_decoder fail");
        return -1;
    }
    codec_ctx_ = avcodec_alloc_context3(codec_);
    if (!codec_ctx_) {
        LOGE("avcodec_alloc_context3 fail");
        return -1;
    }

    codec_ctx_->framerate = AVRational{0, 1};
    if (extra_size > 0){
        codec_ctx_->extradata = (uint8_t*)malloc(extra_size + 32);
        memcpy(codec_ctx_->extradata, extra_data, extra_size);
        codec_ctx_->extradata_size = extra_size;
    }else{
        codec_ctx_->extradata_size = 0;
    }

    int ret = avcodec_open2(codec_ctx_, codec_, NULL);
    if ( ret < 0) {
        printf("avcodec_open2 fail\n");
        return -1;
    }
    init_ = true;

    if (!(frame_ = av_frame_alloc())) {
        printf("Can not alloc frame");
        return -1;
    }

    return 0;
}

int FFDecoder::InitAudioDecoder(AVCodecID codec_id,
                           uint8_t* extra_data,
                           int extra_size,
                           AudioFrameInfo_t *info) {
    if (init_ != false) {
        LOGE("decoder inited fail");
        return -1;
    }
    //        avcodec_register_all();
    codec_ = avcodec_find_decoder(codec_id);
    if (!codec_) {
        LOGE("avcodec_find_decoder fail");
        return -1;
    }
    codec_ctx_ = avcodec_alloc_context3(codec_);
    if (!codec_ctx_) {
        LOGE("avcodec_alloc_context3 fail");
        return -1;
    }
    codec_ctx_->sample_fmt = (AVSampleFormat)info->sample_fmt;
    codec_ctx_->sample_rate = info->sample_rate;
    codec_ctx_->channels = info->channel;

    codec_ctx_->framerate = AVRational{0, 1};
    if (extra_size > 0){
        codec_ctx_->extradata = (uint8_t*)malloc(extra_size + 32);
        memcpy(codec_ctx_->extradata, extra_data, extra_size);
        codec_ctx_->extradata_size = extra_size;
    }else{
        codec_ctx_->extradata_size = 0;
    }

    int ret = avcodec_open2(codec_ctx_, codec_, NULL);
    if ( ret < 0) {
        printf("avcodec_open2 fai");
        return -1;
    }
    init_ = true;

    if (!(frame_ = av_frame_alloc())) {
        printf("Can not alloc frame");
        return -1;
    }

    return 0;
}


int FFDecoder::DecoderVideo(uint8_t* pFrame,
                            int uiLen,
                            int64_t pts) {
    av_init_packet(&avpkt_);
    avpkt_.data = (uint8_t*)pFrame;
    avpkt_.size = uiLen;
    avpkt_.pts = pts;

    int got_frame = 0;
    int ret = avcodec_decode_video2(codec_ctx_, frame_, &got_frame, &avpkt_);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
        LOGE("avcodec_decode_video2 failed(%d: %s)", ret, errbuf);

        av_packet_unref(&avpkt_);
        return ret;
    }

#ifndef FF_API_PKT_PTS
    if (yuv_buf->pts == AV_NOPTS_VALUE) {
    yuv_buf->pts = yuv_buf->pkt_pts;
  }
#endif  // ! FF_API_PKT_PTS

    if (got_frame) {
          ++ n_;
//        av_frame_copy(&frame_copy, this->frame_);
        AVFrame *frame_copy = av_frame_clone(this->frame_);
        av_frame_unref(this->frame_);
        if(frame_cb_){
            frame_cb_(cb_obj_,frame_copy);
//            av_frame_unref(frame_copy);
        }
    }
    int64_t  now = av_gettime_ms();
    int64_t diff =  now - last_;
    if (diff > 1000){

         printf("n is %d %f\n",n_, (double)n_*1000 / diff);
         last_ = now;
         n_ = 0;
    }

    av_packet_unref(&avpkt_);
    return 1;
}
int FFDecoder::FeedPcm(uint8_t* pFrame,
            int uiLen,
            int64_t pts,
            AudioFrameInfo_t *info)
{
    auto buf = std::make_shared<ACAudioFrameS16>(pFrame, uiLen,
                                                 pts);
    audio_pcm_buf_.Push(buf);
    if (audio_pcm_buf_.Size() > 40){
        audio_pcm_buf_.Pop();
        audio_pcm_buf_.Pop();
        audio_pcm_buf_.Pop();
        //LOGI("drop audio pcm");
    }
    if (!init_audio_render_){
        if(audio_cb_){
            audio_cb_(cb_obj_,info, this);
            init_audio_render_ = true;
        }
    }
    return 0;
}

int FFDecoder::FillAudio(uint8_t* buf, int len){
    int need = len;
    uint8_t* p = buf;
    while (need > 0 && !Stop_) {
        auto pcm = this->audio_pcm_buf_.Front();
        if (pcm) {
            int n = pcm->GetData(p, need);
            need -= n;
            p += n;
            if (pcm->IsEmpty()) {
                audio_pcm_buf_.Pop();
            }
        } else {
            av_msleep(20);
        }
    }
    return len - need;
}

void FFDecoder::Stop(){
    Stop_ = true;
}

int FFDecoder::DecoderAudio(
        uint8_t* pFrame,
        int uiLen,
        int64_t pts) {
    av_init_packet(&avpkt_);
    avpkt_.data = (uint8_t*)pFrame;
    avpkt_.size = uiLen;
    avpkt_.pts = pts;

    int got_frame = 0;
    int ret = avcodec_decode_audio4(codec_ctx_, frame_, &got_frame, &avpkt_);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
        printf("avcodec_decode_audio4 failed(%d: %s)\n", ret, errbuf);

        av_packet_unref(&avpkt_);
        return ret;
    }

#ifndef FF_API_PKT_PTS
    if (yuv_buf->pts == AV_NOPTS_VALUE) {
    yuv_buf->pts = yuv_buf->pkt_pts;
  }
#endif  // ! FF_API_PKT_PTS

    if (got_frame) {

        if (!resample_.IsInit()){
            uint64_t  layout = frame_->channel_layout;
            if (layout == 0){
                layout = av_get_default_channel_layout(frame_->channels);
            }
            ret = resample_.Init(layout,frame_->sample_rate,codec_ctx_->sample_fmt);
            if (ret != 0){
                printf("resample_.Init failed \n");
            }
        }
        AVFrame* frameOut = av_frame_alloc();
        resample_.Resample(frame_,frameOut);

        AudioFrameInfo_t  frameInfo;
        frameInfo.codec_id = frameOut->format;
        frameInfo.timestamp = frameOut->pts;
        frameInfo.sample_rate = frameOut->sample_rate;
        frameInfo.channel = frameOut->channels;
        frameInfo.sample_fmt = AV_SAMPLE_FMT_S16;

        FeedPcm((uint8_t*)frameOut->data[0], frameOut->linesize[0], frame_->pts, &frameInfo);

        av_frame_free(&frameOut);
    }

    av_packet_unref(&avpkt_);
    return 1;
}

void FFDecoder::SetCallback(void *obj, OnFrame cb1, OnAudioInfo cb2) {
    cb_obj_ = obj;
    frame_cb_ = cb1;
    audio_cb_ = cb2;
}

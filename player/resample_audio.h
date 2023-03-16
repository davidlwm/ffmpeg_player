//
// Created by ljg on 2019/9/10.
//

#ifndef LIBMCU_RESAMPLEAUDIO_H
#define LIBMCU_RESAMPLEAUDIO_H
#include <list>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
};

class ResampleAudio {
  struct SwrContext* swr_ctx_;
  int64_t dst_ch_layout_;
  enum AVSampleFormat dst_sample_fmt_;
  int dst_rate_;

 public:
  ResampleAudio();
  ~ResampleAudio();
  int Init(int64_t src_ch_layout,
           int src_rate,
           enum AVSampleFormat src_sample_fmt);
  int Resample(AVFrame* input, AVFrame* out);
  // int FillBuffer(AVFrame* out);
  bool IsInit() { return swr_ctx_ != nullptr; };
};

#endif  // LIBMCU_RESAMPLEAUDIO_H

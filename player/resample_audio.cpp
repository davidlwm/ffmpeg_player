//
// Created by ljg on 2019/9/10.
//

#include "resample_audio.h"
// #include "Log4Module.h"

ResampleAudio::ResampleAudio()
    : swr_ctx_(nullptr),
      dst_ch_layout_(AV_CH_LAYOUT_MONO),
      dst_sample_fmt_(AV_SAMPLE_FMT_S16),
      dst_rate_(44100) {}

int ResampleAudio::Init(int64_t src_ch_layout,
                        int src_rate,
                        enum AVSampleFormat src_sample_fmt) {
  int ret;
  // int src_channel =
  //     av_get_channel_layout_nb_channels(src_ch_layout);  //有几个channel 1
  /* create resampler context */
  swr_ctx_ = swr_alloc();
  if (!swr_ctx_) {
    ret = AVERROR(ENOMEM);
    return ret;
  }

  //只改变dst_sample_fmt_,其他保持不变
  dst_ch_layout_ = src_ch_layout;
  dst_rate_ = src_rate;
  dst_sample_fmt_ = AV_SAMPLE_FMT_S16;

  /* set options */
  av_opt_set_int(swr_ctx_, "in_channel_layout", src_ch_layout, 0);
  av_opt_set_int(swr_ctx_, "in_sample_rate", src_rate, 0);
  av_opt_set_sample_fmt(swr_ctx_, "in_sample_fmt", src_sample_fmt, 0);

  av_opt_set_int(swr_ctx_, "out_channel_layout", dst_ch_layout_, 0);
  av_opt_set_int(swr_ctx_, "out_sample_rate", dst_rate_, 0);
  av_opt_set_sample_fmt(swr_ctx_, "out_sample_fmt", dst_sample_fmt_, 0);

  /* initialize the resampling context */
  if ((ret = swr_init(swr_ctx_)) < 0) {
    fprintf(stderr, "Failed to initialize the resampling context\n");
    return -1;
  }

  return 0;
}

ResampleAudio::~ResampleAudio() {
  //        std::lock_guard<std::mutex> lck(m);
  swr_free(&swr_ctx_);
}

int ResampleAudio::Resample(AVFrame* input, AVFrame* out) {
  int dst_nb_channels =
      av_get_channel_layout_nb_channels(dst_ch_layout_);  //有几个channel 1
  //    avcodec_fill_audio_frame(out, dst_nb_channels ,dst_sample_fmt_,)
  uint8_t** dst_data;
  int dst_linesize;
  int ret;

  int dst_nb_samples =
      av_rescale_rnd(input->nb_samples, dst_rate_, input->sample_rate,
                     AV_ROUND_UP);  // 1024 sample

  //                printf("audio nb sample %d \n",input->nb_samples);
  //计算大小
  int needed_size = av_samples_get_buffer_size(
      NULL, dst_nb_channels, dst_nb_samples, dst_sample_fmt_, 1);

  // printf("need size %d ,dst_nb_channels %d  dst_sample_fmt_ %d\n",
  // needed_size,
  //        dst_nb_channels, dst_sample_fmt_);

  //分配内存
  ret = av_samples_alloc_array_and_samples(&dst_data, &dst_linesize,
                                           dst_nb_channels, dst_nb_samples,
                                           dst_sample_fmt_, 1);  // dst_linesize

  if (ret < 0) {
    fprintf(stderr, "Could not allocate destination samples\n");
    return -1;
  }
  //    av_samples_get_buffer_size
  ret = swr_convert(swr_ctx_, dst_data, dst_nb_samples,
                    (const uint8_t**)input->data, input->nb_samples);

  // out 并不知道这些信息，无法填充数据
  out->nb_samples = dst_nb_samples;
  out->sample_rate = dst_rate_;
  out->format = dst_sample_fmt_;
  out->channels = dst_nb_channels;
  out->channel_layout = dst_ch_layout_;
  out->pts = input->pts;

  ret = avcodec_fill_audio_frame(out, dst_nb_channels, dst_sample_fmt_,
                                 (const uint8_t*)dst_data[0], needed_size, 1);

  if (ret < 0) {
    //printf("avcodec_fill_audio_frame failed \n");
  }

  av_free(dst_data);
  return 0;
}

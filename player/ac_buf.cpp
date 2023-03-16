
#include "ac_buf.h"

#include <algorithm>
#include <iostream>
#include <sstream>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/time.h>
}

using namespace std;

ACPacket::ACPacket(ACMediaType type,
                   uint8_t* data,
                   int len,
                   bool key,
                   int64_t time_stamp)
    : type_(type),
      data_(nullptr),
      data_size_(len),
      key_frame_(key),
      time_stamp_(time_stamp) {
  if (data && len > 0) {
    data_ = (uint8_t*)av_malloc(len);
    if (data_) {
      memcpy(data_, data, len);
    }
    recieve_time_ = av_gettime();
  }
}

ACPacket::~ACPacket() {
  // cout  << "~ACPacket()\n";
  if (data_) {
    av_free(data_);
    data_ = nullptr;
  }
}

void ACPacket::PrintDelay() {
  string tag = type_ == ACVideo ? "v" : "a";
  cout << tag << " delay " << (av_gettime() - recieve_time_) / 1000 << " \n";
}

ACAudioFrameS16::ACAudioFrameS16(uint8_t* data, int linesize, int64_t ts)
    : buf_(nullptr),
      cur_(nullptr),
      linesize_(linesize),
      time_stamp_(ts),
      decodc_time_(0),
      recieve_time_(0) {
    if (data && linesize > 0) {
        buf_ = (uint8_t*)av_malloc(linesize);
        cur_ = buf_;
        if (buf_) {
            memcpy(buf_, data, linesize);
        }
        recieve_time_ = av_gettime();
    }
}
ACAudioFrameS16::~ACAudioFrameS16() {
  if (buf_) {
      av_free(buf_);
      buf_ = nullptr;
  }
}

int ACAudioFrameS16::GetData(uint8_t* data, int size) {
  int data_size = buf_ + linesize_ - cur_;
  int n = std::min(size, data_size);
  memcpy(data, cur_, n);
  cur_ += n;
  if (cur_ < buf_ + linesize_) {
    // update timestamp
  }
  return n;
}
bool ACAudioFrameS16::IsEmpty() {
  return cur_ >= buf_ + linesize_;
}
std::string ACAudioFrameS16::DelayToString() {
  stringstream ss;
  auto now = av_gettime();
  ss << "[" << (now - decodc_time_) / 1000 << ","
     << (decodc_time_ - recieve_time_) / 1000 << ","
     << (now - recieve_time_) / 1000 << "] ";
  return ss.str();
}

ACVideoFrameYV12::ACVideoFrameYV12(uint8_t** data,
                                   int* linesize,
                                   int64_t ts,
                                   int w,
                                   int h)
    : time_stamp_(ts), recieve_time_(0), decodc_time_(0), width_(w), heigh_(h) {
  for (int i = 0; i < NUM_DATA_POINTERS; ++i) {
    data_[i] = data[i];
    linesize_[i] = linesize[i];
  }
  decodc_time_ = av_gettime();
}

ACVideoFrameYV12::~ACVideoFrameYV12() {
  for (int i = 0; i < NUM_DATA_POINTERS; ++i) {
    if (data_[i]) {
      // printf("~ACVideoFrameYV12 %d\n", i);
      av_free(data_[i]);
      data_[i] = nullptr;
    }
  }
}

std::string ACVideoFrameYV12::DelayToString() {
  stringstream ss;
  auto now = av_gettime();
  ss << "[" << (now - decodc_time_) / 1000 << ","
     << (decodc_time_ - recieve_time_) / 1000 << ","
     << (now - recieve_time_) / 1000 << "] ";
  return ss.str();
}
int ACVideoFrameYV12::GetDelay() {
  return (av_gettime() - recieve_time_) / 1000;
}
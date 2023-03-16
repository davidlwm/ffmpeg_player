
#ifndef __AC_BUFFER__
#define __AC_BUFFER__

#include <list>
#include <memory>
#include <mutex>
// TODO: 这个改改下名
using std::shared_ptr;

enum ACMediaType { ACVideo = 1, ACAudio = 2, ACOther = 3 };

class ACPacket {
 public:
  ACPacket(ACMediaType type,
           uint8_t* data,
           int len,
           bool key,
           int64_t time_stamp);
  ~ACPacket();
  ACMediaType type_;
  uint8_t* data_;
  int data_size_;
  bool key_frame_;
  int64_t time_stamp_;
  int64_t recieve_time_;

  void PrintDelay();
};

class ACAudioFrameS16 {
 public:
  ACAudioFrameS16(uint8_t* data, int linesize, int64_t ts);
  ~ACAudioFrameS16();
  int GetData(uint8_t* data, int size);
  std::string DelayToString();
  bool IsEmpty();
  uint8_t* buf_;
  uint8_t* cur_;
  int linesize_;
  int64_t time_stamp_;
  int64_t decodc_time_;
  int64_t recieve_time_;
};

/**
 * ACVideoFrameYV12 a struct for yuv frame , simalar with AVFrame,
 * NUM_DATA_POINTERS number of plane for yuv data
 */
#define NUM_DATA_POINTERS 3

class ACVideoFrameYV12 {
 public:
  ACVideoFrameYV12(uint8_t** data, int* linesize, int64_t ts, int w, int h);
  ~ACVideoFrameYV12();
  // void PrintDelay();
  std::string DelayToString();
  int GetDelay();
  uint8_t* data_[NUM_DATA_POINTERS];
  int linesize_[NUM_DATA_POINTERS];
  int64_t time_stamp_;
  int64_t recieve_time_;
  int64_t decodc_time_;
  int width_;
  int heigh_;
};

#endif
#ifndef FDECODE_H
#define FDECODE_H

#include <QThread>
#include <QString>
#include <QImage>

#include <QCamera>
#include <QCameraInfo>
//当前C++兼容C语言
extern "C"
{
//avcodec:编解码(最重要的库)
#include <libavcodec/avcodec.h>
//avformat:封装格式处理
#include <libavformat/avformat.h>
//swscale:视频像素数据格式转换
#include <libswscale/swscale.h>
//avdevice:各种设备的输入输出
#include <libavdevice/avdevice.h>
//avutil:工具库（大部分库都需要这个库的支持）
#include <libavutil/avutil.h>
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
}
class fdecode:public QThread
{
    Q_OBJECT
public:
    fdecode();
    fdecode(QString fileName);
    void openVideoStream(QString fileName);
    void registerFFmpeg();

    void run();


private:
    QString fileName;
signals:
    void sendImage(QImage img);
};

#endif // FDECODE_H

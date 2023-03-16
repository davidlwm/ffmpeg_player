#ifndef CAMERA_H
#define CAMERA_H

#include <QWidget>
#include "player/p2preader.h"
#include "yuv_render.h"
#include <widget.h>

class camera: public YUV420P_Render
{
    Q_OBJECT
public:
    explicit camera();
    void Init(QString sourceId,QString p2pUid);
};

#endif // CAMERA_H

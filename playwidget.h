#ifndef PLAYWIDGET_H
#define PLAYWIDGET_H

#include <QWidget>
#include <QImage>
#include <fdecode.h>
#include <QPaintEvent>
#include <QPainter>
#include <QImage>

class playWidget:public QWidget
{
    Q_OBJECT
public:
    playWidget();
    playWidget(QString fileName);

    void paintEvent(QPaintEvent *);

private:
    fdecode *fdec;
    QImage img;//不是 QImage *img;
    QString m_fileName;


public slots:

    void receiveImage(QImage img);
};

#endif // PLAYWIDGET_H

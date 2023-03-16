#include "playwidget.h"

playWidget::playWidget()
{

}

playWidget::playWidget(QString fileName)
{
    this->resize(800,368);
    //先创建解码线程对象
    m_fileName = fileName;
    this->fdec = new fdecode(m_fileName);

    connect(fdec,SIGNAL(sendImage(QImage)),this,SLOT(receiveImage(QImage)));//注意sendImage和receiveImage的参数
    //启动解码线程
    fdec->start();
}
void playWidget::paintEvent(QPaintEvent *)
{
//    QPaintEvent painter(this);
    QPainter painter(this);
    if(!this->img.isNull())
    {
        painter.drawImage(QRect(0,0,this->rect().width(),this->rect().height()),this->img);
    }
}

void playWidget::receiveImage(QImage img)
{
    this->img = img;
    this->update();
}

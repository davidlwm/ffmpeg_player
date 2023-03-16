#include "playerpanel.h"
#include "ui_playerpanel.h"
#include <QPushButton>
#include <QtWidgets>

playerpanel::playerpanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::playerpanel)
{
    ui->setupUi(this);
    this->pBtn1 = nullptr;
    this->pBtn2 = nullptr;
    this->w = nullptr;
//    this->pBtn1 = new QPushButton();
//    this->pBtn1->setFixedSize(40,40);
//    this->pBtn1->setObjectName("objBtn");
//    this->pBtn1->setStyleSheet("QPushButton#objBtn{border-radius:20px;background:white;}");
//    this->pBtn1->setText("<");
//    this->pBtn1->show();
//    this->pBtn1->setParent(this);

//    this->pBtn2 = new QPushButton();
//    this->pBtn2->setFixedSize(40,40);
//    this->pBtn2->setObjectName("objBtn1");
//    this->pBtn2->setStyleSheet("QPushButton#objBtn1{border-radius:20px;background:white;}");
//    this->pBtn2->setText(">");
//    this->pBtn2->show();
//    this->pBtn2->setParent(this);

}

void playerpanel::resizeEvent(QResizeEvent *event)
{
    QSize size = event->size();
    // 连接窗口的信号和槽以侦听窗口大小更改事件
    if(this->pBtn1 && this->pBtn2)
    {
        int x = 10;
        int y = size.height()/2;

        this->pBtn1->move(x,y);
        this->pBtn2->move(size.width()-10 - pBtn1->width() ,y);
    }
}

playerpanel::~playerpanel()
{
    delete ui;
}

#include "widget.h"
#include <QPushButton>

#include <QtGui/QKeyEvent>
#include <QtGui/QWindow>
#include <QtGui/QScreen>
#include <qwebchannel.h>
#include <QJsonDocument>

#include "playwidget.h"
#include <QtWidgets>
#include "camera.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QWebChannel* channel = new QWebChannel(this);
    channel->registerObject("CoreId", Core::getInstance());  // 向QWebChannel注册用于Qt和Web交互的对象。

    this->win = new QWidget();
    this->win->setAttribute(Qt::WA_DeleteOnClose,true);
    this->stackedWidget = new QStackedWidget();
//    ui->webEngineView->page()->setWebChannel(channel);       // 将与webEngineView要使用的web通道实例设置为channel
//    ui->webEngineView->setUrl(QDir("D:/github/QMDemo/Web/QtWebExamples/channelDemo2/web2/webClient.html").absolutePath());

    // 绑定槽函数，接收web界面中javascript传递过来的信号
    connect(Core::getInstance(), &Core::toQtBool, this, &Widget::on_toQtBool);
    connect(Core::getInstance(), &Core::toQtDouble, this, &Widget::on_toQtDouble);
    connect(Core::getInstance(), &Core::toQtString, this, &Widget::on_toQtString);
    connect(Core::getInstance(), &Core::toQtJsonArray, this, &Widget::on_toQtJsonArray);
    connect(Core::getInstance(), &Core::toQtJsonObject, this, &Widget::on_toQtJsonObject);
}

Widget::~Widget()
{

}

// 键盘事件处理函数
void Widget::keyPressEvent(QKeyEvent* event)
{
    // 按下 F11 切换全屏和窗口模式
    if (event->key() == Qt::Key_F11)
    {
        if (isFullScreen())
        {
            showNormal();
            setWindowFlags(Qt::Window);
            show();
        }
        else
        {
            setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
            showFullScreen();
        }
    }
    // 按下 ESC 退出全屏模式
    else if (event->key() == Qt::Key_Escape && isFullScreen())
    {
        showNormal();
        setWindowFlags(Qt::Window);
        show();
    }
    // 按下 F12
    else if (event->key() == Qt::Key_F12)
    {
        QWidget *win = new QWidget();
        win->setAttribute(Qt::WA_DeleteOnClose,true);
        playWidget *p1 = new playWidget("C:/aaaa.mp4");

        QGridLayout *gridLayout = new QGridLayout(this->win);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->addWidget(p1, 0, 0);
        win->setWindowIcon(QIcon(":/icon.jpeg"));
        win->setWindowTitle("Camera");
        win->setLayout(gridLayout);
        win->resize(800, 400);
        win->show();
    }
    // 按下 F9
    else if (event->key() == Qt::Key_F9)
    {
        QWidget * showCamera = new QWidget();
        //showCamera->setAttribute(Qt::WA_DeleteOnClose,true);
        QString sourceId =  "807C62CDF1A8";
        QString p2pid = "UZ93CNTYH6KYHWY7111A";
        camera *cam1 = new camera();
        cam1->Init(sourceId,p2pid);
        //添加网格布局
        QGridLayout *gridLayout = new QGridLayout(showCamera);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->addWidget(cam1, 0, 0);

        showCamera->setWindowIcon(QIcon(":/icon.jpeg"));
        showCamera->setWindowTitle("Camera");
        showCamera->setLayout(gridLayout);
        showCamera->resize(800, 600);
        showCamera->show();
    }

}


/**
 * @brief       显示Web发送给Qt的数据
 * @param value
 */
void Widget::on_toQtBool(bool value)
{
    //ui->textEdit->append(QString("Bool类型数据：%1").arg(value ? "true" : "false"));
    if (this->stackedWidget->currentIndex() == 0)
        this->stackedWidget->setCurrentIndex(1);
    else
        this->stackedWidget->setCurrentIndex(0);
}

void Widget::on_toQtDouble(double value)
{
    //ui->textEdit->append(QString("double类型数据：%1").arg(value));
}

void Widget::on_toQtString(QString value)
{
    //ui->textEdit->append(QString("QString类型数据：%1").arg(value));
}

void Widget::on_toQtJsonArray(QJsonArray value)
{
    QJsonDocument doc;
    doc.setArray(value);
    //ui->textEdit->append(QString("QJsonArray类型数据：%1").arg(doc.toJson().data()));
}

void Widget::on_toQtJsonObject(QJsonObject value)
{
    QJsonDocument doc;
    doc.setObject(value);
    //ui->textEdit->append(QString("QJsonArray类型数据：%1").arg(doc.toJson().data()));
}



/**
 * @brief        带返回值的槽函数，将返回值传递给javascript
 * @param value
 * @return
 */
QString Core::on_returnValue(int value)
{
    qDebug() << "调用Qt槽函数，并返回值";
    return QString("调用成功：%1").arg(value);
}

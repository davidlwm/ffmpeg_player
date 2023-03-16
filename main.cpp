
#include <QApplication>
#include <QDebug>
#include "playwidget.h"


#include <QtWidgets/QMainWindow>
#include <QtWidgets/QHBoxLayout>


#include <QWebEngineView>
#include <widget.h>
#include <QPushButton>
#include <mainwindow.h>
#include "webviewtask.h"
#include <QtWidgets>
#include "playerpanel.h"

#include "player/p2preader.h"
#include "yuv_render.h"
#include "camera.h"

#include <qdesktopwidget.h>

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


//void DrawFrame(void*obj, AVFrame *f){
////    printf("draw frame\n");
//    YUV420P_Render *render = (YUV420P_Render*)obj;
//    if(render){
////        printf("draw frame 111\n");
//        render->setFrame(f);
//    }
//}




int main01(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;


    if (InitLicense() != TUTK_ER_NoERROR){
        printf("InitLicense  failed\n");
        return 0;
    }


    //8R533SZRV22812DJ111A 3CE36B7AE4A6
    //QString sourceId =  "0043F82B6B9E";

    QString sourceId =  "807C62CDF1A8";
    P2PReader *player = new P2PReader(sourceId.toStdString());
    YUV420P_Render render;
    //player->SetCallback(&render,DrawFrame,nullptr);
    //player->Init("8R533SZRV22812DJ111A");
    player->Init("UZ93CNTYH6KYHWY7111A");
    player->Start();


    w.setCentralWidget(&render);
    //player->Stop();

    w.show();
    return a.exec();

}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QDesktopWidget *desktop = QApplication::desktop();

    //init tutk
    if (InitLicense() != TUTK_ER_NoERROR){
        printf("InitLicense  failed\n");
        return 0;
    }

    //main window
    Widget window;

//    QString sourceId =  "807C62CDF1A8";
//    QString p2pid = "UZ93CNTYH6KYHWY7111A";
    QString p2pid1 = "8R533SZRV22812DJ111A";
    camera *cam1 = new camera();
    cam1->Init("3CE36B7AE4A6",p2pid1);


    QString sourceId1 =  "78605BE20F89";
    QString p2pida = "NR1Y2WHMVS9P8ECN111A";
    camera *cam2 = new camera();
    cam2->Init("3CE36B7AE4A6",p2pid1);

    camera *cam3 = new camera();

    cam3->Init("3CE36B7AE4A6",p2pid1);

    playWidget *p4 = new playWidget("rtsp://admin:haiber0702@192.168.1.21:554/stream1");

    //4 camera play panel
    playerpanel * widgetVideo = new playerpanel();

    //添加网格布局
    QGridLayout *gridLayout = new QGridLayout(widgetVideo);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->addWidget(cam1, 0, 0);
    gridLayout->addWidget(cam2, 0, 1);
    gridLayout->addWidget(cam3, 1, 0);
    gridLayout->addWidget(p4, 1, 1);

    widgetVideo->setWindowTitle("4 Videos Player");
    widgetVideo->setLayout(gridLayout);
    widgetVideo->resize(800, 600);
    widgetVideo->show();


    //网页
    QWebEngineView *view = new QWebEngineView();
    QWidget * widgetWeb = new QWidget();
    QGridLayout *layout = new QGridLayout(widgetWeb);

    widgetWeb->setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(view, 0, 0, 0, 0);
    view->setUrl(QUrl("http://192.168.1.63:8083/#/home/"));
    widgetWeb->show();

    //将QWidget添加到QStackedWidget中
    window.stackedWidget->addWidget(widgetVideo);
    window.stackedWidget->addWidget(widgetWeb);

    // 创建一个QVBoxLayout，并将QStackedWidget和QPushButton添加到该布局中
    QVBoxLayout layout3;
    layout3.setContentsMargins(0,0,0,0);
    layout3.addWidget(window.stackedWidget);

    window.setWindowIcon(QIcon(":/icon.jpeg"));
    window.setWindowTitle(QStringLiteral("边缘计算视频监控平台"));
    window.resize(1600, 780);
    window.setLayout(&layout3);


    widgetVideo->pBtn1 = new QPushButton();
    widgetVideo->pBtn1->setFixedSize(40,40);
    widgetVideo->pBtn1->setObjectName("objBtn");
    widgetVideo->pBtn1->setStyleSheet("QPushButton#objBtn{border-radius:20px;background:white;}");
    widgetVideo->pBtn1->setText("<");
    widgetVideo->pBtn1->show();
    widgetVideo->pBtn1->setParent(widgetVideo);

    widgetVideo->pBtn2 = new QPushButton();
    widgetVideo->pBtn2->setFixedSize(40,40);
    widgetVideo->pBtn2->setObjectName("objBtn1");
    widgetVideo->pBtn2->setStyleSheet("QPushButton#objBtn1{border-radius:20px;background:white;}");
    widgetVideo->pBtn2->setText(">");
    widgetVideo->pBtn2->show();
    widgetVideo->pBtn2->setParent(widgetVideo);


    //切换camera和网页
    QObject::connect(widgetVideo->pBtn1, &QPushButton::clicked, [&](){
        // 切换QWidget的显示
        if (window.stackedWidget->currentIndex() == 0)
            window.stackedWidget->setCurrentIndex(1);
        else
            window.stackedWidget->setCurrentIndex(0);
    });

    QObject::connect(widgetVideo->pBtn2, &QPushButton::clicked, [&](){
        // 切换QWidget的显示
        if (window.stackedWidget->currentIndex() == 0)
            window.stackedWidget->setCurrentIndex(1);
        else
            window.stackedWidget->setCurrentIndex(0);
    });


    //居中
    window.move((desktop->width() - window.width())/ 2, (desktop->height() - window.height()) /2);
    //window.showFullScreen();
    window.show();

    qDebug()<<"version:"<<avcodec_version();
    return a.exec();
}






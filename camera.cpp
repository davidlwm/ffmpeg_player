#include "camera.h"



void DrawFrame1(void*obj, AVFrame *f){
//    printf("draw frame\n");
    YUV420P_Render *render = (YUV420P_Render*)obj;
    if(render){
//        printf("draw frame 111\n");
        render->setFrame(f);
    }
}


camera::camera()
{

}


void camera::Init(QString sourceId,QString p2pUid)
{
    P2PReader *player = new P2PReader(sourceId.toStdString());

    player->SetCallback(this,DrawFrame1,nullptr);
    player->Init(p2pUid.toStdString().c_str());
    player->Start();
}

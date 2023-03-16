#include "fdecode.h"
#include <QDebug>
fdecode::fdecode()
{

}

fdecode::fdecode(QString fileName)
{
    this->fileName = fileName;
}
void fdecode::openVideoStream(QString fileName)
{
    //参数1:封装格式上下文->AVFormatContext->包含了视频信息(视频格式、大小等等...)双指针定义一颗星*，
    //参数2:要打开流的路径（文件名）
    //AVFormatContext保存视频（视频流）相关信息的结构体
    AVFormatContext  * formatContent = avformat_alloc_context();
//        char * filename="Warcraft3_End.avi";
    /*2、打开视频文件或者摄像头*/
//    AVInputFormat *fmt = av_find_input_format("dshow");//windows上视频流的格式
//    int res = avformat_open_input(&formatContent,"video=Integrated Camera",fmt,nullptr);//打开视频文件
        int res = avformat_open_input(&formatContent,fileName.toStdString().c_str(),nullptr,nullptr);//打开摄像头

    if(res!=0)
    {
        qDebug()<<"打开视频失败";
        return ;
    }
//    QList<QCameraInfo> cameraList = QCameraInfo::availableCameras();
//    qDebug()<<cameraList.size()<<endl;//摄像头个数
//    for(int i=0;i<cameraList.size();i++)
//    {
//        qDebug()<<cameraList.at(i).description();//摄像头名称
//    }

    /*3、打开成功之后相关的结构体信息放在了formatContent里面，接下来获取视频文件信息*/
    //3.1先看有没有视频流信息（avformat_find_stream_info），进行判断的原因是有可能打开普通文件
    res = avformat_find_stream_info(formatContent,nullptr);
    if(res<0)
    {
        //qDebug()<<"打开流媒体信息失败";
        return ;
    }


    //AVFormatContext(含有解码器的id，去streams的流数组里面找视频流)->AVStream->AVCodecContext
    //->codec（有解码器的AVCodec）->AVCodec(含有编解码器的id、类型)
    //AVCodecContext 保存视频音频编解码相关的信息

    int videoType = -1;
    //3.2一个视频中有多股码流（用循环），存在AVFormatContext的streams数组中
    for(int i=0;i<formatContent->nb_streams;i++)
    {
        if(formatContent->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)//streams->有AVStream结构体,AVStream->codec
        {
            //找到视频流(只有一个)
            videoType = i;//标识视频流这个类型
            break;
        }
    }
    if(videoType == -1)
    {
        qDebug()<<"没有找到视频流相关信息";
        return;
    }
    //对视频的编码只有编码器是不够的还要有宽高格式等
    //3.3根据视频流查找编码器对应的上下文对象结构体，存储编码器以及宽高格式等
    AVCodecContext *codec = formatContent->streams[videoType]->codec;
    /*4、有视频流，则查找对应视频流的解码器*/
    AVCodec *decoder = avcodec_find_decoder(codec->codec_id);//需要解码器的id
    if(decoder ==nullptr)
    {
        qDebug()<<"没有找到对应的解码器";
        return;
    }

    /*5、找到解码器后打开解码器*/
    //参数：1.初始化的上下文对象 2.打开的解码器 3.类似目录的东西（没有）
    res = avcodec_open2(codec,decoder,nullptr);
    if(res!=0)
    {
        qDebug()<<"解码器打开失败";
        return;
    }
    /*6、获取到的每一帧码流（视频流）数据写到文件中，进行循环解码*/
    /*6.1写到文件用FILE结构体*/
    //    FILE *fp = fopen("saveH264.h264","wb+");
    //    FILE *fpYUV = fopen("saveYUV.yuv","wb+");
    //    FILE *fpRGB = fopen("saveRGB.jpg","wb+");
    AVPacket *pkt=nullptr;//pkt这时没有指向，要我们给他分配内存空间，希望把读出来的数据放到这块内存去
    pkt = (AVPacket *)malloc(sizeof(AVPacket));
    //码流数据是存到buffer里面，也需要我们动态开空间(AVBufferRef *buf;)
    //开空间不知道一帧的码流数据是多少？其实编解码器告诉了宽高，以此可以计算出给码流数据开多大空间
    int bufSize = codec->width*codec->height;//计算一帧（图）数据的大小
    av_new_packet(pkt,bufSize);

    AVFrame *pictureRGB = nullptr;//保存解码及剔除后的像素数据（做准备）
    pictureRGB = av_frame_alloc();
    pictureRGB->width = codec->width;
    pictureRGB->height = codec->height;
    pictureRGB->format = codec->pix_fmt;//格式的设置
    //要存解码后的像素数据到pictureRGB，那这个数据有多大呢？
    //获取解码后的一帧像素数据有多大
    int numByte = avpicture_get_size(AV_PIX_FMT_RGB32,codec->width,codec->height);
    //开的空间用来保存像素数据的大小
    uint8_t *buffer = (uint8_t *)av_malloc(numByte*sizeof(uint8_t));
    //像素数据填充到AVFrame的pictureRGB里
    avpicture_fill((AVPicture *)pictureRGB,buffer,AV_PIX_FMT_RGB32,codec->width,codec->height);
    //因为解码之后要伸展，所以先进行转换规则的设置，转换完进入第七步解码
    SwsContext *swsContent = nullptr;
    swsContent = sws_getContext(codec->width,codec->height,codec->pix_fmt,
                                codec->width,codec->height,AV_PIX_FMT_RGB32,
                                SWS_BICUBIC,nullptr,nullptr,nullptr);


    //av_read_frame()参数：1最初保存信息的结构体 2包
    int count = 0;//保存帧数的变量
    while(av_read_frame(formatContent,pkt) >= 0)//成功读到了数据（一帧一帧读（循环）：av_read_frame）
    {
        /*6.2AVPacket->AVStream，要判断读到的每一帧的码流数据是不是视频流*/
        if(pkt->stream_index == videoType)
        {
            //是视频流则写到文件中
            //            fwrite(pkt->data,pkt->size,1,fp);//每次写一个结构体
            //读到一帧是视频流就进行解码的动作
            /*7、解码——得到RGB保存在AVFrame结构体里*/
            //参数：1编解码器上下文对象的结构体 2存放解码后的像素数据（AVFrame类型）
            //3判断有没有数据可以进行解码：指针类型的变量 4对谁进行解码：一帧的码流数据
            //功能：把得到的一帧码流数据用编解码器上下文对象去解，存放在AVFrame结构体里
            int got_picture_ptr = -1;

            AVFrame *picture = av_frame_alloc();//保存原始RGB数据
            avcodec_decode_video2(codec,picture,&got_picture_ptr,pkt);
            if(got_picture_ptr != 0)
            {
                //把解码得到的损坏的像素数据剔除(存到pictureRGB中)
                sws_scale(swsContent,picture->data,picture->linesize,0,picture->height,
                          pictureRGB->data,pictureRGB->linesize);
                count++;
                //if(count % 25 == 0)//每25帧保存一张图片
                // {
                //                    uchar* transData = (unsigned char*)pictureRGB->data[0];//格式装换
                QImage desImage = QImage((uchar*)buffer,codec->width,codec->height,
                                         QImage::Format_RGB32,nullptr,nullptr); //RGB32
                //                    desImage.save(QString("./pictures/rgbPicture%1.png").arg(count-25),"PNG", 100);
                //每解码一帧图像给显示窗口发送一个显示图像的信号
                emit sendImage(desImage);
                msleep(25);//播放倍速设置，可以通过延时来调


                // }


            }


        }
        //每次都存在同一块内存空间里，要清空上一次的操作
        av_packet_unref(pkt);//不是free
    }

    qDebug()<<"保存码流数据和像素数据成功";
    //    fclose(fp);
    //    fclose(fpYUV);
    //    fclose(fpRGB);

}

void fdecode::registerFFmpeg()
{
    av_register_all();//注册所有组件
    avdevice_register_all();//注册摄像头
}

void fdecode::run()
{
    this->registerFFmpeg();
    this->openVideoStream(this->fileName);
}





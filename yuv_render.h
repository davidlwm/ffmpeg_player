#ifndef YUV_RENDER_H
#define YUV_RENDER_H

extern "C" {
#include <libavutil/frame.h>
}

#include <QObject>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <mutex>
#include <widget.h>


class YUV420P_Render: protected QOpenGLFunctions, public QOpenGLWidget
{

public:
    YUV420P_Render();
    ~YUV420P_Render();

    //初始化gl
    void initialize();
    //刷新显示
    void render(uchar* py,uchar* pu,uchar* pv,int width,int height,int type);
    void render(uchar* ptr,int width,int height,int type);
    void setFrame(AVFrame *f);
    Widget *w;
    void onTimer();
//    bool event(QEvent *event) Q_DECL_OVERRIDE;
protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;

private:
    //shader程序
    QOpenGLShaderProgram m_program;
    //shader中yuv变量地址
    GLuint m_textureUniformY, m_textureUniformU , m_textureUniformV;
    //创建纹理
    GLuint m_idy , m_idu , m_idv;

    AVFrame *m_frme;
    std::mutex m_mtx;
    QTimer *timer;

};


#endif // YUV_RENDER_H

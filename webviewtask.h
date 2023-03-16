#ifndef WEBVIEWTASK_H
#define WEBVIEWTASK_H

#include <QWebEngineView>

class webviewTask : public QWebEngineView
{
public:
    explicit webviewTask(QWebEngineView *parent = 0);
    ~webviewTask();
};

#endif // WEBVIEWTASK_H

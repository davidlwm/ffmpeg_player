#ifndef PLAYERPANEL_H
#define PLAYERPANEL_H

#include <QWidget>
#include <QPushButton>
#include <widget.h>

namespace Ui {
class playerpanel;
}

class playerpanel : public QWidget
{
    Q_OBJECT

public:
    explicit playerpanel(QWidget *parent = nullptr);
    ~playerpanel();
    QPushButton *pBtn1;
    QPushButton *pBtn2;
    Widget *w;

private:
    Ui::playerpanel *ui;

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // PLAYERPANEL_H

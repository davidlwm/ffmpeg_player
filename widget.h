#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QJsonArray>
#include <QJsonObject>
#include <qdebug.h>
#include <QPushButton>
#include <QtWidgets>

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    QWidget * win;
    QPushButton *pBtn1;
    QPushButton *pBtn2;
    QStackedWidget *stackedWidget;
    void keyPressEvent(QKeyEvent* event) override;
private slots:
    void on_toQtBool(bool value)             ;
    void on_toQtDouble(double value)         ;
    void on_toQtString(QString value)        ;
    void on_toQtJsonArray(QJsonArray value)  ;
    void on_toQtJsonObject(QJsonObject value);

};

/**
 * @brief  Qt和Web端交互的中介单例类
 */
class Core : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)  // 定义一个属性，javascript可以读取属性值

public:
    enum CoreEnum
    {
        Value1 = 100,
        Value2
    };
    Q_ENUM(CoreEnum)    // 使用Q_ENUM标记的枚举，javascript可以直接访问

public:
    static Core* getInstance()
    {
        static Core core;
        return &core;
    }

    int value() {return m_value;}
    void setValue(int v) {m_value = v;}

signals:
    void valueChanged();
    /**
     * @brief     Qt发送给Web的信号
     * @param str
     */
    void toWebBool(bool value);
    void toWebDouble(double value);
    void toWebString(QString value);
    void toWebJsonArray(QJsonArray value);
    void toWebJsonObject(QJsonObject value);

    /**
     * @brief     Web发送给Qt的信号
     * @param str
     */
    void toQtBool(bool value);
    void toQtDouble(double value);
    void toQtString(QString value);
    void toQtJsonArray(QJsonArray value);
    void toQtJsonObject(QJsonObject value);

public slots:
    /**
     * @brief     Web端需要调用Qt槽函数来传递，必须声明为public slots，否则web找不到
     * @param str
     */
    void on_toQtBool(bool value)              {emit toQtBool(value);}
    void on_toQtDouble(double value)          {emit toQtDouble(value);}
    void on_toQtString(QString value)         {emit toQtString(value);}
    void on_toQtJsonArray(QJsonArray value)   {emit toQtJsonArray(value);}
    void on_toQtJsonObject(QJsonObject value) {emit toQtJsonObject(value);}

    /**
     * @brief        定义一个带有返回值的槽函数，javascript调用该函数后可以获取返回值
     * @param value
     * @return
     */
    QString on_returnValue(int value);
private:
    int m_value = 10;
};


#endif // WIDGET_H

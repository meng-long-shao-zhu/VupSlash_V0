#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

/*
定义一个新的QWidget类,将标题栏和UI界面都放到这个类里面,定义这个类是为了方便改变主题
*/
class WidgetPar : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetPar(QWidget *parent = 0);

signals:

public slots:
    //void paintEvent(QPaintEvent *event);
};

/*
自定义的标题栏类
*/
class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = 0);

signals:

public slots:
    //右上角三个按钮共用一个槽函数
    void on_TiBar_Clicked();

protected:
    //重写鼠标事件
    virtual void mouseDoubleClickEvent(QMouseEvent *event);// 双击标题栏进行界面的最大化/还原
    virtual void mousePressEvent(QMouseEvent *event);// 进行鼠界面的拖动
    virtual bool eventFilter(QObject *obj, QEvent *event);// 设置界面标题与图标

    // 标题栏跑马灯效果时钟;
    //QTimer m_titleRollTimer;
private:
    void updateMaximize();// 最大化/还原

private://自定义标题栏控件
    QLabel *TiBar_pIconLabel;               //左上角图标label
    QLabel *TiBar_pTitleLabel;              //中间标题栏label
    QPushButton *TiBar_pMinimizeBtn;        //右上角缩小到底部任务栏按钮,最小化
    QPushButton *TiBar_pMaximizeBtn;        //右上角放大缩小按钮
    QPushButton *TiBar_pCloseBtn;           //右上角关闭按钮
public:


};

#endif // TITLEBAR_H

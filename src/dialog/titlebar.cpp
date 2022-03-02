#include "titlebar.h"
#include <QHBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>

#ifdef Q_OS_WIN
#pragma comment(lib, "user32.lib")
#include <qt_windows.h>
#endif

#include <QPainter>

#include <QDebug>

TitleBar::TitleBar(QWidget *parent) :
    QWidget(parent)
{
    TiBar_pIconLabel    = new QLabel;
    TiBar_pTitleLabel   = new QLabel;
    TiBar_pMinimizeBtn  = new QPushButton;
    TiBar_pMaximizeBtn  = new QPushButton;
    TiBar_pCloseBtn     = new QPushButton;

    TiBar_pIconLabel->setFixedSize(32, 32); //设置图标固定大小
    TiBar_pIconLabel->setScaledContents(true);  //设置图片显示合适大小

    TiBar_pTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);//此属性保存窗口小部件的默认布局行为。

    TiBar_pMinimizeBtn->setFixedSize(32, 32);//设置三个按钮的固定大小
    TiBar_pMaximizeBtn->setFixedSize(32, 32);
    TiBar_pCloseBtn->setFixedSize(32, 32);
    TiBar_pTitleLabel->setFixedHeight(32);


    TiBar_pMinimizeBtn->setIcon(QIcon(":/image/image/small.ico"));
    TiBar_pMinimizeBtn->setIconSize(TiBar_pMinimizeBtn->size());
    TiBar_pMaximizeBtn->setIcon(QIcon(":/image/image/max.ico"));
    TiBar_pMaximizeBtn->setIconSize(TiBar_pMinimizeBtn->size());
    TiBar_pCloseBtn->setIcon(QIcon(":/image/image/close.ico"));
    TiBar_pCloseBtn->setIconSize(TiBar_pMinimizeBtn->size());
    QPixmap icon(":/resource/icon/sgs.ico");
    TiBar_pIconLabel->setPixmap(icon);
    //TiBar_pIconLabel->resize(icon.width(),icon.height());


    TiBar_pTitleLabel->setObjectName("whiteLabel"); //此属性保存此对象的名称。
    TiBar_pMinimizeBtn->setObjectName("minimizeButton");
    TiBar_pMaximizeBtn->setObjectName("maximizeButton");
    TiBar_pCloseBtn->setObjectName("closeButton");

    TiBar_pMinimizeBtn->setToolTip("最小化"); //鼠标停留在上面的提示
    TiBar_pMaximizeBtn->setToolTip("放大/缩小");
    TiBar_pCloseBtn->setToolTip("关闭");

    QHBoxLayout *pLayout = new QHBoxLayout(this);
    pLayout->addWidget(TiBar_pIconLabel);
    //pLayout->addSpacing(5);
    pLayout->addWidget(TiBar_pTitleLabel);
    pLayout->addWidget(TiBar_pMinimizeBtn);
    pLayout->addWidget(TiBar_pMaximizeBtn);
    pLayout->addWidget(TiBar_pCloseBtn);
    pLayout->setSpacing(0);//设置控件之间的间距
    pLayout->setContentsMargins(0, 0, 0, 0);//设置左上右下的边距

    this->setLayout(pLayout);

    connect(TiBar_pMinimizeBtn,SIGNAL(clicked()),this,SLOT(on_TiBar_Clicked()));
    connect(TiBar_pMaximizeBtn,SIGNAL(clicked()),this,SLOT(on_TiBar_Clicked()));
    connect(TiBar_pCloseBtn,SIGNAL(clicked()),this,SLOT(on_TiBar_Clicked()));
}
/*  标题栏右上角三个按钮的槽函数*/
void TitleBar::on_TiBar_Clicked()
{
    QPushButton *pBtn = qobject_cast<QPushButton *>(sender());//如果在由信号激活的槽中调用，则返回指向发送信号的对象的指针; 否则它返回0.
    /*  返回此窗口小部件的窗口，即具有（或可能具有）窗口系统框架的下一个祖先窗口小部件。 如果窗口小部件是窗口，则返回窗口小部件本身。*/
    QWidget *pWindow = this->window();
    if (pWindow->isWindow())//如果窗口小部件是独立窗口，则返回true，否则返回false。
    {
        if (pBtn == TiBar_pMinimizeBtn)
        {
            pWindow->showMinimized();//最小化窗口小部件，作为图标。调用此函数仅影响窗口。
        }
        else if (pBtn == TiBar_pMaximizeBtn)
        {
            pWindow->isMaximized() ? pWindow->showNormal() : pWindow->showMaximized();//放大缩小
        }
        else if (pBtn == TiBar_pCloseBtn)
        {
            pWindow->close();//关闭窗口
        }
    }
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    emit TiBar_pMaximizeBtn->clicked();
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    if (ReleaseCapture())
    {
        QWidget *pWindow = this->window();
        if (pWindow->isWindow())
        {
           SendMessage(HWND(pWindow->winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
        }
    }
    event->ignore();
}

bool TitleBar::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type())
    {
        case QEvent::WindowTitleChange:
        {
            QWidget *pWidget = qobject_cast<QWidget *>(obj);
            if (pWidget)
            {
                TiBar_pTitleLabel->setText(pWidget->windowTitle());
                return true;
            }
        }
        case QEvent::WindowIconChange:
        {
            QWidget *pWidget = qobject_cast<QWidget *>(obj);
            if (pWidget)
            {
                QIcon icon = pWidget->windowIcon();
                TiBar_pIconLabel->setPixmap(icon.pixmap(TiBar_pIconLabel->size()));
                return true;
            }
        }
        case QEvent::WindowStateChange:
        case QEvent::Resize:
            //updateMaximize();
            return true;
        default:
            return false;
    }
    return QWidget::eventFilter(obj, event);
}
//窗口大小发生改变
void TitleBar::updateMaximize()
{
    QWidget *pWindow = this->window();
    if (pWindow->isTopLevel())
    {
        bool bMaximize = pWindow->isMaximized();
        if (bMaximize)
        {
            TiBar_pMaximizeBtn->setToolTip(tr("Restore"));
            TiBar_pMaximizeBtn->setProperty("maximizeProperty", "restore");
        }
        else
        {
            TiBar_pMaximizeBtn->setProperty("maximizeProperty", "maximize");
            TiBar_pMaximizeBtn->setToolTip(tr("Maximize"));
        }
        TiBar_pMaximizeBtn->setStyle(QApplication::style());
    }
}

WidgetPar::WidgetPar(QWidget *parent) :
    QWidget(parent)
{

}



#ifndef _WINDOW_H
#define _WINDOW_H

//#include "button.h"
class Button;

class Window : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit Window(const QString &title, const QSizeF &size, const QString &path = QString());
    ~Window();

    void addContent(const QString &content);
    Button *addCloseButton(const QString &label);
    void shift(int pos_x = 0, int pos_y = 0);
    void keepWhenDisappear();
    void setTitle(const QString &title);

    virtual QRectF boundingRect() const;

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

public slots:
    void appear();
    void disappear();
    void setFullOpacity(qreal value = 1.0);

private:
    QGraphicsTextItem *titleItem;
    QGraphicsScale *scaleTransform;
    QSizeF size;
    bool keep_when_disappear;
    QImage *outimg;
    QPixmap *bg;
    qreal full_opacity;
};

#endif


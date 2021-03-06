#ifndef _CARD_ITEM_H
#define _CARD_ITEM_H

#include "qsan-selectable-item.h"
#include "settings.h"
#include "engine.h"

class FilterSkill;
class General;
class Card;

class CardItem : public QSanSelectableItem
{
    Q_OBJECT
    Q_PROPERTY(qreal X_rotate READ X_rotate WRITE setX_rotate)
    Q_PROPERTY(qreal Y_rotate READ Y_rotate WRITE setY_rotate)

public:
    CardItem(const Card *card);
    CardItem(const QString &general_name);
    ~CardItem();

    virtual QRectF boundingRect() const;
    virtual void setEnabled(bool enabled);

    const Card *getCard() const;
    void setCard(const Card *card);
    inline int getId() const
    {
        return m_cardId;
    }

    // For move card animation
    void setHomePos(QPointF home_pos, float rotate_angle = 0, float slap = 0, float is_hand = 0);
    QPointF homePos() const;
    QAbstractAnimation *getGoBackAnimation(bool doFadeEffect, bool smoothTransition = false,
        int duration = Config.S_MOVE_CARD_ANIMATION_DURATION);
    void goBack(bool playAnimation, bool doFade = true, int type = 0);
    inline QAbstractAnimation *getCurrentAnimation(bool)
    {
        return m_currentAnimation;
    }
    inline void setHomeOpacity(double opacity)
    {
        m_opacityAtHome = opacity;
    }
    inline double getHomeOpacity()
    {
        return m_opacityAtHome;
    }

    void showAvatar(const General *general);
    void hideAvatar();
    void showSuitNumber();
    void hideSuitNumber();
    void setShiny(bool shiny);
    void setNumber(int number);
    void setColor(Card::Color color);
    void setSuit(Card::Suit suit);
    void setAutoBack(bool auto_back);
    void changeGeneral(const QString &general_name);
    void setFootnote(const QString &desc);
    qreal x_rotate = 0;
    void setX_rotate(qreal val, bool fixed = false);
    qreal X_rotate() const;
    bool x_fixed = false;
    qreal y_rotate = 0;
    void setY_rotate(qreal val, bool fixed = false);
    qreal Y_rotate() const;
    bool y_fixed = false;
    bool z_fixed = false;

    inline bool isSelected() const
    {
        return m_isSelected;
    }
    inline void setSelected(bool selected)
    {
        m_isSelected = selected;
    }
    bool isEquipped() const;

    void setFrozen(bool is_frozen);

    inline void showFootnote()
    {
        _m_showFootnote = true;
    }
    inline void hideFootnote()
    {
        _m_showFootnote = false;
    }

    static CardItem *FindItem(const QList<CardItem *> &items, int card_id);

    struct UiHelper
    {
        int tablePileClearTimeStamp;
    } m_uiHelper;

    void clickItem()
    {
        emit clicked();
    }

private slots:
    void currentAnimationDestroyed();

protected:
    void _initialize();
    QAbstractAnimation *m_currentAnimation;
    QImage _m_footnoteImage;
    bool _m_showFootnote;
    bool _m_showSuitNumber;
    Card::Suit m_suit;
    Card::Color m_color;
    int m_number;
    // QGraphicsPixmapItem *_m_footnoteItem;
    QMutex m_animationMutex;
    double m_opacityAtHome;
    bool m_isSelected;
    bool _m_isUnknownGeneral;
    static const int _S_CLICK_JITTER_TOLERANCE;
    static const int _S_MOVE_JITTER_TOLERANCE;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    //virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    int m_cardId;
    QString _m_avatarName;
    QPointF home_pos;
    QPointF _m_lastMousePressScenePos;
    bool auto_back, frozen;
    bool m_isShiny;
    bool m_isMine;
    float rotate_angle;
    float slap;
    float is_hand;

signals:
    void toggle_discards();
    void clicked();
    void double_clicked();
    void thrown();
    void released();
    void enter_hover();
    void leave_hover();
    void movement_animation_finished();
};

#endif


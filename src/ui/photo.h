#ifndef _PHOTO_H
#define _PHOTO_H

//#include "qsan-selectable-item.h"
#include "player.h"
//#include "carditem.h"
//#include "protocol.h"

#include "generic-cardcontainer-ui.h"
//#include "sprite.h"

class ClientPlayer;
class RoleComboBox;
class QPushButton;
class CardItem;
class Sprite;

class Photo : public PlayerCardContainer
{
    Q_OBJECT
    Q_PROPERTY(qreal Y_rotate READ Y_rotate WRITE setY_rotate)
    Q_PROPERTY(qreal Z_rotate READ Z_rotate WRITE setZ_rotate)

public:
    explicit Photo();
    ~Photo();
    const ClientPlayer *getPlayer() const;
    void speak(const QString &content);
    virtual void repaintAll();
    QList<CardItem *> removeCardItems(const QList<int> &card_id, Player::Place place);

    void setEmotion(const QString &emotion, bool permanent = false);
    void tremble();
    void flip(double from, double to, int time);
    void die_throw(double off_x, double off_y);
    void revive();
    void showSkillName(const QString &skill_name);

    enum FrameType
    {
        S_FRAME_PLAYING,
        S_FRAME_RESPONDING,
        S_FRAME_SOS,
        S_FRAME_NO_FRAME
    };

    void setFrame(FrameType type);
    virtual QRectF boundingRect() const;
    QGraphicsItem *getMouseClickReceiver();
    qreal y_rotate = 0;
    void setY_rotate(qreal val);
    qreal Y_rotate() const;
    qreal z_rotate = 0;
    void setZ_rotate(qreal val);
    qreal Z_rotate() const;
    qreal m_x;
    qreal m_y;

public slots:
    void updatePhase();
    void hideEmotion();
    void hideSkillName();
    virtual void updateDuanchang();
    virtual void refresh(bool killed = false);

protected:
    inline virtual QGraphicsItem *_getEquipParent()
    {
        return _m_groupMain;
    }
    inline virtual QGraphicsItem *_getDelayedTrickParent()
    {
        return _m_groupMain;
    }
    inline virtual QGraphicsItem *_getAvatarParent()
    {
        return _m_groupMain;
    }
    inline virtual QGraphicsItem *_getMarkParent()
    {
        return _m_floatingArea;
    }
    inline virtual QGraphicsItem *_getPhaseParent()
    {
        return _m_groupMain;
    }
    inline virtual QGraphicsItem *_getRoleComboBoxParent()
    {
        return _m_groupMain;
    }
    inline virtual QGraphicsItem *_getProgressBarParent()
    {
        return this;
    }
    inline virtual QGraphicsItem *_getFocusFrameParent()
    {
        return _m_groupMain;
    }
    inline virtual QGraphicsItem *_getDeathIconParent()
    {
        return _m_groupDeath;
    }
    virtual QGraphicsItem *_getPileParent()
    {
        return _m_groupMain;
    }
    inline virtual QString getResourceKeyName()
    {
        return QSanRoomSkin::S_SKIN_KEY_PHOTO;
    }
    virtual void _adjustComponentZValues(bool killed = false);
    bool _addCardItems(QList<CardItem *> &card_items, const CardsMoveStruct &moveInfo);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    bool _m_isReadyIconVisible;
    FrameType _m_frameType;
    QGraphicsPixmapItem *_m_mainFrame;
    Sprite *emotion_item;
    QGraphicsPixmapItem *_m_skillNameItem;
    QGraphicsPixmapItem *_m_focusFrame;
    QGraphicsPixmapItem *_m_onlineStatusItem;
    QGraphicsRectItem *_m_duanchangMask;
};

#endif


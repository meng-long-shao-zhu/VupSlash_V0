#include "photo.h"
#include "clientplayer.h"
#include "settings.h"
#include "carditem.h"
#include "engine.h"
#include "standard.h"
#include "client.h"
#include "playercarddialog.h"
#include "rolecombobox.h"
#include "skin-bank.h"
#include "sprite.h"

#include "pixmapanimation.h"

using namespace QSanProtocol;

// skins that remain to be extracted:
// equips
// mark
// emotions
// hp
// seatNumber
// death logo
// kingdom mask and kingdom icon (decouple from player)
// make layers (drawing order) configurable

Photo::Photo() : PlayerCardContainer()
{
    _m_mainFrame = NULL;
    m_player = NULL;
    _m_focusFrame = NULL;
    _m_onlineStatusItem = NULL;
    _m_seatStatusItem = NULL;
    _m_layout = &G_PHOTO_LAYOUT;
    _m_frameType = S_FRAME_NO_FRAME;
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setTransform(QTransform::fromTranslate(-G_PHOTO_LAYOUT.m_normalWidth / 2, -G_PHOTO_LAYOUT.m_normalHeight / 2), true);
    _m_skillNameItem = new QGraphicsPixmapItem(_m_groupMain);

    emotion_item = new Sprite(_m_groupMain);

    _m_duanchangMask = new QGraphicsRectItem(_m_groupMain);
    _m_duanchangMask->setRect(boundingRect());
    _m_duanchangMask->setZValue(32767.0);
    _m_duanchangMask->setOpacity(0.6);
    _m_duanchangMask->hide();
    QBrush duanchang_brush(G_PHOTO_LAYOUT.m_duanchangMaskColor);
    _m_duanchangMask->setBrush(duanchang_brush);

    seat_str = "";

    _createControls();
}

Photo::~Photo()
{
    if (emotion_item) {
        delete emotion_item;
        emotion_item = NULL;
    }
}

void Photo::refresh(bool killed)
{
    PlayerCardContainer::refresh(killed);
    if (!m_player) return;
    QString state_str = m_player->getState();
    if (!state_str.isEmpty() && state_str != "online") {
        QRect rect = G_PHOTO_LAYOUT.m_onlineStatusArea;
        QImage image(rect.size(), QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.fillRect(QRect(0, 0, rect.width(), rect.height()), G_PHOTO_LAYOUT.m_onlineStatusBgColor);
        G_PHOTO_LAYOUT.m_onlineStatusFont.paintText(&painter, QRect(QPoint(0, 0), rect.size()),
            Qt::AlignCenter,
            Sanguosha->translate(state_str));
        QPixmap pixmap = QPixmap::fromImage(image);
        _paintPixmap(_m_onlineStatusItem, rect, pixmap, _m_groupMain);
        _layBetween(_m_onlineStatusItem, _m_mainFrame, _m_chainIcon);
        if (!_m_onlineStatusItem->isVisible()) _m_onlineStatusItem->show();
    } else if (_m_onlineStatusItem != NULL && state_str == "online")
        _m_onlineStatusItem->hide();

    if (!state_str.isEmpty() && state_str == "ready") { //only get the seat while game ready
        int seat = m_player->getSeat();
        if (seat && seat > 0 && seat < 11) {
            seat_str = Sanguosha->translate("CAPITAL("+QString::number(seat)+")")+Sanguosha->translate("SEAT_NUM");
        }
    }
    QRect rect = G_PHOTO_LAYOUT.m_seatStatusArea;
    //rect.translate(0, 20);
    QImage image(rect.size(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.fillRect(QRect(0, 0, rect.width(), rect.height()), G_PHOTO_LAYOUT.m_onlineStatusBgColor);
    G_PHOTO_LAYOUT.m_onlineStatusFont.paintText(&painter, QRect(QPoint(0, 0), rect.size()),
        Qt::AlignCenter,
        seat_str);
    QPixmap pixmap = QPixmap::fromImage(image);
    _paintPixmap(_m_seatStatusItem, rect, pixmap, _m_groupMain);
    _layBetween(_m_seatStatusItem, _m_mainFrame, _m_chainIcon);
    if (!_m_seatStatusItem->isVisible()) _m_seatStatusItem->show();
    if (seat_str == "") _m_seatStatusItem->hide();
}

QRectF Photo::boundingRect() const
{
    return QRect(0, 0, G_PHOTO_LAYOUT.m_normalWidth, G_PHOTO_LAYOUT.m_normalHeight);
}

void Photo::repaintAll()
{
    resetTransform();
    setTransform(QTransform::fromTranslate(-G_PHOTO_LAYOUT.m_normalWidth / 2, -G_PHOTO_LAYOUT.m_normalHeight / 2), true);
    _paintPixmap(_m_mainFrame, G_PHOTO_LAYOUT.m_mainFrameArea, QSanRoomSkin::S_SKIN_KEY_MAINFRAME);
    setFrame(_m_frameType);
    hideSkillName(); // @todo: currently we don't adjust skillName's position for simplicity,
    // consider repainting it instead of hiding it in the future.
    PlayerCardContainer::repaintAll();
    refresh();
}

void Photo::_adjustComponentZValues(bool killed)
{
    PlayerCardContainer::_adjustComponentZValues(killed);
    _layBetween(_m_mainFrame, _m_faceTurnedIcon, _m_equipRegions[3]);
    _layBetween(emotion_item, _m_chainIcon, _m_roleComboBox);
    _layBetween(_m_skillNameItem, _m_chainIcon, _m_roleComboBox);
    _m_progressBarItem->setZValue(_m_groupMain->zValue() + 1);
}

void Photo::setY_rotate(qreal val)
{
    QRectF rect = this->boundingRect();
    y_rotate = val;
    //以y轴进行旋转
    QTransform transform;
    //transform.translate(rect.x(), rect.y() - rect.height()/4);
    //transform.translate(rect.width()/2, -rect.height()/2);
    transform.rotate(y_rotate, Qt::YAxis);
    transform.translate(-rect.width()/2, -rect.height()/2);
    //transform.translate(-rect.x(), -rect.y() + rect.height()/4);
    this->setTransform(transform);
}

qreal Photo::Y_rotate() const
{
    return y_rotate;
}

void Photo::setZ_rotate(qreal val)
{
    QRectF rect = this->boundingRect();
    z_rotate = val;
    //以z轴进行旋转
    QTransform transform;
    //transform.translate(rect.x(), rect.y() - rect.height()/4);
    //transform.translate(rect.width()/2, -rect.height()/2);
    transform.rotate(z_rotate, Qt::ZAxis);
    transform.translate(-rect.width()/2, -rect.height()/2);
    //transform.translate(-rect.x(), -rect.y() + rect.height()/4);
    this->setTransform(transform);
}

qreal Photo::Z_rotate() const
{
    return z_rotate;
}

void Photo::setEmotion(const QString &emotion, bool permanent)
{
    if (emotion == ".") {
        hideEmotion();
        return;
    }

    QString path = QString("image/system/emotion/%1.png").arg(emotion);
    if (QFile::exists(path)) {
        QPixmap pixmap = QPixmap(path);
        emotion_item->setPixmap(pixmap);
        emotion_item->setPos((G_PHOTO_LAYOUT.m_normalWidth - pixmap.width()) / 2,
            (G_PHOTO_LAYOUT.m_normalHeight - pixmap.height()) / 2);
        _layBetween(emotion_item, _m_chainIcon, _m_roleComboBox);

        QPropertyAnimation *appear = new QPropertyAnimation(emotion_item, "opacity");
        appear->setStartValue(0.0);
        if (permanent) {
            appear->setEndValue(1.0);
            appear->setDuration(500);
        } else {
            appear->setKeyValueAt(0.25, 1.0);
            appear->setKeyValueAt(0.75, 1.0);
            appear->setEndValue(0.0);
            appear->setDuration(2000);
        }
        appear->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        PixmapAnimation::GetPixmapAnimation(this, emotion);
    }
}

void Photo::tremble()
{
    QPropertyAnimation *vibrate = new QPropertyAnimation(this, "x");
    static qreal offset = 20;

    vibrate->setKeyValueAt(0.5, x() - offset);
    vibrate->setEndValue(x());

    vibrate->setEasingCurve(QEasingCurve::OutInBounce);

    vibrate->start(QAbstractAnimation::DeleteWhenStopped);
}

void Photo::flip(double from, double to, int time)
{
    QPropertyAnimation *anim = new QPropertyAnimation(this, "Y_rotate");

    //anim->setKeyValueAt(0.5, 90);
    anim->setStartValue(from);
    anim->setEndValue(to);
    anim->setDuration(time);

    anim->setEasingCurve(QEasingCurve::OutQuad);

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void Photo::die_throw(double off_x, double off_y)
{
    QPropertyAnimation *anim_x = new QPropertyAnimation(this, "x");
    QPropertyAnimation *anim_y = new QPropertyAnimation(this, "y");
    QPropertyAnimation *anim_r = new QPropertyAnimation(this, "Z_rotate");

    m_x = x();
    m_y = y();

    anim_x->setEndValue(x() + off_x);
    anim_y->setEndValue(y() + off_y);
    anim_r->setEndValue(qrand() % 120 - 60);

    anim_x->setDuration(500);
    anim_y->setDuration(500);
    anim_r->setDuration(500);

    anim_x->setEasingCurve(QEasingCurve::OutQuad);
    anim_y->setEasingCurve(QEasingCurve::OutQuad);
    anim_r->setEasingCurve(QEasingCurve::OutQuad);

    anim_x->start(QAbstractAnimation::DeleteWhenStopped);
    anim_y->start(QAbstractAnimation::DeleteWhenStopped);
    anim_r->start(QAbstractAnimation::DeleteWhenStopped);
}

void Photo::revive()
{
    this->setProperty("x", m_x);
    this->setProperty("y", m_y);
    this->setProperty("Z_rotate", 0);
    this->setProperty("Y_rotate", 0);
}

void Photo::showSkillName(const QString &skill_name)
{
    G_PHOTO_LAYOUT.m_skillNameFont.paintText(_m_skillNameItem,
        G_PHOTO_LAYOUT.m_skillNameArea,
        Qt::AlignCenter,
        Sanguosha->translate(skill_name));
    _m_skillNameItem->show();
    QTimer::singleShot(1000, this, SLOT(hideSkillName()));
}

void Photo::hideSkillName()
{
    _m_skillNameItem->hide();
}

void Photo::hideEmotion()
{
    QPropertyAnimation *disappear = new QPropertyAnimation(emotion_item, "opacity");
    disappear->setStartValue(1.0);
    disappear->setEndValue(0.0);
    disappear->setDuration(500);
    disappear->start(QAbstractAnimation::DeleteWhenStopped);
}

void Photo::updateDuanchang()
{
    if (!m_player) return;
    _m_duanchangMask->setVisible(m_player->getMark("@duanchang") > 0 || m_player->getMark("surrender") > 0);
}

const ClientPlayer *Photo::getPlayer() const
{
    return m_player;
}

void Photo::speak(const QString &)
{
}

QList<CardItem *> Photo::removeCardItems(const QList<int> &card_ids, Player::Place place)
{
    QList<CardItem *> result;
    if (place == Player::PlaceHand || place == Player::PlaceSpecial) {
        result = _createCards(card_ids);
        updateHandcardNum();
    } else if (place == Player::PlaceEquip) {
        result = removeEquips(card_ids);
    } else if (place == Player::PlaceDelayedTrick) {
        result = removeDelayedTricks(card_ids);
    }

    // if it is just one card from equip or judge area, we'd like to keep them
    // to start from the equip/trick icon.
    if (result.size() > 1 || (place != Player::PlaceEquip && place != Player::PlaceDelayedTrick))
        _disperseCards(result, G_PHOTO_LAYOUT.m_cardMoveRegion, Qt::AlignCenter, false, false);

    update();
    return result;
}

bool Photo::_addCardItems(QList<CardItem *> &card_items, const CardsMoveStruct &moveInfo)
{
    _disperseCards(card_items, G_PHOTO_LAYOUT.m_cardMoveRegion, Qt::AlignCenter, true, false);
    double homeOpacity = 0.0;
    bool destroy = true;

    Player::Place place = moveInfo.to_place;

    foreach(CardItem *card_item, card_items)
        card_item->setHomeOpacity(homeOpacity);
    if (place == Player::PlaceEquip) {
        addEquips(card_items);
        destroy = false;
    } else if (place == Player::PlaceDelayedTrick) {
        addDelayedTricks(card_items);
        destroy = false;
    } else if (place == Player::PlaceHand) {
        updateHandcardNum();
    }
    return destroy;
}

void Photo::setFrame(FrameType type)
{
    _m_frameType = type;
    if (type == S_FRAME_NO_FRAME) {
        if (_m_focusFrame) {
            if (_m_saveMeIcon && _m_saveMeIcon->isVisible())
                setFrame(S_FRAME_SOS);
            else if (m_player->getPhase() != Player::NotActive)
                setFrame(S_FRAME_PLAYING);
            else
                _m_focusFrame->hide();
        }
    } else {
        _paintPixmap(_m_focusFrame, G_PHOTO_LAYOUT.m_focusFrameArea,
            _getPixmap(QSanRoomSkin::S_SKIN_KEY_FOCUS_FRAME, QString::number(type)),
            _m_groupMain);
        _layBetween(_m_focusFrame, _m_avatarArea, _m_mainFrame);
        _m_focusFrame->show();
    }
    update();
}

void Photo::updatePhase()
{
    PlayerCardContainer::updatePhase();
    if (m_player->getPhase() != Player::NotActive)
        setFrame(S_FRAME_PLAYING);
    else
        setFrame(S_FRAME_NO_FRAME);
}

void Photo::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

QGraphicsItem *Photo::getMouseClickReceiver()
{
    return this;
}


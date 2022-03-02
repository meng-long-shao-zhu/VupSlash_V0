#include "carditem.h"
#include "engine.h"
#include "skill.h"
#include "clientplayer.h"
#include "settings.h"
#include "skin-bank.h"

void CardItem::_initialize()
{
    setFlag(QGraphicsItem::ItemIsMovable);
    m_opacityAtHome = 1.0;
    m_currentAnimation = NULL;
    _m_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    _m_height = G_COMMON_LAYOUT.m_cardNormalHeight;
    _m_showFootnote = true;
    _m_showSuitNumber = true;
    m_isSelected = false;
    _m_isUnknownGeneral = false;
    auto_back = true;
    frozen = false;
    m_isShiny = false;
    m_isMine = false;
    m_number = 0;
    m_suit = Card::NoSuit;
    m_color = Card::Colorless;
    rotate_angle = 0;
    slap = 0;
    setTransformOriginPoint(_m_width / 2, _m_height / 2);
    resetTransform();
    setTransform(QTransform::fromTranslate(-_m_width / 2, -_m_height / 2), true);
}

CardItem::CardItem(const Card *card)
{
    _initialize();
    //m_isShiny = (qrand() <= ((RAND_MAX + 1L) / 4096));
    setCard(card);
    setAcceptHoverEvents(true);
}

CardItem::CardItem(const QString &general_name)
{
    m_cardId = Card::S_UNKNOWN_CARD_ID;
    _initialize();
    changeGeneral(general_name);
    m_isShiny = false;
    m_isMine = false;
    m_currentAnimation = NULL;
    m_opacityAtHome = 1.0;
}

QRectF CardItem::boundingRect() const
{
    return (m_isMine) ? QRect(-5,-30,98,160) : G_COMMON_LAYOUT.m_cardFrameArea;
}

void CardItem::setCard(const Card *card)
{
    if (card != NULL) {
        m_cardId = card->getId();
        const Card *engineCard = Sanguosha->getEngineCard(m_cardId);
        Q_ASSERT(engineCard != NULL);
        setObjectName(engineCard->objectName());
        m_color = engineCard->getColor();
        m_suit = engineCard->getSuit();
        m_number = engineCard->getNumber();
        QString description = engineCard->getDescription();
        if (card->hasFlag("potato_mine"))
            m_isMine = true;
        if (m_isMine)
            description.prepend(Sanguosha->translate("potato_mine_card_state"));
        //if (m_isShiny)
        //    description = QString("<font color=#FF0000>%1</font>").arg(description);
        setToolTip(description);
    } else {
        m_cardId = Card::S_UNKNOWN_CARD_ID;
        setObjectName("unknown");
    }
}

void CardItem::setEnabled(bool enabled)
{
    QSanSelectableItem::setEnabled(enabled);
}

CardItem::~CardItem()
{
    m_animationMutex.lock();
    if (m_currentAnimation != NULL) {
        delete m_currentAnimation;
        m_currentAnimation = NULL;
    }
    m_animationMutex.unlock();
}

void CardItem::changeGeneral(const QString &general_name)
{
    setObjectName(general_name);
    const General *general = Sanguosha->getGeneral(general_name);
    if (general) {
        _m_isUnknownGeneral = false;
        setToolTip(general->getSkillDescription(true));
    } else {
        _m_isUnknownGeneral = true;
        setToolTip(QString());
    }
}

const Card *CardItem::getCard() const
{
    return Sanguosha->getCard(m_cardId);
}

void CardItem::setX_rotate(qreal val, bool fixed)
{
    x_rotate = val;
    x_fixed = fixed;
    //以x轴进行旋转
    QTransform transform;
    transform.rotate(x_rotate, Qt::XAxis);
    transform.rotate(y_rotate, Qt::YAxis);
    transform.translate(-_m_width/2, -_m_height/2);
    this->setTransform(transform);
}

qreal CardItem::X_rotate() const
{
    return x_rotate;
}

void CardItem::setY_rotate(qreal val, bool fixed)
{
    y_rotate = val;
    y_fixed = fixed;
    //以y轴进行旋转
    QTransform transform;
    transform.rotate(x_rotate, Qt::XAxis);
    transform.rotate(y_rotate, Qt::YAxis);
    transform.translate(-_m_width/2, -_m_height/2);
    this->setTransform(transform);
}

qreal CardItem::Y_rotate() const
{
    return y_rotate;
}

void CardItem::setHomePos(QPointF home_pos, float rotate_angle, float slap, float is_hand)
{
    this->home_pos = home_pos;
    this->rotate_angle = rotate_angle;
    this->slap = slap;
    this->is_hand = is_hand;
}

QPointF CardItem::homePos() const
{
    return home_pos;
}

void CardItem::goBack(bool playAnimation, bool doFade, int type)
{
    if (type == 1) {
        z_fixed = true;
    } else {
        z_fixed = false;
    }
    if (playAnimation) {
        getGoBackAnimation(doFade);
        if (m_currentAnimation != NULL)
            m_currentAnimation->start();
    } else {
        m_animationMutex.lock();
        if (m_currentAnimation != NULL) {
            m_currentAnimation->stop();
            delete m_currentAnimation;
            m_currentAnimation = NULL;
        }
        setPos(homePos());
        m_animationMutex.unlock();
    }
}

QAbstractAnimation *CardItem::getGoBackAnimation(bool doFade, bool smoothTransition, int duration)
{
    m_animationMutex.lock();
    if (m_currentAnimation != NULL) {
        m_currentAnimation->stop();
        delete m_currentAnimation;
        m_currentAnimation = NULL;
    }
    QParallelAnimationGroup *group = new QParallelAnimationGroup;

    QPropertyAnimation *goback = new QPropertyAnimation(this, "pos");
    goback->setEndValue(home_pos);
    goback->setEasingCurve(QEasingCurve::OutQuad);
    goback->setDuration(duration);
    if (is_hand){
        QPoint check_pos = QPoint(home_pos.x(), home_pos.y() - 120.0);
        goback->setKeyValueAt(0.55, check_pos);
        goback->setKeyValueAt(0.75, check_pos);
    }
    group->addAnimation(goback);

    if (!z_fixed){
        QPropertyAnimation *rotation = new QPropertyAnimation(this, "rotation");
        rotation->setEndValue(rotate_angle);
        rotation->setEasingCurve(QEasingCurve::OutQuad);
        rotation->setDuration(duration);
        group->addAnimation(rotation);
    }

    QPropertyAnimation *scale = new QPropertyAnimation(this, "scale");
    scale->setEndValue(1.0);
    if (slap != 0) {
        scale->setStartValue(slap);
        //scale->setKeyValueAt(0.5, slap);
        scale->setEasingCurve(QEasingCurve::InBack);
        scale->setDuration(duration);
    } else {
        scale->setEasingCurve(QEasingCurve::OutQuad);
        scale->setDuration(duration/2);
    }
    group->addAnimation(scale);

    if (x_rotate != 0 && !x_fixed) {
        QPropertyAnimation *anim_x = new QPropertyAnimation(this, "X_rotate");

        //anim_x->setKeyValueAt(0.5, 90);
        anim_x->setStartValue(x_rotate);
        anim_x->setEndValue(0);
        anim_x->setDuration(duration-0.75);

        anim_x->setEasingCurve(QEasingCurve::OutQuad);
        group->addAnimation(anim_x);
    }

    if (y_rotate != 0 && !y_fixed) {
        QPropertyAnimation *anim_y = new QPropertyAnimation(this, "Y_rotate");

        //anim_y->setKeyValueAt(0.5, 90);
        anim_y->setStartValue(y_rotate);
        anim_y->setEndValue(0);
        anim_y->setDuration(duration-0.75);

        anim_y->setEasingCurve(QEasingCurve::OutQuad);
        group->addAnimation(anim_y);
    }

    if (doFade) {
        QPropertyAnimation *disappear = new QPropertyAnimation(this, "opacity");
        double middleOpacity = qMax(opacity(), m_opacityAtHome);
        if (middleOpacity == 0) middleOpacity = 1.0;
        disappear->setEndValue(m_opacityAtHome);
        if (!smoothTransition) {
            disappear->setKeyValueAt(0.2, middleOpacity);
            disappear->setKeyValueAt(0.8, middleOpacity);
            disappear->setDuration(duration);
        }

        group->addAnimation(disappear);
    }

    /*if (slap>0) {
        slap = 0;
        QPropertyAnimation *slap_anim = new QPropertyAnimation(this, "scale");
        slap_anim->setEndValue(1.1);
        //if (!smoothTransition) {
            //slap_anim->setKeyValueAt(0.2, 1.1);
            //slap_anim->setKeyValueAt(0.8, 1.1);
            slap_anim->setDuration(duration);
            slap_anim->setEasingCurve(QEasingCurve::OutQuad);
        //}
        group->addAnimation(slap_anim);
    }*/

    m_currentAnimation = group;

    m_animationMutex.unlock();
    connect(m_currentAnimation, SIGNAL(finished()), this, SIGNAL(movement_animation_finished()));
    connect(m_currentAnimation, SIGNAL(destroyed()), this, SLOT(currentAnimationDestroyed()));
    return m_currentAnimation;
}

void CardItem::currentAnimationDestroyed()
{
    QObject *ca = sender();
    if (m_currentAnimation == ca)
        m_currentAnimation = NULL;
}

void CardItem::showAvatar(const General *general)
{
    _m_avatarName = general->objectName();
}

void CardItem::hideAvatar()
{
    _m_avatarName = QString();
}

void CardItem::showSuitNumber()
{
    _m_showSuitNumber = true;
}

void CardItem::hideSuitNumber()
{
    _m_showSuitNumber = false;
}

void CardItem::setShiny(bool shiny)
{
    m_isShiny = shiny;
}

void CardItem::setNumber(int number)
{
    m_number = number;
}

void CardItem::setColor(Card::Color color)
{
    m_color = color;
}

void CardItem::setSuit(Card::Suit suit)
{
    m_suit = suit;
    if (m_suit == Card::Heart || m_suit == Card::Diamond)
        m_color = Card::Red;
    else if (m_suit == Card::Spade || m_suit == Card::Club)
        m_color = Card::Black;
    else
        m_color = Card::Colorless;
}

void CardItem::setAutoBack(bool auto_back)
{
    this->auto_back = auto_back;
}

bool CardItem::isEquipped() const
{
    const Card *card = getCard();
    Q_ASSERT(card);
    return Self->hasEquip(card);
}

void CardItem::setFrozen(bool is_frozen)
{
    frozen = is_frozen;
}

CardItem *CardItem::FindItem(const QList<CardItem *> &items, int card_id)
{
    foreach (CardItem *item, items) {
        if (item->getCard() == NULL) {
            if (card_id == Card::S_UNKNOWN_CARD_ID)
                return item;
            else
                continue;
        }
        if (item->getCard()->getId() == card_id)
            return item;
    }

    return NULL;
}

const int CardItem::_S_CLICK_JITTER_TOLERANCE = 1600;
const int CardItem::_S_MOVE_JITTER_TOLERANCE = 200;

void CardItem::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (frozen) return;
    //if (!G_COMMON_LAYOUT.m_cardFrameArea.contains(mouseEvent->pos().x(), mouseEvent->pos().y())) return;
    _m_lastMousePressScenePos = mapToParent(mouseEvent->pos());
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (frozen) return;
    //if (!G_COMMON_LAYOUT.m_cardFrameArea.contains(mouseEvent->pos().x(), mouseEvent->pos().y())) return;

    QPointF totalMove = mapToParent(mouseEvent->pos()) - _m_lastMousePressScenePos;
    if (totalMove.x() * totalMove.x() + totalMove.y() * totalMove.y() < _S_MOVE_JITTER_TOLERANCE)
        emit clicked();
    else
        emit released();

    if (auto_back) {
        goBack(true, false);
    }
}

void CardItem::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    //if (!G_COMMON_LAYOUT.m_cardFrameArea.contains(mouseEvent->pos().x(), mouseEvent->pos().y())) return;

    if (!(flags() & QGraphicsItem::ItemIsMovable)) return;
    QPointF newPos = mapToParent(mouseEvent->pos());
    QPointF totalMove = newPos - _m_lastMousePressScenePos;
    if (totalMove.x() * totalMove.x() + totalMove.y() * totalMove.y() >= _S_CLICK_JITTER_TOLERANCE) {
        QPointF down_pos = mouseEvent->buttonDownPos(Qt::LeftButton);
        setPos(newPos - this->transform().map(down_pos));
    }
}

void CardItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (frozen) return;
    //if (!G_COMMON_LAYOUT.m_cardFrameArea.contains(event->pos().x(), event->pos().y())) return;

    if (hasFocus()) {
        event->accept();
        emit double_clicked();
    } else
        emit toggle_discards();
}

void CardItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    //if (!G_COMMON_LAYOUT.m_cardFrameArea.contains(event->pos().x(), event->pos().y())) return;
    emit enter_hover();
}

void CardItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //if (!G_COMMON_LAYOUT.m_cardFrameArea.contains(event->pos().x(), event->pos().y())) return;
    emit leave_hover();
}

/*void CardItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    bool now_in_boundary = G_COMMON_LAYOUT.m_cardFrameArea.contains(event->pos().x(), event->pos().y());
    bool last_in_boundary = G_COMMON_LAYOUT.m_cardFrameArea.contains(event->lastPos().x(), event->lastPos().y());
    if (now_in_boundary && !last_in_boundary)
        emit enter_hover();
    else if (!now_in_boundary && last_in_boundary)
        emit leave_hover();
}*/


void CardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (!isEnabled()) {
        painter->fillRect(G_COMMON_LAYOUT.m_cardMainArea, QColor(100, 100, 100, 255 * opacity()));
        painter->setOpacity(0.7 * opacity());
    }

    if (!_m_isUnknownGeneral)
        painter->drawPixmap(G_COMMON_LAYOUT.m_cardMainArea, G_ROOM_SKIN.getCardMainPixmap(objectName(), true));
    else
        painter->drawPixmap(G_COMMON_LAYOUT.m_cardMainArea, G_ROOM_SKIN.getPixmap("generalCardBack", QString(), true));
    const Card *card = Sanguosha->getEngineCard(m_cardId);
    if (card) {
        if (_m_showSuitNumber && m_suit != Card::NoSuit && m_number > 0 && m_number <= 13 && (m_color == Card::Black || m_color == Card::Red)) {
            painter->drawPixmap(G_COMMON_LAYOUT.m_cardSuitArea, G_ROOM_SKIN.getCardSuitPixmap(m_suit));
            painter->drawPixmap(G_COMMON_LAYOUT.m_cardNumberArea, G_ROOM_SKIN.getCardNumberPixmap(m_number, (m_color == Card::Black)));
        }
        QRect rect = G_COMMON_LAYOUT.m_cardFootnoteArea;
        // Deal with stupid QT...
        if (_m_showFootnote) painter->drawImage(rect, _m_footnoteImage);
    }

    if (!_m_avatarName.isEmpty())
        painter->drawPixmap(G_COMMON_LAYOUT.m_cardAvatarArea, G_ROOM_SKIN.getCardAvatarPixmap(_m_avatarName));

    if (!_m_isUnknownGeneral && _m_showSuitNumber && m_isMine) {    //only show mine pic when the card is a visible real-card
        painter->drawPixmap(QRect(0,-30,93,130), G_ROOM_SKIN.getCardMainPixmap("mine_card", true));
    }

    if (m_isShiny) {
        QBrush painter_brush(QColor(255, 215, 0, 64));
        painter->setBrush(painter_brush);
        painter->drawRect(G_COMMON_LAYOUT.m_cardMainArea);
    }
}

void CardItem::setFootnote(const QString &desc)
{
    const IQSanComponentSkin::QSanShadowTextFont &font = G_COMMON_LAYOUT.m_cardFootnoteFont;
    QRect rect = G_COMMON_LAYOUT.m_cardFootnoteArea;
    rect.moveTopLeft(QPoint(0, 0));
    _m_footnoteImage = QImage(rect.size(), QImage::Format_ARGB32);
    _m_footnoteImage.fill(Qt::transparent);
    QPainter painter(&_m_footnoteImage);
    font.paintText(&painter, QRect(QPoint(0, 0), rect.size()),
        (Qt::AlignmentFlag)((int)Qt::AlignHCenter | Qt::AlignBottom | Qt::TextWrapAnywhere), desc);
}


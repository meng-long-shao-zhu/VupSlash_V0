#include "magatamas-item.h"
#include "skin-bank.h"
#include "sprite.h"

MagatamasBoxItem::MagatamasBoxItem()
    : QGraphicsObject(NULL)
{
    m_hp = 0;
    m_maxHp = 0;
}

MagatamasBoxItem::MagatamasBoxItem(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    m_hp = 0;
    m_maxHp = 0;
}

void MagatamasBoxItem::setOrientation(Qt::Orientation orientation)
{
    m_orientation = orientation;
    _updateLayout();
}

void MagatamasBoxItem::_updateLayout()
{
    int xStep, yStep;
    if (this->m_orientation == Qt::Horizontal) {
        xStep = m_iconSize.width();
        yStep = 0;
    } else {
        xStep = 0;
        yStep = m_iconSize.height();
    }

    for (int i = 0; i <= 6; i++) {
        _icons[i] = G_ROOM_SKIN.getPixmap(QString(QSanRoomSkin::S_SKIN_KEY_MAGATAMAS).arg(QString::number(i)), QString(), true)
            .scaled(m_iconSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    for (int i = 1; i <= 5; i++) {
        QSize bgSize;
        if (this->m_orientation == Qt::Horizontal) {
            bgSize.setWidth((xStep + 1) * i);
            bgSize.setHeight(m_iconSize.height());
        } else {
            bgSize.setWidth((yStep + 1) * i);
            bgSize.setHeight(m_iconSize.width());
        }
        _bgImages[i] = G_ROOM_SKIN.getPixmap(QString(QSanRoomSkin::S_SKIN_KEY_MAGATAMAS_BG).arg(QString::number(i)))
            .scaled(bgSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
}

void MagatamasBoxItem::setIconSize(QSize size)
{
    m_iconSize = size;
    _updateLayout();
}

QRectF MagatamasBoxItem::boundingRect() const
{
    int buckets = qMin(m_maxHp, 5) + G_COMMON_LAYOUT.m_hpExtraSpaceHolder;
    if (m_hp > m_maxHp) buckets = 5 + G_COMMON_LAYOUT.m_hpExtraSpaceHolder;
    if (m_orientation == Qt::Horizontal)
        return QRectF(0, 0, buckets * m_iconSize.width(), m_iconSize.height());
    else
        return QRectF(0, 0, m_iconSize.width(), buckets * m_iconSize.height());
}

void MagatamasBoxItem::setHp(int hp)
{
    _doHpChangeAnimation(hp);
    m_hp = hp;
    update();
}

void MagatamasBoxItem::setAnchor(QPoint anchor, Qt::Alignment align)
{
    m_anchor = anchor;
    m_align = align;
}

void MagatamasBoxItem::setMaxHp(int maxHp)
{
    m_maxHp = maxHp;
    _autoAdjustPos();
}

void MagatamasBoxItem::_autoAdjustPos()
{
    if (!anchorEnabled) return;
    QRectF rect = boundingRect();
    Qt::Alignment hAlign = m_align & Qt::AlignHorizontal_Mask;
    if (hAlign == Qt::AlignRight)
        setX(m_anchor.x() - rect.width());
    else if (hAlign == Qt::AlignHCenter)
        setX(m_anchor.x() - rect.width() / 2);
    else
        setX(m_anchor.x());
    Qt::Alignment vAlign = m_align & Qt::AlignVertical_Mask;
    if (vAlign == Qt::AlignBottom)
        setY(m_anchor.y() - rect.height());
    else if (vAlign == Qt::AlignVCenter)
        setY(m_anchor.y() - rect.height() / 2);
    else
        setY(m_anchor.y());
}

void MagatamasBoxItem::update()
{
    _updateLayout();
    _autoAdjustPos();
    QGraphicsItem::update();
}

void MagatamasBoxItem::_doHpChangeAnimation(int newHp)
{
    if (newHp >= m_hp) return;

    int width = m_imageArea.width();
    int height = m_imageArea.height();
    int xStep, yStep;
    if (this->m_orientation == Qt::Horizontal) {
        xStep = width;
        yStep = 0;
    } else {
        xStep = 0;
        yStep = height;
    }

    int mHp = m_hp;
    if (m_hp < 0) {
        newHp -= m_hp;
        mHp = 0;
    }
    for (int i = qMax(newHp, mHp - 10); i < mHp; i++) {
        Sprite *aniMaga = new Sprite;
        int imageIndex = qBound(0, i, 5);
        if (i >= m_maxHp) imageIndex = 6;
        aniMaga->setPixmap(_icons[imageIndex]);
        aniMaga->setParentItem(this);
        aniMaga->setOffset(QPoint(-(width - m_imageArea.left()) / 2, -(height - m_imageArea.top()) / 2));

        int pos = (m_maxHp > 5)||(newHp > m_maxHp) ? 0 : i;
        aniMaga->setPos(QPoint(xStep * pos - aniMaga->offset().x(), yStep * pos - aniMaga->offset().y()));

        QPropertyAnimation *fade = new QPropertyAnimation(aniMaga, "opacity");
        fade->setEndValue(0);
        fade->setDuration(500);
        QPropertyAnimation *grow = new QPropertyAnimation(aniMaga, "scale");
        grow->setEndValue(4);
        grow->setDuration(500);

        connect(fade, SIGNAL(finished()), aniMaga, SLOT(deleteLater()));

        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        group->addAnimation(fade);
        group->addAnimation(grow);

        group->start(QAbstractAnimation::DeleteWhenStopped);

        aniMaga->show();
    }
}

void MagatamasBoxItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (m_maxHp <= 0 && m_hp <= 0) return;
    int imageIndex = qBound(0, m_hp, 5);
    int bgIndex = qMin(m_maxHp, 5);
    if (m_hp == m_maxHp) imageIndex = 5;
    if (m_hp > m_maxHp) {
        imageIndex = 6;
        bgIndex = 5;
    }

    int xStep, yStep;
    if (this->m_orientation == Qt::Horizontal) {
        xStep = m_iconSize.width();
        yStep = 0;
    } else {
        xStep = 0;
        yStep = m_iconSize.height();
    }

    if (m_showBackground) {
        if (this->m_orientation == Qt::Vertical) {
            painter->save();
            painter->translate(m_iconSize.width(), 0);
            painter->rotate(90);
        }
        painter->drawPixmap(0, 0, _bgImages[bgIndex]);
        if (this->m_orientation == Qt::Vertical)
            painter->restore();
    }

    if (m_maxHp <= 5 && m_hp <= m_maxHp) {
        int i;
        for (i = 0; i < m_hp; i++) {
            QRect rect(xStep * i, yStep * i, m_imageArea.width(), m_imageArea.height());
            rect.translate(m_imageArea.topLeft());
            painter->drawPixmap(rect, _icons[imageIndex]);
        }
        for (; i < m_maxHp; i++) {
            QRect rect(xStep * i, yStep * i, m_imageArea.width(), m_imageArea.height());
            rect.translate(m_imageArea.topLeft());
            painter->drawPixmap(rect, _icons[0]);
        }
    } else {
        painter->drawPixmap(m_imageArea, _icons[imageIndex]);
        QRect rect(xStep, yStep, m_imageArea.width() * 1.2, m_imageArea.height() * 1.2);
        QRect rect_wide(xStep - m_imageArea.width() * 0.4, yStep, m_imageArea.width() * 2, m_imageArea.height() * 1.2);
        rect.translate(m_imageArea.topLeft());
        rect_wide.translate(m_imageArea.topLeft());
        if (this->m_orientation == Qt::Horizontal) {
            rect.translate(xStep * 0.5 * 1.2, yStep * 0.5 * 1.2);
            rect_wide.translate(xStep * 0.5 * 1.2, yStep * 0.5 * 1.2);
        }

        QString hp = QString::number(m_hp);
        QString maxHp = QString::number(m_maxHp);

        G_COMMON_LAYOUT.m_hpFont[imageIndex].paintText(painter, (hp.length()>1)?rect_wide:rect, Qt::AlignCenter, hp);
        rect.translate(xStep * 1.2, yStep * 1.2);
        rect_wide.translate(xStep * 1.2, yStep * 1.2);
        G_COMMON_LAYOUT.m_hpFont[imageIndex].paintText(painter, rect, Qt::AlignCenter, "/");
        rect.translate(xStep * 1.2, yStep * 1.2);
        rect_wide.translate(xStep * 1.2, yStep * 1.2);
        G_COMMON_LAYOUT.m_hpFont[imageIndex].paintText(painter, (maxHp.length()>1)?rect_wide:rect, Qt::AlignCenter, maxHp);
    }
}


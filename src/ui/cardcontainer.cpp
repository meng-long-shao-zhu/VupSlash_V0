#include "cardcontainer.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"
#include "client.h"

CardContainer::CardContainer()
    : _m_background("image/system/card-container.png")
{
    setTransform(QTransform::fromTranslate(-_m_background.width() / 2, -_m_background.height() / 2), true);
    _m_boundingRect = QRectF(QPoint(0, 0), _m_background.size());
    setFlag(ItemIsFocusable);
    setFlag(ItemIsMovable);
    close_button = new CloseButton;
    close_button->setParentItem(this);
    //close_button->setPos(517, 21);
    close_button->setPos(710, 17);
    close_button->hide();
    connect(close_button, SIGNAL(clicked()), this, SLOT(clear()));

    log_button = new LogButton;
    log_button->setParentItem(this);
    log_button->setPos(30, 17);
    log_button->hide();
}

void CardContainer::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->drawPixmap(0, 0, _m_background);
}

QRectF CardContainer::boundingRect() const
{
    return _m_boundingRect;
}

void CardContainer::fillCards(const QList<int> &card_ids, const QList<int> &disabled_ids, bool hide_suit_number, const QString &foot_notes)
{
    QList<CardItem *> card_items;
    if (card_ids.isEmpty() && items.isEmpty())
        return;
    else if (card_ids.isEmpty() && !items.isEmpty()) {
        card_items = items;
        items.clear();
    } else if (!items.isEmpty()) {
        retained_stack.push(retained());
        items_stack.push(items);
        foreach(CardItem *item, items)
            item->hide();
        items.clear();
    }

    close_button->hide();
    log_button->hide();
    if (card_items.isEmpty())
        card_items = _createCards(card_ids);

    int card_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    QPointF pos1(30 + card_width / 2, 60 + G_COMMON_LAYOUT.m_cardNormalHeight / 2);
    QPointF pos2(30 + card_width / 2, 204 + G_COMMON_LAYOUT.m_cardNormalHeight / 2);
    int skip = 102;
    int column = 7;
    qreal whole_width = skip * (column-1);
    items.append(card_items);
    int n = items.length();

    QStringList footnote_list = foot_notes.split("|");

    for (int i = 0; i < n; i++) {
        QPointF pos;
        if (n <= 2*column) {
            if (i < column) {
                pos = pos1;
                pos.setX(pos.x() + i * skip);
            } else {
                pos = pos2;
                pos.setX(pos.x() + (i - column) * skip);
            }
        } else {
            int half = (n + 1) / 2;
            qreal real_skip = whole_width / (half - 1);

            if (i < half) {
                pos = pos1;
                pos.setX(pos.x() + i * real_skip);
            } else {
                pos = pos2;
                pos.setX(pos.x() + (i - half) * real_skip);
            }
        }
        CardItem *item = items[i];
        item->setPos(pos);
        item->setHomePos(pos);
        item->setOpacity(1.0);
        item->setHomeOpacity(1.0);
        item->setFlag(QGraphicsItem::ItemIsFocusable);
        if (hide_suit_number) item->hideSuitNumber();
        if (footnote_list.size() > i) {
            QStringList footnote_parts = footnote_list.at(i).split("+");
            QString footnote_string = "";
            foreach(QString footnote_part, footnote_parts) {
                footnote_string += Sanguosha->translate(footnote_part);
            }
            item->setFootnote(footnote_string);
            item->showFootnote();
        }
        if (disabled_ids.contains(item->getCard()->getEffectiveId())) item->setEnabled(false);
        item->show();
    }
}

bool CardContainer::_addCardItems(QList<CardItem *> &, const CardsMoveStruct &)
{
    return true;
}

bool CardContainer::retained()
{
    return close_button != NULL && close_button->isVisible();
}

void CardContainer::clear()
{
    foreach (CardItem *item, items) {
        item->hide();
        item = NULL;
        delete item;
    }

    items.clear();
    if (!items_stack.isEmpty()) {
        items = items_stack.pop();
        bool retained = retained_stack.pop();
        fillCards();
        if (retained && close_button)
            close_button->show();
    } else {
        close_button->hide();
        log_button->hide();
        hide();
    }
}

void CardContainer::freezeCards(bool is_frozen)
{
    foreach(CardItem *item, items)
        item->setFrozen(is_frozen);
}

QList<CardItem *> CardContainer::removeCardItems(const QList<int> &card_ids, Player::Place)
{
    QList<CardItem *> result;
    foreach (int card_id, card_ids) {
        CardItem *to_take = NULL;
        foreach (CardItem *item, items) {
            if (item->getCard()->getId() == card_id) {
                to_take = item;
                break;
            }
        }
        if (to_take == NULL) continue;

        to_take->setEnabled(false);

        CardItem *copy = new CardItem(to_take->getCard());
        copy->setPos(mapToScene(to_take->pos()));
        copy->setEnabled(false);
        result.append(copy);

        if (m_currentPlayer) {
            to_take->showAvatar(m_currentPlayer->getGeneral());
            to_take->setFootnote(Sanguosha->translate(m_currentPlayer->getGeneralName()));
            to_take->showFootnote();
        }
    }
    return result;
}

int CardContainer::getFirstEnabled() const
{
    foreach (CardItem *card, items) {
        if (card->isEnabled())
            return card->getCard()->getId();
    }
    return -1;
}

void CardContainer::startChoose()
{
    close_button->hide();
    foreach (CardItem *item, items) {
        connect(item, SIGNAL(leave_hover()), this, SLOT(grabItem()));
        connect(item, SIGNAL(double_clicked()), this, SLOT(chooseItem()));
    }
}

void CardContainer::startGongxin(const QList<int> &enabled_ids)
{
    if (enabled_ids.isEmpty()) return;
    foreach (CardItem *item, items) {
        const Card *card = item->getCard();
        if (card && enabled_ids.contains(card->getEffectiveId()))
            connect(item, SIGNAL(double_clicked()), this, SLOT(gongxinItem()));
        else
            item->setEnabled(false);
    }
}

void CardContainer::addCloseButton()
{
    close_button->show();
}

void CardContainer::setLogButton(QString reason)
{
    if (reason == "" || Sanguosha->translate("^AG_"+reason) == "^AG_"+reason) {
        log_button->hide();
    } else {
        log_button->show();
        //if (Sanguosha->translate("^AG_"+reason) == "^AG_"+reason)
        //    reason = "default";
        log_button->setToolTip(Sanguosha->translate("^AG_"+reason));
    }
}

void CardContainer::grabItem()
{
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if (card_item && !collidesWithItem(card_item)) {
        card_item->disconnect(this);
        emit item_chosen(card_item->getCard()->getId());
    }
}

void CardContainer::chooseItem()
{
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if (card_item) {
        card_item->disconnect(this);
        emit item_chosen(card_item->getCard()->getId());
    }
}

void CardContainer::gongxinItem()
{
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if (card_item) {
        emit item_gongxined(card_item->getCard()->getId());
        clear();
    }
}

CloseButton::CloseButton()
    : QSanSelectableItem("image/system/close.png", false)
{
    setFlag(ItemIsFocusable);
    setAcceptedMouseButtons(Qt::LeftButton);
}

void CloseButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void CloseButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    emit clicked();
}

LogButton::LogButton()
    : QSanSelectableItem("image/system/log_button.png", false)
{
    setFlag(ItemIsFocusable);
    setAcceptedMouseButtons(Qt::LeftButton);
}

void LogButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void LogButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    QToolTip::showText(QCursor::pos(), this->toolTip());
    emit clicked();
}

void CardContainer::view(const ClientPlayer *player)
{
    QList<int> card_ids;
    QList<const Card *> cards = player->getHandcards();
    foreach(const Card *card, cards)
        card_ids << card->getEffectiveId();

    fillCards(card_ids);
}

GuanxingBox::GuanxingBox()
    : QSanSelectableItem("image/system/guanxing-box.png", true)
{
    setFlag(ItemIsFocusable);
    setFlag(ItemIsMovable);
}

void GuanxingBox::doGuanxing(const QList<int> &card_ids, int up_limit, int down_limit)
{
    if (card_ids.isEmpty()) {
        clear();
        return;
    }

    this->up_limit = up_limit;
    this->down_limit = down_limit;
    up_items.clear();
    down_items.clear();

    foreach (int card_id, card_ids) {
        CardItem *card_item = new CardItem(Sanguosha->getCard(card_id));
        card_item->setAutoBack(false);
        card_item->setFlag(QGraphicsItem::ItemIsFocusable);
        connect(card_item, SIGNAL(released()), this, SLOT(adjust()));
        if (up_items.length() < up_limit)
            up_items << card_item;
        else if (down_items.length() < down_limit)
            down_items << card_item;
        else
            down_items << card_item;
        card_item->setParentItem(this);
    }

    show();

    QPointF source(start_x, start_y1);
    for (int i = 0; i < up_items.length(); i++) {
        CardItem *card_item = up_items.at(i);
        QPointF pos(start_x + i * skip, start_y1);
        card_item->setPos(source);
        card_item->setHomePos(pos);
        card_item->goBack(true);
    }

    QPointF source2(start_x, start_y2);
    for (int i = 0; i < down_items.length(); i++) {
        CardItem *card_item = down_items.at(i);
        QPointF pos(start_x + i * skip, start_y2);
        card_item->setPos(source2);
        card_item->setHomePos(pos);
        card_item->goBack(true);
    }
}

void GuanxingBox::adjust()
{
    CardItem *item = qobject_cast<CardItem *>(sender());
    if (item == NULL) return;

    up_items.removeOne(item);
    down_items.removeOne(item);

    //if (up_limit + down_limit == up_items.length() + down_items.length() + 1) {     //it's full so at least you need to swap
        QList<CardItem *> *items = (item->y() > middle_y) ? &down_items : &up_items;
        int c = (item->x() + item->boundingRect().width() / 2 - start_x) / G_COMMON_LAYOUT.m_cardNormalWidth;
        c = qBound(0, c, items->length());
        items->insert(c, item);

        if (up_items.length() > up_limit) {
            CardItem *remove_item = up_items.last();
            up_items.removeLast();
            QList<CardItem *> *to_items = &down_items;
            to_items->insert(to_items->length(), remove_item);
        } else if (down_items.length() > down_limit) {
            CardItem *remove_item = down_items.last();
            down_items.removeLast();
            QList<CardItem *> *to_items = &up_items;
            to_items->insert(to_items->length(), remove_item);
        }
    /*} else {
        QList<CardItem *> *items = ((up_items.length() >= up_limit) || ((item->y() > middle_y) && (down_items.length() < down_limit))) ? &down_items : &up_items;
        int c = (item->x() + item->boundingRect().width() / 2 - start_x) / G_COMMON_LAYOUT.m_cardNormalWidth;
        c = qBound(0, c, items->length());
        items->insert(c, item);
    }*/

    for (int i = 0; i < up_items.length(); i++) {
        QPointF pos(start_x + i * skip, start_y1);
        up_items.at(i)->setHomePos(pos);
        up_items.at(i)->goBack(true);
    }

    for (int i = 0; i < down_items.length(); i++) {
        QPointF pos(start_x + i * skip, start_y2);
        down_items.at(i)->setHomePos(pos);
        down_items.at(i)->goBack(true);
    }
}

void GuanxingBox::clear()
{
    foreach(CardItem *card_item, up_items)
        card_item->deleteLater();
    foreach(CardItem *card_item, down_items)
        card_item->deleteLater();

    up_items.clear();
    down_items.clear();

    hide();
}

void GuanxingBox::reply()
{
    QList<int> up_cards, down_cards;
    foreach(CardItem *card_item, up_items)
        up_cards << card_item->getCard()->getId();

    foreach(CardItem *card_item, down_items)
        down_cards << card_item->getCard()->getId();

    ClientInstance->onPlayerReplyGuanxing(up_cards, down_cards);
    clear();
}


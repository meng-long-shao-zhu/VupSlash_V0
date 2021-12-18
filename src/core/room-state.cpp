#include "room-state.h"
#include "engine.h"
#include "wrapped-card.h"

RoomState::~RoomState()
{
    foreach(Card *card, m_cards.values())
        delete card;
    m_cards.clear();
}

Card *RoomState::getCard(int cardId) const
{
    if (!m_cards.contains(cardId))
        return NULL;
    return m_cards[cardId];
}

void RoomState::resetCard(int cardId)
{
    Card *newCard = Card::Clone(Sanguosha->getEngineCard(cardId));
    if (newCard == NULL) return;
    if (m_cards[cardId] == NULL) {
        m_cards[cardId] = new WrappedCard(newCard);
    } else {
        newCard->setFlags(m_cards[cardId]->getFlags());
        newCard->tag = m_cards[cardId]->tag;
    }
    m_cards[cardId]->copyEverythingFrom(newCard);
    newCard->clearFlags();
    newCard->tag.clear();
    newCard->setOvert(false);
    m_cards[cardId]->setModified(false);
}

// Reset all cards, generals' states of the room instance
void RoomState::reset()
{
    foreach(WrappedCard *card, m_cards.values())
        delete card;
    m_cards.clear();

    int n = Sanguosha->getCardCount();
    for (int i = 0; i < n; i++) {
        const Card *card = Sanguosha->getEngineCard(i);
        Card *clonedCard = Card::Clone(card);
        m_cards[i] = new WrappedCard(Card::Clone(clonedCard));
        delete clonedCard;
    }
}


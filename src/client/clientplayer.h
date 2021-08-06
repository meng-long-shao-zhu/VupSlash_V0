#ifndef _CLIENT_PLAYER_H
#define _CLIENT_PLAYER_H

#include "player.h"
//#include "clientstruct.h"

class Client;
class QTextDocument;

class ClientPlayer : public Player
{
    Q_OBJECT
    Q_PROPERTY(int handcard READ getHandcardNum WRITE setHandcardNum)

public:
    explicit ClientPlayer(Client *client);
    QList<const Card *> getHandcards() const;
    QList<const Card *> getPublicCards() const;
    void setCards(const QList<int> &card_ids);
    void setPublicCards(const QList<int> &card_ids);
    QTextDocument *getMarkDoc() const;
    void changePile(const QString &name, bool add, QList<int> card_ids);
    void refreshPile(const QString &name);
    QString getDeathPixmapPath() const;
    void setHandcardNum(int n);
    QString getGameMode() const;

    void setFlags(const QString &flag);
    int aliveCount() const;
    int getHandcardNum() const;
    //int getOvertcardNum() const;
    void removeCard(const Card *card, Place place);
    void addCard(const Card *card, Place place);
    void addKnownHandCard(const Card *card);
    bool isLastHandCard(const Card *card, bool contain = false) const;
    void setMark(const QString &mark, int value);

private:
    int handcard_num;
    QList<const Card *> known_cards;
    QList<const Card *> public_cards;
    QTextDocument *mark_doc;

signals:
    void pile_changed(const QString &name);
    void drank_changed();
    void action_taken();
    void duanchang_invoked();
    void Mark_changed(const QString &name, bool get);
};

extern ClientPlayer *Self;

#endif


#include "mobilezhi.h"
#include "settings.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "clientplayer.h"
#include "engine.h"
#include "maneuvering.h"
#include "util.h"
#include "wrapped-card.h"
#include "room.h"
#include "roomthread.h"

MobileZhiQiaiCard::MobileZhiQiaiCard()
{
    will_throw = false;
    handling_method = Card::MethodNone;
}

void MobileZhiQiaiCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.from->getRoom();
    room->giveCard(effect.from, effect.to, this, "mobilezhiqiai");
    if (effect.from->isDead() || effect.to->isDead()) return;
    QStringList choices;
    if (effect.from->getLostHp() > 0)
        choices << "recover";
    choices << "draw";
    if (room->askForChoice(effect.to, "mobilezhiqiai", choices.join("+"), QVariant::fromValue(effect.from)) == "recover")
        room->recover(effect.from, RecoverStruct(effect.to));
    else
        effect.from->drawCards(2, "mobilezhiqiai");
}

class MobileZhiQiai : public OneCardViewAsSkill
{
public:
    MobileZhiQiai() : OneCardViewAsSkill("mobilezhiqiai")
    {
        filter_pattern = "^BasicCard";
    }

    const Card *viewAs(const Card *card) const
    {
        MobileZhiQiaiCard *c = new MobileZhiQiaiCard;
        c->addSubcard(card);
        return c;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("MobileZhiQiaiCard");
    }
};

class MobileZhiShanxi : public TriggerSkill
{
public:
    MobileZhiShanxi() : TriggerSkill("mobilezhishanxi")
    {
        events << EventPhaseStart << HpRecover;
    }

    bool transferMark(ServerPlayer *to, Room *room) const
    {
        int n = 0;
        foreach (ServerPlayer *p, room->getOtherPlayers(to)) {
            if (to->isDead()) break;
            if (p->isAlive() && p->getMark("&mobilezhixi") > 0) {
                n++;
                int mark = p->getMark("&mobilezhixi");
                p->loseAllMarks("&mobilezhixi");
                to->gainMark("&mobilezhixi", mark);
            }
        }
        return n > 0;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &) const
    {
        if (event == EventPhaseStart) {
            if (player->getPhase() != Player::Play || !player->hasSkill(this)) return false;
            QList<ServerPlayer *> targets;
            foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                if (p->getMark("&mobilezhixi") > 0) continue;
                targets << p;
            }
            if (targets.isEmpty()) return false;
            ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName(), "@mobilezhishanxi-invoke", true, true);
            if (!target) return false;
            room->broadcastSkillInvoke(objectName());
            if (transferMark(target, room)) return false;
            target->gainMark("&mobilezhixi");
        } else {
            if (player->getMark("&mobilezhixi") <= 0 || player->hasFlag("Global_Dying")) return false;
            foreach (ServerPlayer *p, room->getAllPlayers()) {
                if (player->isDead()) return false;
                if (p->isDead() || !p->hasSkill(this)) continue;
                room->sendCompulsoryTriggerLog(p, objectName(), true, true);
                if (p == player || player->getCardCount() < 2)
                    room->loseHp(player);
                else {
                    const Card *card = room->askForExchange(player, objectName(), 2, 2, true, "mobilezhishanxi-give:" + p->objectName(), true);
                    if (!card)
                        room->loseHp(player);
                    else
                        room->giveCard(player, p, card, objectName());
                }
            }

        }
        return false;
    }
};

MobileZhiShamengCard::MobileZhiShamengCard()
{
}

void MobileZhiShamengCard::onEffect(const CardEffectStruct &effect) const
{
    effect.to->drawCards(2, "mobilezhishameng");
    effect.from->drawCards(3, "mobilezhishameng");
}

class MobileZhiShameng : public ViewAsSkill
{
public:
    MobileZhiShameng() : ViewAsSkill("mobilezhishameng")
    {
    }

    bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        if (to_select->isEquipped() || Self->isJilei(to_select) || selected.length() > 1) return false;
        if (selected.isEmpty()) return true;
        if (selected.length() == 1)
            return to_select->sameColorWith(selected.first());
        return false;
    }

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length() != 2) return NULL;
        MobileZhiShamengCard *c = new MobileZhiShamengCard;
        c->addSubcards(cards);
        return c;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("MobileZhiShamengCard");
    }
};

class MobileZhiFubi : public GameStartSkill
{
public:
    MobileZhiFubi() : GameStartSkill("mobilezhifubi")
    {
    }

    void onGameStart(ServerPlayer *player) const
    {
        Room *room = player->getRoom();
       ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName(), "@mobilezhifubi-invoke", true, true);
       if (!target) return;
       room->broadcastSkillInvoke(objectName());
       target->gainMark("&mobilezhifu");
    }
};

class MobileZhiFubiKeep : public MaxCardsSkill
{
public:
    MobileZhiFubiKeep() : MaxCardsSkill("#mobilezhifubi")
    {
        frequency = NotFrequent;
    }

    int getExtra(const Player *target) const
    {
        if (target->getMark("&mobilezhifu") > 0) {
            int sunshao = 0;
            foreach (const Player *p, target->getAliveSiblings()) {
                if (p->hasSkill("mobilezhifubi"))
                    sunshao++;
            }
            return 3 * sunshao;
        }
        return 0;
    }
};

class MobileZhiZuici : public TriggerSkill
{
public:
    MobileZhiZuici() : TriggerSkill("mobilezhizuici")
    {
        events << Dying;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.who != player) return false;
        QStringList areas;
        for (int i = 0; i < 5; i++) {
            if (player->getEquip(i) && player->hasEquipArea(i))
                areas << QString::number(i);
        }
        if (areas.isEmpty()) return false;
        if (!player->askForSkillInvoke(this)) return false;
        room->broadcastSkillInvoke(objectName());
        QString area = room->askForChoice(player, objectName(), areas.join("+"));
        player->throwEquipArea(area.toInt());
        room->recover(player, RecoverStruct(player, NULL, 1 - player->getHp()));
        return false;
    }
};

MobileZhiDuojiCard::MobileZhiDuojiCard()
{
}

bool MobileZhiDuojiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && to_select != Self && !to_select->getEquips().isEmpty();
}

void MobileZhiDuojiCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.from->getRoom();
    room->removePlayerMark(effect.from, "@mobilezhiduojiMark");
    room->doSuperLightbox("mobilezhi_xunchen", "mobilezhiduoji");
    QList<int> equiplist = effect.to->getEquipsId();
    if (equiplist.isEmpty()) return;
    DummyCard equips(equiplist);
    room->obtainCard(effect.from, &equips);
}

class MobileZhiDuoji : public ViewAsSkill
{
public:
    MobileZhiDuoji() : ViewAsSkill("mobilezhiduoji")
    {
        frequency = Limited;
        limit_mark = "@mobilezhiduojiMark";
    }

    bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        return selected.length() < 2 && !to_select->isEquipped() && !Self->isJilei(to_select);
    }

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length() != 2) return NULL;
        MobileZhiDuojiCard *c = new MobileZhiDuojiCard;
        c->addSubcards(cards);
        return c;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return player->getMark("@mobilezhiduojiMark") > 0;
    }
};

MobileZhiJianzhanCard::MobileZhiJianzhanCard()
{
}

void MobileZhiJianzhanCard::onEffect(const CardEffectStruct &effect) const
{
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->deleteLater();
    slash->setSkillName("_mobilezhijianzhan");

    Room *room = effect.from->getRoom();
    QStringList choices;
    QList<ServerPlayer *> can_slash;
    foreach (ServerPlayer *p, room->getOtherPlayers(effect.to)) {
        if (!effect.to->canSlash(p, slash) || p->getHandcardNum() >= effect.to->getHandcardNum()) continue;
        can_slash << p;
    }
    if (!can_slash.isEmpty())
        choices << "slash";
    choices << "draw";

    QString choice = room->askForChoice(effect.to, "mobilezhijianzhan", choices.join("+"), QVariant::fromValue(effect.from));
    if (choice == "slash") {
        foreach (ServerPlayer *p, can_slash) {
            if (!effect.to->canSlash(p, slash) || p->getHandcardNum() >= effect.to->getHandcardNum())
                can_slash.removeOne(p);
        }
        if (can_slash.isEmpty()) return;
        ServerPlayer *to = room->askForPlayerChosen(effect.from, can_slash, "mobilezhijianzhan", "@mobilezhijianzhan-slash:" + effect.to->objectName());
        room->useCard(CardUseStruct(slash, effect.to, to));
    } else
        effect.from->drawCards(1, "mobilezhijianzhan");
}

class MobileZhiJianzhan : public ZeroCardViewAsSkill
{
public:
    MobileZhiJianzhan() : ZeroCardViewAsSkill("mobilezhijianzhan")
    {
    }

    const Card *viewAs() const
    {
        return new MobileZhiJianzhanCard;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("MobileZhiJianzhanCard");
    }
};

MobileZhiPackage::MobileZhiPackage()
    : Package("mobilezhi")
{
    General *mobilezhi_wangcan = new General(this, "mobilezhi_wangcan", "wei", 3);
    mobilezhi_wangcan->addSkill(new MobileZhiQiai);
    mobilezhi_wangcan->addSkill(new MobileZhiShanxi);

    General *mobilezhi_chenzhen = new General(this, "mobilezhi_chenzhen", "shu", 3);
    mobilezhi_chenzhen->addSkill(new MobileZhiShameng);

    General *mobilezhi_sunshao = new General(this, "mobilezhi_sunshao", "wu", 3);
    mobilezhi_sunshao->addSkill(new MobileZhiFubi);
    mobilezhi_sunshao->addSkill(new MobileZhiFubiKeep);
    mobilezhi_sunshao->addSkill(new MobileZhiZuici);
    related_skills.insertMulti("mobilezhifubi", "#mobilezhifubi");

    General *mobilezhi_xunchen = new General(this, "mobilezhi_xunchen", "qun", 3);
    mobilezhi_xunchen->addSkill(new MobileZhiDuoji);
    mobilezhi_xunchen->addSkill(new MobileZhiJianzhan);

    addMetaObject<MobileZhiQiaiCard>();
    addMetaObject<MobileZhiShamengCard>();
    addMetaObject<MobileZhiDuojiCard>();
    addMetaObject<MobileZhiJianzhanCard>();
}

ADD_PACKAGE(MobileZhi)

#include "sp3.h"
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
#include "yjcm2013.h"
#include "wind.h"

class TenyearMeibu : public PhaseChangeSkill
{
public:
    TenyearMeibu() : PhaseChangeSkill("tenyearmeibu")
    {
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive() && target->getPhase() == Player::Play;
    }

    bool onPhaseChange(ServerPlayer *player) const
    {
        Room *room = player->getRoom();
        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            if (player->isDead()) return false;
            if (p->isDead() || !p->hasSkill(this) || !player->inMyAttackRange(p)) continue;
            if (!p->canDiscard(p, "he") || !room->askForCard(p, "..", "@tenyearmeibu-dis:" + player->objectName(), QVariant::fromValue(player), objectName())) continue;
            room->broadcastSkillInvoke(objectName());
            room->acquireOneTurnSkills(player, QString(), "tenyearzhixi");
        }
        return false;
    }
};

class TenyearMumu : public PhaseChangeSkill
{
public:
    TenyearMumu() : PhaseChangeSkill("tenyearmumu")
    {
    }

    bool onPhaseChange(ServerPlayer *player) const
    {
        if (player->getPhase() != Player::Play) return false;
        Room *room = player->getRoom();
        QList<ServerPlayer *> targets, targets2;
        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            if (p != player && !p->getEquips().isEmpty() && player->canDiscard(p, "e"))
                targets << p;
            if (p->getArmor())
                targets2 << p;
        }

        QStringList choices;
        if (!targets.isEmpty())
            choices << "discard";
        if (!targets2.isEmpty())
            choices << "get";
        if (choices.isEmpty()) return false;
        if (!player->askForSkillInvoke(this)) return false;
        room->broadcastSkillInvoke(objectName());
        QString choice = room->askForChoice(player, objectName(), choices.join("+"));
        if (choice == "discard") {
            ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName(), "@tenyearmumu-dis");
            room->doAnimate(1, player->objectName(), target->objectName());
            if (!player->canDiscard(target, "e")) return false;
            int id = room->askForCardChosen(player, target, "e", objectName(), false, Card::MethodDiscard);
            room->throwCard(id, target, player);
            room->addSlashCishu(player, 1);
        } else {
            ServerPlayer *target = room->askForPlayerChosen(player, targets2, objectName(), "@tenyearmumu-get");
            room->doAnimate(1, player->objectName(), target->objectName());
            if (!target->getArmor()) return false;
            room->obtainCard(player, target->getArmor(), true);
            room->addSlashCishu(player, -1);
        }
        return false;
    }
};

class TenyearZhixi : public TriggerSkill
{
public:
    TenyearZhixi() : TriggerSkill("tenyearzhixi")
    {
        events << CardUsed;
        frequency = Compulsory;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (player->getPhase() != Player::Play) return false;
        CardUseStruct use = data.value<CardUseStruct>();
        if (!use.card->isKindOf("Slash") && !use.card->isNDTrick()) return false;
        if (player->isKongcheng()) return false;
        room->sendCompulsoryTriggerLog(player, objectName(), true, true);
        room->askForDiscard(player, objectName(), 1, 1);
        return false;
    }
};

YujueCard::YujueCard(QString zhihu) : zhihu(zhihu)
{
    target_fixed = true;
}

void YujueCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    QStringList choices;
    for (int i = 0; i < 5; i++) {
        if (source->hasEquipArea(i))
            choices << QString::number(i);
    }
    if (choices.isEmpty()) return;

    QString choice = room->askForChoice(source, "yujue", choices.join("+"));
    source->throwEquipArea(choice.toInt());

    if (source->isDead()) return;

    QList<ServerPlayer *> targets;
    foreach (ServerPlayer *p, room->getOtherPlayers(source)) {
        if (p->isKongcheng()) continue;
        targets << p;
    }
    if (targets.isEmpty()) return;

    QString skill = "yujue";
    if (zhihu == "secondzhihu") skill = "secondyujue";
    ServerPlayer *target = room->askForPlayerChosen(source, targets, skill, "@yujue-invoke");
    room->doAnimate(1, source->objectName(), target->objectName());

    if (target->isKongcheng()) return;
    const Card *c = room->askForExchange(target, "yujue", 1, 1, false, "@yujue-give:" + source->objectName());
    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, target->objectName(), source->objectName(), "yujue", QString());
    room->obtainCard(source, c, reason, false);

    if (target->isDead() || source->isDead()) return;
    if (target->hasSkill(zhihu, true)) return;

    QStringList names = source->tag[zhihu + "_names"].toStringList();
    if (!names.contains(target->objectName())) {
        names << target->objectName();
        source->tag[zhihu + "_names"] = names;
    }
    room->acquireSkill(target, zhihu);
}

class YujueVS : public ZeroCardViewAsSkill
{
public:
    YujueVS() : ZeroCardViewAsSkill("yujue")
    {
    }

    const Card *viewAs() const
    {
        return new YujueCard;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return player->hasEquipArea() && !player->hasUsed("YujueCard");
    }
};

class Yujue : public PhaseChangeSkill
{
public:
    Yujue() : PhaseChangeSkill("yujue")
    {
        view_as_skill = new YujueVS;
    }

    bool onPhaseChange(ServerPlayer *player) const
    {
        if (player->getPhase() != Player::RoundStart) return false;
        QStringList names = player->tag["zhihu_names"].toStringList();
        if (names.isEmpty()) return false;
        Room *room = player->getRoom();
        player->tag.remove("zhihu_names");
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            if (!names.contains(p->objectName())) continue;
            if (!p->hasSkill("zhihu", true)) continue;
            targets << p;
        }
        if (targets.isEmpty()) return false;
        room->sortByActionOrder(targets);
        foreach (ServerPlayer *p, targets) {
            if (p->isDead() || !p->hasSkill("zhihu", true)) continue;
            room->detachSkillFromPlayer(p, "zhihu");
        }
        return false;
    }
};

SecondYujueCard::SecondYujueCard() : YujueCard("secondzhihu")
{
    target_fixed = true;
}

class SecondYujueVS : public ZeroCardViewAsSkill
{
public:
    SecondYujueVS() : ZeroCardViewAsSkill("secondyujue")
    {
    }

    const Card *viewAs() const
    {
        return new SecondYujueCard;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return player->hasEquipArea() && !player->hasUsed("SecondYujueCard");
    }
};

class SecondYujue : public PhaseChangeSkill
{
public:
    SecondYujue() : PhaseChangeSkill("secondyujue")
    {
        view_as_skill = new SecondYujueVS;
    }

    bool onPhaseChange(ServerPlayer *player) const
    {
        if (player->getPhase() != Player::RoundStart) return false;
        QStringList names = player->tag["secondzhihu_names"].toStringList();
        if (names.isEmpty()) return false;
        Room *room = player->getRoom();
        player->tag.remove("secondzhihu_names");
        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            if (!names.contains(p->objectName())) continue;
            if (!p->hasSkill("secondzhihu", true)) continue;
            targets << p;
        }
        if (targets.isEmpty()) return false;
        room->sortByActionOrder(targets);
        foreach (ServerPlayer *p, targets) {
            if (p->isDead() || !p->hasSkill("secondzhihu", true)) continue;
            room->detachSkillFromPlayer(p, "secondzhihu");
        }
        return false;
    }
};

class Tuxing : public TriggerSkill
{
public:
    Tuxing() : TriggerSkill("tuxing")
    {
        events << ThrowEquipArea << DamageCaused;
        frequency = Compulsory;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == ThrowEquipArea) {
            if (!player->hasSkill(this)) return false;
            QVariantList areas = data.toList();
            for (int i = 0; i < areas.length(); i++) {
                room->sendCompulsoryTriggerLog(player, objectName(), true, true);
                room->gainMaxHp(player);
                room->recover(player, RecoverStruct(player));
            }
            if (player->hasEquipArea()) return false;
            room->loseMaxHp(player, 4);
            player->tag["Tuxing_Damage"] = true;
        } else {
            if (!player->tag["Tuxing_Damage"].toBool()) return false;
            DamageStruct damage = data.value<DamageStruct>();
            LogMessage log;
            log.type = "#TuxingDamage";
            log.from = player;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(++damage.damage);
            room->sendLog(log);
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class Zhihu : public TriggerSkill
{
public:
    Zhihu() : TriggerSkill("zhihu")
    {
        events << Damage;
        frequency = Compulsory;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (!room->hasCurrent()) return false;
        if (player->getMark("zhihu-Clear") >= 2) return false;
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from == damage.to) return false;
        room->sendCompulsoryTriggerLog(player, objectName(), true, true);
        room->addPlayerMark(player, "zhihu-Clear");
        player->drawCards(2, objectName());
        return false;
    }
};

class SecondZhihu : public TriggerSkill
{
public:
    SecondZhihu() : TriggerSkill("secondzhihu")
    {
        events << Damage;
        frequency = Compulsory;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (!room->hasCurrent()) return false;
        if (player->getMark("secondzhihu-Clear") >= 3) return false;
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from == damage.to) return false;
        room->sendCompulsoryTriggerLog(player, objectName(), true, true);
        room->addPlayerMark(player, "secondzhihu-Clear");
        player->drawCards(1, objectName());
        return false;
    }
};

SpNiluanCard::SpNiluanCard()
{
    mute = true;
    will_throw = false;
    handling_method = Card::MethodUse;
}

bool SpNiluanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    const Card *card = Sanguosha->getCard(getSubcards().first());
    Slash *slash = new Slash(card->getSuit(), card->getNumber());
    slash->addSubcard(card);
    slash->setSkillName("spniluan");
    slash->deleteLater();
    return !Self->isLocked(slash) && slash->targetFilter(targets, to_select, Self) && to_select->getHp() > Self->getHp();
}

void SpNiluanCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    const Card *card = Sanguosha->getCard(getSubcards().first());
    Slash *slash = new Slash(card->getSuit(), card->getNumber());
    slash->addSubcard(card);
    slash->setSkillName("spniluan");
    slash->deleteLater();
    room->useCard(CardUseStruct(slash, card_use.from, card_use.to), true);
}

class SpNiluanVS : public OneCardViewAsSkill
{
public:
    SpNiluanVS() : OneCardViewAsSkill("spniluan")
    {
        filter_pattern = ".|black";
        response_or_use = true;
    }

    const Card *viewAs(const Card *card) const
    {
        SpNiluanCard *c = new SpNiluanCard;
        c->addSubcard(card);
        return c;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return Slash::IsAvailable(player);
    }
};

class SpNiluan : public TriggerSkill
{
public:
    SpNiluan() : TriggerSkill("spniluan")
    {
        events << DamageDone << CardFinished;
        view_as_skill = new SpNiluanVS;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *, QVariant &data) const
    {
        if (event == DamageDone) {
            DamageStruct damage = data.value<DamageStruct>();
            if (!damage.card || !damage.card->isKindOf("Slash") || !damage.from) return false;
            if (damage.card->getSkillName() != objectName()) return false;
            room->setCardFlag(damage.card, "spniluan_damage");
        } else {
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.card->isKindOf("Slash") || use.from->isDead() || !use.m_addHistory) return false;
            if (use.card->getSkillName() != objectName()) return false;
            if (use.card->hasFlag("spniluan_damage")) return false;
            room->addPlayerHistory(use.from, "Slash", -1);
        }
        return false;
    }
};

WeiwuCard::WeiwuCard()
{
    mute = true;
    will_throw = false;
    handling_method = Card::MethodUse;
}

bool WeiwuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    const Card *card = Sanguosha->getCard(getSubcards().first());
    Snatch *snatch = new Snatch(card->getSuit(), card->getNumber());
    snatch->addSubcard(card);
    snatch->setSkillName("weiwu");
    snatch->deleteLater();
    return !Self->isLocked(snatch) && snatch->targetFilter(targets, to_select, Self) && to_select->getHandcardNum() > Self->getHandcardNum()
            && !Self->isProhibited(to_select, snatch);
}

void WeiwuCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    const Card *card = Sanguosha->getCard(getSubcards().first());
    Snatch *snatch = new Snatch(card->getSuit(), card->getNumber());
    snatch->addSubcard(card);
    snatch->setSkillName("weiwu");
    snatch->deleteLater();
    room->useCard(CardUseStruct(snatch, card_use.from, card_use.to), true);
}

class Weiwu : public OneCardViewAsSkill
{
public:
    Weiwu() : OneCardViewAsSkill("weiwu")
    {
        filter_pattern = ".|red";
        response_or_use = true;
    }

    const Card *viewAs(const Card *card) const
    {
        WeiwuCard *c = new WeiwuCard;
        c->addSubcard(card);
        return c;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        Snatch *snatch = new Snatch(Card::NoSuit, 0);
        snatch->setSkillName("weiwu");
        snatch->deleteLater();
        return !player->hasUsed("WeiwuCard") && !player->isLocked(snatch);
    }
};

class Gongjian : public TriggerSkill
{
public:
    Gongjian() : TriggerSkill("gongjian")
    {
        events << TargetSpecified;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if (!use.card->isKindOf("Slash")) return false;
        QStringList names = room->getTag("gongjian_slash_targets").toStringList();
        foreach (ServerPlayer *p, use.to) {
            if (player->isDead() || player->getMark("gongjian-Clear") > 0) return false;
            if (!names.contains(p->objectName()) || p->isDead() || !player->canDiscard(p, "he")) continue;
            player->tag["gongjianData"] = data;
            bool invoke = player->askForSkillInvoke(this, p);
            player->tag.remove("gongjianData");
            if (!invoke) continue;
            room->broadcastSkillInvoke(objectName());
            room->addPlayerMark(player, "gongjian-Clear");

            int n = qMin(p->getCards("he").length(), 2);
            QStringList dis_num;
            for (int i = 1; i <= n; ++i)
                dis_num << QString::number(i);

            int ad = Config.AIDelay;
            Config.AIDelay = 0;

            bool ok = false;
            int discard_n = room->askForChoice(player, objectName(), dis_num.join("+"), QVariant::fromValue(p)).toInt(&ok);
            if (!ok || discard_n == 0) {
                Config.AIDelay = ad;
                continue;
            }

            QList<Player::Place> orig_places;
            QList<int> cards;
            p->setFlags("gongjian_InTempMoving");
            for (int i = 0; i < n; ++i) {
                if (!player->canDiscard(p, "he")) break;
                int id = room->askForCardChosen(player, p, "he", objectName(), false, Card::MethodDiscard);
                Player::Place place = room->getCardPlace(id);
                orig_places << place;
                cards << id;
                p->addToPile("#gongjian", id, false);
            }
            for (int i = 0; i < n; ++i) {
                if (orig_places.isEmpty()) break;
                room->moveCardTo(Sanguosha->getCard(cards.value(i)), p, orig_places.value(i), false);
            }
            p->setFlags("-gongjian_InTempMoving");
            Config.AIDelay = ad;

            if (!cards.isEmpty()) {
                DummyCard dummy(cards);
                room->throwCard(&dummy, p, player);
                DummyCard *slash = new DummyCard;
                slash->deleteLater();
                foreach (int id, cards) {
                    if (!Sanguosha->getCard(id)->isKindOf("Slash")) continue;
                    slash->addSubcard(id);
                }
                if (slash->subcardsLength() <= 0) continue;
                room->obtainCard(player, slash, true);
            }
        }
        return false;
    }
};

class GongjianRecord : public TriggerSkill
{
public:
    GongjianRecord() : TriggerSkill("#gongjian-record")
    {
        events << CardFinished;
        global = true;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if (!use.card->isKindOf("Slash")) return false;
        QStringList names;
        foreach (ServerPlayer *p, use.to) {
            if (names.contains(p->objectName())) continue;
            names << p->objectName();
        }
        room->setTag("gongjian_slash_targets", names);
        return false;
    }
};

class Kuimang : public TriggerSkill
{
public:
    Kuimang() : TriggerSkill("kuimang")
    {
        events << Death;
        frequency = Compulsory;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        DeathStruct death = data.value<DeathStruct>();
        if (!player->tag["kuimang_damage_" + death.who->objectName()].toBool()) return false;
        room->sendCompulsoryTriggerLog(player, objectName(), true, true);
        player->drawCards(2, objectName());
        return false;
    }
};

class KuimangRecord : public TriggerSkill
{
public:
    KuimangRecord() : TriggerSkill("#kuimang-record")
    {
        events << PreDamageDone;
        global = true;
    }

    bool trigger(TriggerEvent, Room *, ServerPlayer *, QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (!damage.from) return false;
        damage.from->tag["kuimang_damage_" + damage.to->objectName()] = true;
        return false;
    }
};

class TenyearFuqi : public TriggerSkill
{
public:
    TenyearFuqi() : TriggerSkill("tenyearfuqi")
    {
        events << CardUsed;
        frequency = Compulsory;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("SkillCard")) return false;

        QList<ServerPlayer *> tos;
        foreach (ServerPlayer *p, room->getOtherPlayers(use.from)) {
            if (use.from->distanceTo(p) != 1) continue;
            tos << p;
            use.no_respond_list << p->objectName();
            room->setCardFlag(use.card, "no_respond_" + use.from->objectName() + p->objectName());
        }
        if (tos.isEmpty()) return false;

        LogMessage log;
        log.type = "#FuqiNoResponse";
        log.from = use.from;
        log.arg = objectName();
        log.card_str = use.card->toString();
        log.to = tos;
        room->sendLog(log);
        room->notifySkillInvoked(use.from, objectName());
        room->broadcastSkillInvoke(objectName());

        data = QVariant::fromValue(use);
        return false;
    }
};

CixiaoCard::CixiaoCard()
{
    mute = true;
}

bool CixiaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const
{
    if (targets.length() > 1) return false;
    if (targets.isEmpty())
        return to_select->getMark("&cxyizi") > 0;
    return true;
}

bool CixiaoCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const
{
    return targets.length() == 2;
}

void CixiaoCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    CardUseStruct use = card_use;
    QVariant data = QVariant::fromValue(use);
    RoomThread *thread = room->getThread();

    thread->trigger(PreCardUsed, room, card_use.from, data);
    use = data.value<CardUseStruct>();

    room->broadcastSkillInvoke("cixiao");

    LogMessage log;
    log.from = card_use.from;
    log.to << card_use.to;
    log.type = "#UseCard";
    log.card_str = toString();
    room->sendLog(log);

    CardMoveReason reason(CardMoveReason::S_REASON_THROW, card_use.from->objectName(), QString(), "cixiao", QString());
    room->moveCardTo(this, card_use.from, NULL, Player::DiscardPile, reason, true);

    thread->trigger(CardUsed, room, card_use.from, data);
    use = data.value<CardUseStruct>();
    thread->trigger(CardFinished, room, card_use.from, data);
}

void CixiaoCard::use(Room *, ServerPlayer *, QList<ServerPlayer *> &targets) const
{
    ServerPlayer *first = targets.at(0);
    if (first->isDead() || first->getMark("&cxyizi") <= 0) return;
    ServerPlayer *second = targets.at(1);
    if (second->isDead()) return;

    first->loseMark("&cxyizi");
    second->gainMark("&cxyizi");
}

class CixiaoVS : public OneCardViewAsSkill
{
public:
    CixiaoVS() : OneCardViewAsSkill("cixiao")
    {
        filter_pattern = ".!";
        response_pattern = "@@cixiao";
    }

    const Card *viewAs(const Card *card) const
    {
        CixiaoCard *c = new CixiaoCard;
        c->addSubcard(card);
        return c;
    }
};

class Cixiao : public PhaseChangeSkill
{
public:
    Cixiao() : PhaseChangeSkill("cixiao")
    {
        view_as_skill = new CixiaoVS;
    }

    bool onPhaseChange(ServerPlayer *player) const
    {
        if (player->getPhase() != Player::Start) return false;
        Room *room = player->getRoom();
        bool yizi = false;
        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            if (p->getMark("&cxyizi") <= 0) continue;
            yizi = true;
            break;
        }
        if (yizi && player->canDiscard(player, "he"))
            room->askForUseCard(player, "@@cixiao", "@cixiao", -1, Card::MethodDiscard);
        else if (!yizi) {
            ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), "cixiao", "@cixiao-give", true, true);
            if (!target) return false;
            room->broadcastSkillInvoke(objectName());
            target->gainMark("&cxyizi");
        }
        return false;
    }
};

class CixiaoSkill : public TriggerSkill
{
public:
    CixiaoSkill() : TriggerSkill("#cixiao-skill")
    {
        events << MarkChanged;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    /*bool hasDingyuan(Room *room) const
    {
        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            if (p->hasSkill("cixiao"))
                return true;
        }
        return false;
    }*/

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        //if (!hasDingyuan(room)) return false;
        MarkStruct mark = data.value<MarkStruct>();
        if (mark.name != "&cxyizi") return false;
        if (player->getMark("&cxyizi") <= 0)
            room->detachSkillFromPlayer(player, "panshi");
        else
            room->acquireSkill(player, "panshi");
        return false;
    }
};

class Xianshuai : public TriggerSkill
{
public:
    Xianshuai() : TriggerSkill("xianshuai")
    {
        events << Damage << RoundStart;
        frequency = Compulsory;
        global = true;
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *, QVariant &data) const
    {
        if (event == Damage) {
            if (room->getTag("XianshuaiFirstDamage").toBool()) return false;
            room->setTag("XianshuaiFirstDamage", true);
            DamageStruct damage = data.value<DamageStruct>();
            foreach (ServerPlayer *p, room->getAllPlayers()) {
                if (p->isDead() || !p->hasSkill(this)) continue;
                room->sendCompulsoryTriggerLog(p, objectName(), true, true);
                p->drawCards(1, objectName());
                if (damage.from && p->isAlive() && p == damage.from && damage.to->isAlive())
                    room->damage(DamageStruct("xianshuai", p, damage.to));
            }
        } else
            room->removeTag("XianshuaiFirstDamage");

        return false;
    }
};

class Panshi : public TriggerSkill
{
public:
    Panshi() : TriggerSkill("panshi")
    {
        events << EventPhaseStart << DamageCaused << Damage;
        frequency = Compulsory;
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == EventPhaseStart) {
            if (player->getPhase() != Player::Start) return false;
            if (player->isKongcheng()) return false;
            //QList<ServerPlayer *> dingyuans = room->findPlayersBySkillName("cixiao");
            QList<ServerPlayer *> dingyuans;
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->hasSkill("cixiao", true) && p != player) {
                    dingyuans << p;
                    room->setPlayerFlag(p, "dingyuan");
                }
            }
            if (dingyuans.isEmpty()) return false;
            room->sendCompulsoryTriggerLog(player, objectName(), true, true);

            try {
                while (!player->isKongcheng()) {
                    if (player->isDead()) break;
                    QList<int> cards = player->handCards();
                    ServerPlayer *dingyuan = room->askForYiji(player, cards, objectName(), false, false, false, 1,
                                            dingyuans, CardMoveReason(), "@panshi-give", false);
                    if (!dingyuan) {
                        dingyuan = dingyuans.at(qrand() % dingyuans.length());
                        const Card *card = player->getRandomHandCard();
                        CardMoveReason reason(CardMoveReason::S_REASON_GIVE, player->objectName(), dingyuan->objectName(), "panshi", QString());
                        room->obtainCard(dingyuan, card, reason, false);
                    }
                    dingyuans.removeOne(dingyuan);
                    room->setPlayerFlag(dingyuan, "-dingyuan");
                    foreach (ServerPlayer *p, dingyuans) {
                        if (!p->hasSkill("cixiao", true)) {
                            dingyuans.removeOne(p);
                            room->setPlayerFlag(p, "-dingyuan");
                        }
                    }

                    if (dingyuans.isEmpty()) break;
                }
            }
            catch (TriggerEvent triggerEvent) {
                if (triggerEvent == TurnBroken || triggerEvent == StageChange) {
                    foreach (ServerPlayer *p, room->getAlivePlayers()) {
                        if (p->hasFlag("dingyuan"))
                            room->setPlayerFlag(p, "-dingyuan");
                    }
                }
                throw triggerEvent;
            }

            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->hasFlag("dingyuan"))
                    room->setPlayerFlag(p, "-dingyuan");
            }
        } else {
            if (player->getPhase() != Player::Play) return false;
            DamageStruct damage = data.value<DamageStruct>();
            if (!damage.card || !damage.card->isKindOf("Slash")) return false;
            if (!damage.to->hasSkill("cixiao", true)) return false;
            if (event == DamageCaused) {
                if (damage.to->isDead()) return false;
                LogMessage log;
                log.type = "#PanshiDamage";
                log.from = player;
                log.to << damage.to;
                log.arg = objectName();
                log.arg2 = QString::number(++damage.damage);
                room->sendLog(log);
                room->notifySkillInvoked(player, objectName());
                room->broadcastSkillInvoke(objectName());
                data = QVariant::fromValue(damage);
            } else {
                player->endPlayPhase();
            }
        }
        return false;
    }
};

JieyinghCard::JieyinghCard()
{
    mute = true;
}

bool JieyinghCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.length() < Self->getMark("&jieyingh") && to_select->hasFlag("jieyingh_canchoose");
}

void JieyinghCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    foreach (ServerPlayer *p, card_use.to)
        room->setPlayerFlag(p, "jieyingh_extratarget");
}

class JieyinghVS : public ZeroCardViewAsSkill
{
public:
    JieyinghVS() : ZeroCardViewAsSkill("jieyingh")
    {
        response_pattern = "@@jieyingh";
    }

    bool isEnabledAtPlay(const Player *) const
    {
        return false;
    }

    const Card *viewAs() const
    {
        if (Self->hasFlag("jieyingh_now_use_collateral"))
            return new ExtraCollateralCard;
        else
            return new JieyinghCard;
    }
};

class Jieyingh : public TriggerSkill
{
public:
    Jieyingh() : TriggerSkill("jieyingh")
    {
        events << PreCardUsed << Damage << EventPhaseChanging;
        global = true;
        view_as_skill = new JieyinghVS;
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == Damage) {
            if (player->getMark("&jieyingh") <= 0 || player->getPhase() == Player::NotActive) return false;
            LogMessage log;
            log.type = "#JieyinghEffect";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);
            room->setPlayerCardLimitation(player, "use", ".", true);
        } else if (event == EventPhaseChanging) {
            if (data.value<PhaseChangeStruct>().to != Player::NotActive) return false;
            if (player->getMark("&jieyingh") <= 0) return false;
            room->setPlayerMark(player, "&jieyingh", 0);
        } else {
            if (player->getMark("&jieyingh") <= 0) return false;
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Nullification")) return false;
            if (!use.card->isKindOf("Slash") && !use.card->isNDTrick()) return false;
            if (player->getPhase() == Player::NotActive) return false;

            if (!use.card->isKindOf("Collateral")) {
                bool canextra = false;
                foreach (ServerPlayer *p, room->getAlivePlayers()) {
                    if (use.card->isKindOf("AOE") && p == player) continue;
                    if (use.to.contains(p) || room->isProhibited(player, p, use.card)) continue;
                    if (use.card->targetFixed()) {
                        if (!use.card->isKindOf("Peach") || p->getLostHp() > 0) {
                            canextra = true;
                            room->setPlayerFlag(p, "jieyingh_canchoose");
                        }
                    } else {
                        if (use.card->targetFilter(QList<const Player *>(), p, player)) {
                            canextra = true;
                            room->setPlayerFlag(p, "jieyingh_canchoose");
                        }
                    }
                }

                if (canextra == false) return false;
                player->tag["jieyinghData"] = data;
                const Card *card = room->askForUseCard(player, "@@jieyingh", "@jieyingh:" + use.card->objectName());
                player->tag.remove("jieyinghData");
                if (!card) return false;
                QList<ServerPlayer *> add;
                foreach(ServerPlayer *p, room->getAlivePlayers()) {
                    if (p->hasFlag("jieyingh_canchoose"))
                        room->setPlayerFlag(p, "-jieyingh_canchoose");
                    if (p->hasFlag("jieyingh_extratarget")) {
                        room->setPlayerFlag(p,"-jieyingh_extratarget");
                        use.to.append(p);
                        add << p;
                    }
                }
                if (add.isEmpty()) return false;
                room->sortByActionOrder(add);
                LogMessage log;
                log.type = "#QiaoshuiAdd";
                log.from = player;
                log.to = add;
                log.card_str = use.card->toString();
                log.arg = "jieyingh";
                room->sendLog(log);
                foreach(ServerPlayer *p, add)
                    room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), p->objectName());

                room->sortByActionOrder(use.to);
                data = QVariant::fromValue(use);
            } else {
                for (int i = 1; i <= player->getMark("&jieyingh"); i++) {
                    bool canextra = false;
                    foreach (ServerPlayer *p, room->getAlivePlayers()) {
                        if (use.to.contains(p) || room->isProhibited(player, p, use.card)) continue;
                        if (use.card->targetFilter(QList<const Player *>(), p, player)) {
                            canextra = true;
                            break;
                        }
                    }
                    if (canextra == false) break;

                    QStringList toss;
                    QString tos;
                    foreach(ServerPlayer *t, use.to)
                        toss.append(t->objectName());
                    tos = toss.join("+");
                    room->setPlayerProperty(player, "extra_collateral", use.card->toString());
                    room->setPlayerProperty(player, "extra_collateral_current_targets", tos);
                    room->setPlayerFlag(player, "jieyingh_now_use_collateral");
                    room->askForUseCard(player, "@@jieyingh", "@jieyingh:" + use.card->objectName());
                    room->setPlayerFlag(player, "-jieyingh_now_use_collateral");
                    room->setPlayerProperty(player, "extra_collateral", QString());
                    room->setPlayerProperty(player, "extra_collateral_current_targets", QString());

                    foreach(ServerPlayer *p, room->getAlivePlayers()) {
                        if (p->hasFlag("ExtraCollateralTarget")) {
                            room->setPlayerFlag(p,"-ExtraCollateralTarget");
                            LogMessage log;
                            log.type = "#QiaoshuiAdd";
                            log.from = player;
                            log.to << p;
                            log.card_str = use.card->toString();
                            log.arg = "jieyingh";
                            room->sendLog(log);
                            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), p->objectName());

                            use.to.append(p);
                            ServerPlayer *victim = p->tag["collateralVictim"].value<ServerPlayer *>();
                            Q_ASSERT(victim != NULL);
                            if (victim) {
                                LogMessage log;
                                log.type = "#CollateralSlash";
                                log.from = player;
                                log.to << victim;
                                room->sendLog(log);
                                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, p->objectName(), victim->objectName());
                            }
                        }
                    }
                    room->sortByActionOrder(use.to);
                    data = QVariant::fromValue(use);
                }
            }
        }
        return false;
    }
};

class JieyinghInvoke : public PhaseChangeSkill
{
public:
    JieyinghInvoke() : PhaseChangeSkill("#jieyingh-invoke")
    {
    }

    bool onPhaseChange(ServerPlayer *player) const
    {
        if (player->getPhase() != Player::Finish || !player->hasSkill("jieyingh")) return false;
        Room *room = player->getRoom();
        ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), "jieyingh", "@jieyingh-invoke", true, true);
        if (!target) return false;
        room->broadcastSkillInvoke("jieyingh");
        room->addPlayerMark(target, "&jieyingh");
        return false;
    }
};

class JieyinghTargetMod : public TargetModSkill
{
public:
    JieyinghTargetMod() : TargetModSkill("#jieyingh-target")
    {
        frequency = NotFrequent;
        pattern = ".";
    }

    int getDistanceLimit(const Player *from, const Card *card, const Player *) const
    {
        if (from->getMark("&jieyingh") > 0 && (card->isKindOf("Slash") || card->isNDTrick()) && !card->isKindOf("Nullification"))
            return 1000;
        else
            return 0;
    }
};

class Weipo : public TriggerSkill
{
public:
    Weipo() : TriggerSkill("weipo")
    {
        events << CardFinished << TargetSpecified << EventPhaseStart;
        frequency = Compulsory;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == EventPhaseStart) {
            if (player->getPhase() != Player::RoundStart) return false;
            player->tag["Weipo_Wuxiao"] = false;
        } else if (event == TargetSpecified) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.card->isKindOf("Slash") && !use.card->isNDTrick()) return false;
            foreach (ServerPlayer *p, use.to) {
                if (p == use.from) continue;
                if (p->isDead() || !p->hasSkill(this) || p->tag["Weipo_Wuxiao"].toBool()) continue;
                if (p->getHandcardNum() >= p->getMaxHp()) continue;
                room->sendCompulsoryTriggerLog(p, objectName(), true, true);
                p->drawCards(p->getMaxHp() - p->getHandcardNum(), objectName());
                p->tag["weipo_" + use.card->toString()] = p->getHandcardNum() + 1;
            }
        } else {
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.card->isKindOf("Slash") && !use.card->isNDTrick()) return false;
            foreach (ServerPlayer *p, room->getAllPlayers(true)) {
                int n = p->tag["weipo_" + use.card->toString()].toInt() - 1;
                if (n >= 0) {
                    p->tag.remove("weipo_" + use.card->toString());
                    if (p->isDead() || p->getHandcardNum() >= n) continue;
                    p->tag["Weipo_Wuxiao"] = true;
                    if (use.from->isAlive() && !p->isKongcheng()) {
                        const Card *c = room->askForExchange(p, "weipo", 1, 1, false, "@weipo-give:" + use.from->objectName());
                        CardMoveReason reason(CardMoveReason::S_REASON_GIVE, p->objectName(), use.from->objectName(), "weipo", QString());
                        room->obtainCard(use.from, c, reason, false);
                    }
                }
            }
        }
        return false;
    }
};

MinsiCard::MinsiCard()
{
    target_fixed = true;
}

void MinsiCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    source->drawCards(2 * subcardsLength(), "minsi");
}

class MinsiVS : public ViewAsSkill
{
public:
    MinsiVS() : ViewAsSkill("minsi")
    {
    }

    bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        if (Self->isJilei(to_select)) return false;
        int num = 0;
        foreach (const Card *card, selected)
            num += card->getNumber();
        return num + to_select->getNumber() <= 13;
    }

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        int num = 0;
        foreach (const Card *card, cards)
            num += card->getNumber();
        if (num != 13) return NULL;

        MinsiCard *c = new MinsiCard;
        c->addSubcards(cards);
        return c;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("MinsiCard");
    }
};

class Minsi : public TriggerSkill
{
public:
    Minsi() : TriggerSkill("minsi")
    {
        events << CardsMoveOneTime;
        view_as_skill = new MinsiVS;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.reason.m_skillName != objectName() || move.to != player || move.to_place != Player::PlaceHand) return false;
        QStringList minsi_black = player->property("minsi_black").toString().split("+");
        QVariantList minsi_red = player->tag["minsi_red"].toList();
        foreach (int id, move.card_ids) {
            const Card *card = Sanguosha->getCard(id);
            if (card->isRed()) {
                //room->ignoreCards(player, id); //万一之后有什么技能让红色牌视为黑色，这些牌需要计入手牌数？
                if (minsi_red.contains(id)) continue;
                minsi_red << id;
            } else if (card->isBlack()) {
                if (minsi_black.contains(QString::number(id))) continue;
                minsi_black << QString::number(id);
            }
        }
        room->setPlayerProperty(player, "minsi_black", minsi_black.join("+"));
        player->tag["minsi_red"] = minsi_red;
        return false;
    }
};

class MinsiEffect : public TriggerSkill
{
public:
    MinsiEffect() : TriggerSkill("#minsi-effect")
    {
        events << EventPhaseChanging << EventPhaseProceeding;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == EventPhaseChanging) {
            if (data.value<PhaseChangeStruct>().to != Player::NotActive) return false;
            player->tag.remove("minsi_red");
            room->setPlayerProperty(player, "minsi_black", QString());
        } else {
            if (player->getPhase() != Player::Discard) return false;
            QVariantList minsi_red = player->tag["minsi_red"].toList();
            if (minsi_red.isEmpty()) return false;
            room->ignoreCards(player, VariantList2IntList(minsi_red));
        }
        return false;
    }
};

class MinsiTargetMod : public TargetModSkill
{
public:
    MinsiTargetMod() : TargetModSkill("#minsi-target")
    {
        frequency = NotFrequent;
        pattern = "^SkillCard";
    }

    int getDistanceLimit(const Player *from, const Card *card, const Player *) const
    {
        QStringList minsi_black = from->property("minsi_black").toString().split("+");
        if ((!card->isVirtualCard() || card->subcardsLength() == 1) && minsi_black.contains(QString::number(card->getEffectiveId()))
                && card->isBlack())
            return 1000;
        else
            return 0;
    }
};

JijingCard::JijingCard()
{
    mute = true;
    will_throw = false;
    target_fixed = true;
}

void JijingCard::onUse(Room *, const CardUseStruct &) const
{
}

class JijingVS : public ViewAsSkill
{
public:
    JijingVS() : ViewAsSkill("jijing")
    {
        response_pattern = "@@jijing";
    }

    bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        if (Self->isJilei(to_select)) return false;
        int judge_num = Self->property("jijing_judge").toInt();
        if (judge_num <= 0) return false;
        int num = 0;
        foreach (const Card *card, selected)
            num += card->getNumber();
        return num + to_select->getNumber() <= judge_num;
    }

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        int judge_num = Self->property("jijing_judge").toInt();
        if (judge_num <= 0) return NULL;
        int num = 0;
        foreach (const Card *card, cards)
            num += card->getNumber();
        if (num != judge_num) return NULL;

        JijingCard *card = new JijingCard;
        card->addSubcards(cards);
        return card;
    }
};

class Jijing : public MasochismSkill
{
public:
    Jijing() : MasochismSkill("jijing")
    {
        view_as_skill = new JijingVS;
    }

    void onDamaged(ServerPlayer *player, const DamageStruct &) const
    {
        if (!player->askForSkillInvoke(this)) return;
        Room *room = player->getRoom();
        room->broadcastSkillInvoke(objectName());

        JudgeStruct judge;
        judge.reason = objectName();
        judge.who = player;
        judge.pattern = ".";
        judge.play_animation = false;
        room->judge(judge);

        if (judge.card->getNumber() <= 0) {
            room->recover(player, RecoverStruct(player));
            return;
        }

        room->setPlayerProperty(player, "jijing_judge", judge.card->getNumber());
        const Card *card = room->askForUseCard(player, "@@jijing", "@jijing:" + QString::number(judge.card->getNumber()), -1, Card::MethodDiscard);
        room->setPlayerProperty(player, "jijing_judge", 0);
        if (!card) return;
        CardMoveReason reason(CardMoveReason::S_REASON_DISCARD, player->objectName(), "jijing", QString());
        room->throwCard(card, reason, player);
        room->recover(player, RecoverStruct(player));
    }
};

class Zhuide : public TriggerSkill
{
public:
    Zhuide() : TriggerSkill("zhuide")
    {
        events << Death;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->hasSkill(this);
    }

    int getDraw(const QString &name, Room *room) const
    {
        QList<int> ids;
        foreach (int id, room->getDrawPile()) {
            const Card *card = Sanguosha->getCard(id);
            if ((name == "slash" && card->isKindOf("Slash")) || card->objectName() == name)
                ids << id;
        }
        if (ids.isEmpty()) return -1;
        return ids.at(qrand() % ids.length());
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        DeathStruct death = data.value<DeathStruct>();
        if (death.who != player)
            return false;

        ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName(), "@zhuide-invoke", true, true);
        if (!target) return false;
        room->broadcastSkillInvoke(objectName());

        QStringList names;
        foreach (int id, room->getDrawPile()) {
            const Card *card = Sanguosha->getCard(id);
            if (!card->isKindOf("BasicCard")) continue;
            QString name = card->objectName();
            if (card->isKindOf("Slash"))
                name = "slash";
            if (names.contains(name)) continue;
            names << name;
        }
        if (names.isEmpty()) return false;

        QList<int> draw_ids;
        foreach (QString name, names) {
            int id = getDraw(name, room);
            if (id <0) continue;
            draw_ids << id;
        }
        if (draw_ids.isEmpty()) return false;

        CardsMoveStruct move;
        move.card_ids = draw_ids;
        move.from = NULL;
        move.to = target;
        move.to_place = Player::PlaceHand;
        move.reason = CardMoveReason(CardMoveReason::S_REASON_DRAW, target->objectName(), objectName(), QString());
        room->moveCardsAtomic(move, true);

        return false;
    }
};

class Shiyuan : public TriggerSkill
{
public:
    Shiyuan() : TriggerSkill("shiyuan")
    {
        events << TargetConfirmed;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (!room->hasCurrent()) return false;
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("SkillCard")) return false;
        if (use.from->isDead() || use.from == player || !use.to.contains(player)) return false;
        int n = 1;
        if (room->getCurrent()->getKingdom() == "qun" && player->hasLordSkill("yuwei"))
            n++;

        int from_hp = use.from->getHp();
        int player_hp = player->getHp();
        int draw_num = 3;

        if (from_hp > player_hp) {
            if (player->getMark("shiyuan_dayu-Clear") >= n) return false;
            room->addPlayerMark(player, "shiyuan_dayu-Clear");
        } else if (from_hp == player_hp) {
            if (player->getMark("shiyuan_dengyu-Clear") >= n) return false;
            draw_num = 2;
            room->addPlayerMark(player, "shiyuan_dengyu-Clear");
        } else {
            if (player->getMark("shiyuan_xiaoyu-Clear") >= n) return false;
            draw_num = 1;
            room->addPlayerMark(player, "shiyuan_xiaoyu-Clear");
        }

        room->sendCompulsoryTriggerLog(player, objectName(), true, true);
        player->drawCards(draw_num, objectName());
        return false;
    }
};

class SpDushi : public TriggerSkill
{
public:
    SpDushi() : TriggerSkill("spdushi")
    {
        events << Death;
        frequency = Compulsory;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->hasSkill(this);
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        DeathStruct death = data.value<DeathStruct>();
        if (death.who != player)
            return false;

        ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName(), "@spdushi-invoke", false, true);
        room->broadcastSkillInvoke(objectName());
        /*if (!target) {  askForPlayerChosen会自动随机选择
            if (death.damage && death.damage->from && death.damage->from->isAlive())
                target = death.damage->from;
            else
                target = room->getOtherPlayers(player).at(qrand() % room->getOtherPlayers(player).length());
        }*/
        room->handleAcquireDetachSkills(target, "spdushi");
        return false;
    }
};

class SpDushiDying : public TriggerSkill
{
public:
    SpDushiDying() : TriggerSkill("#spdushi-dying")
    {
        // just to broadcast audio effects and to send log messages
        // main part in the AskForPeaches trigger of Game Rule
        events << AskForPeaches;
        frequency = Compulsory;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    int getPriority(TriggerEvent) const
    {
        return 7;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (player == room->getAllPlayers().first()) {
            DyingStruct dying = data.value<DyingStruct>();
            if (!dying.who->hasSkill("spdushi")) return false;
            room->sendCompulsoryTriggerLog(dying.who, "spdushi", true, true);
        }
        return false;
    }
};

class SpDushiPro : public ProhibitSkill
{
public:
    SpDushiPro() : ProhibitSkill("#spdushi-pro")
    {
    }

    bool isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &) const
    {
        return to->hasSkill("spdushi") && card->isKindOf("Peach") && to->hasFlag("Global_Dying") && from != to;
    }
};

DaojiCard::DaojiCard()
{
    handling_method = Card::MethodDiscard;
}

bool DaojiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && to_select != Self && !to_select->getEquips().isEmpty();
}

void DaojiCard::onEffect(const CardEffectStruct &effect) const
{
    if (effect.to->getEquips().isEmpty() || effect.from->isDead()) return;
    Room *room = effect.from->getRoom();
    int id = room->askForCardChosen(effect.from, effect.to, "e", "daoji");
    room->obtainCard(effect.from, id);

    const Card *equip = Sanguosha->getCard(id);
    if (!equip->isAvailable(effect.from)) return;

    effect.from->tag["daoji_equip"] = id + 1;
    effect.from->tag["daoji_target_" + QString::number(id + 1)] = QVariant::fromValue(effect.to);

    room->useCard(CardUseStruct(equip, effect.from, effect.from));

    effect.from->tag.remove("daoji_equip");
    effect.from->tag.remove("daoji_target_" + QString::number(id + 1));
}

class DaojiVS : public OneCardViewAsSkill
{
public:
    DaojiVS() : OneCardViewAsSkill("daoji")
    {
    }

    bool viewFilter(const Card *to_select) const
    {
        return !Self->isJilei(to_select) && !to_select->isKindOf("BasicCard");
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("DaojiCard");
    }

    const Card *viewAs(const Card *originalcard) const
    {
        DaojiCard *card = new DaojiCard;
        card->addSubcard(originalcard);
        return card;
    }
};

class Daoji : public TriggerSkill
{
public:
    Daoji() : TriggerSkill("daoji")
    {
        events << CardFinished;
        view_as_skill = new DaojiVS;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if (!use.card->isKindOf("Weapon")) return false;

        int use_id = use.card->getEffectiveId();
        int id = player->tag["daoji_equip"].toInt() - 1;
        if (id < 0 || use_id != id) return false;

        ServerPlayer *target = player->tag["daoji_target_" + QString::number(id + 1)].value<ServerPlayer *>();
        if (!target) return false;

        player->tag.remove("daoji_equip");
        player->tag.remove("daoji_target_" + QString::number(id + 1));

        if (target->isDead()) return false;
        room->damage(DamageStruct("daoji", player, target));
        return false;
    }
};

class MobileNiluan : public PhaseChangeSkill
{
public:
    MobileNiluan() : PhaseChangeSkill("mobileniluan")
    {
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target && target->isAlive() && target->getPhase() == Player::Finish && target->getMark("qieting") > 0;
    }

    bool onPhaseChange(ServerPlayer *player) const
    {
        Room *room = player->getRoom();
        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            if (player->isDead()) return false;
            if (p->isDead() || !p->hasSkill(this)) continue;
            if (!p->canSlash(player, NULL, false)) continue;

            try {
                p->setFlags("MobileniluanSlash");
                const Card *slash = room->askForUseSlashTo(p, player, "@mobileniluan:" + player->objectName(), false, false, false,
                                    NULL, NULL, "mobileniluan_slash");
                if (!slash) {
                    p->setFlags("-MobileniluanSlash");
                    continue;
                }
            }
            catch (TriggerEvent triggerEvent) {
                if (triggerEvent == TurnBroken || triggerEvent == StageChange) {
                    if (p->hasFlag("mobileniluan_damage_" + player->objectName()))
                        room->setPlayerFlag(p, "-mobileniluan_damage_" + player->objectName());
                }
                throw triggerEvent;
            }

            if (!p->hasFlag("mobileniluan_damage_" + player->objectName())) continue;
            if (player->isDead() || !p->canDiscard(player, "he")) continue;
            int id = room->askForCardChosen(p, player, "he", objectName(), false, Card::MethodDiscard);
            room->throwCard(id, player, p);
        }
        return false;
    }
};

class MobileNiluanLog : public TriggerSkill
{
public:
    MobileNiluanLog() : TriggerSkill("#mobileniluan-log")
    {
        events << ChoiceMade;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (player->hasFlag("MobileniluanSlash") && data.canConvert<CardUseStruct>()) {
            room->broadcastSkillInvoke("mobileniluan");
            room->notifySkillInvoked(player, "mobileniluan");

            LogMessage log;
            log.type = "#InvokeSkill";
            log.from = player;
            log.arg = "mobileniluan";
            room->sendLog(log);

            player->setFlags("-MobileniluanSlash");
        }
        return false;
    }
};

class MobileNiluanDamage : public TriggerSkill
{
public:
    MobileNiluanDamage() : TriggerSkill("#mobileniluan-damage")
    {
        events << DamageDone;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target && target->isAlive();
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from && damage.from->isAlive() && damage.card && damage.card->isKindOf("Slash")) {
            if (!damage.card->hasFlag("mobileniluan_slash")) return false;
            room->setPlayerFlag(damage.from, "mobileniluan_damage_" + damage.to->objectName());
        }
        return false;
    }
};

class MobileXiaoxi : public OneCardViewAsSkill
{
public:
    MobileXiaoxi() : OneCardViewAsSkill("mobilexiaoxi")
    {
        response_or_use = true;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return Slash::IsAvailable(player);
    }

    bool isEnabledAtResponse(const Player *, const QString &pattern) const
    {
        return pattern == "slash";
    }

    bool viewFilter(const Card *card) const
    {
        if (!card->isBlack())
            return false;

        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY) {
            Slash *slash = new Slash(Card::SuitToBeDecided, -1);
            slash->addSubcard(card->getEffectiveId());
            slash->deleteLater();
            return slash->isAvailable(Self);
        }
        return true;
    }

    const Card *viewAs(const Card *originalCard) const
    {
        Card *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
        slash->addSubcard(originalCard->getId());
        slash->setSkillName(objectName());
        return slash;
    }
};

PingjianCard::PingjianCard()
{
    target_fixed = true;
}

void PingjianCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    //QStringList skills = Sanguosha->getSkillNames();
    QStringList pingjian_skills = source->property("pingjian_has_used_skills").toStringList();
    QStringList ava_generals;
    QStringList general_names = Sanguosha->getLimitedGeneralNames();
    foreach (QString general_name, general_names) {
        if (ava_generals.contains(general_name)) continue;
        const General *general = Sanguosha->getGeneral(general_name);
        foreach (const Skill *skill, general->getSkillList()) {
            if (!skill->isVisible() || skill->objectName() == "pingjian") continue;
            if (pingjian_skills.contains(skill->objectName())) continue;
            if (!skill->inherits("ViewAsSkill")) continue;
            QString translation = skill->getDescription();
            if (!translation.contains("出牌阶段限一次，") && !translation.contains("阶段技，") && !translation.contains("出牌阶段限一次。")
                    && !translation.contains("阶段技。")) continue;
            if (translation.contains("，出牌阶段限一次") || translation.contains("，阶段技") || translation.contains("（出牌阶段限一次") ||
                    translation.contains("（阶段技")) continue;
            ava_generals << general_name;
        }
    }

    QStringList generals;
    for (int i = 1; i <= 3; i++) {
        if (ava_generals.isEmpty()) break;
        QString general = ava_generals.at(qrand() % ava_generals.length());
        generals << general;
        ava_generals.removeOne(general);
    }
    if (generals.isEmpty()) return;

    QString general = room->askForGeneral(source, generals);

    QStringList skills;
    foreach (const Skill *skill, Sanguosha->getGeneral(general)->getSkillList()) {
        if (!skill->isVisible() || skill->objectName() == "pingjian") continue;
        if (pingjian_skills.contains(skill->objectName())) continue;
        if (!skill->inherits("ViewAsSkill")) continue;
        QString translation = skill->getDescription();
        if (!translation.contains("出牌阶段限一次，") && !translation.contains("阶段技，") && !translation.contains("出牌阶段限一次。")
                && !translation.contains("阶段技。")) continue;
        if (translation.contains("，出牌阶段限一次") || translation.contains("，阶段技") || translation.contains("（出牌阶段限一次") ||
                translation.contains("（阶段技")) continue;
        skills << skill->objectName();
    }
    if (skills.isEmpty()) return;

    QString skill_name = room->askForChoice(source, "pingjian", skills.join("+"));
    pingjian_skills << skill_name;
    room->setPlayerProperty(source, "pingjian_has_used_skills", pingjian_skills);
    if (!Sanguosha->getSkill(skill_name)->inherits("ViewAsSkill")) return;

    room->setPlayerProperty(source, "PingjianNowUseSkill", "@@" + skill_name);

    try {
        room->askForUseCard(source, "@@" + skill_name, "@pingjian:" + skill_name);
    }
    catch (TriggerEvent triggerEvent) {
        if (triggerEvent == TurnBroken || triggerEvent == StageChange)
            room->setPlayerProperty(source, "PingjianNowUseSkill", QString());
        throw triggerEvent;
    }
    room->setPlayerProperty(source, "PingjianNowUseSkill", QString());
}

class PingjianVS : public ZeroCardViewAsSkill
{
public:
    PingjianVS() : ZeroCardViewAsSkill("pingjian")
    {
    }

    const Card *viewAs() const
    {
        return new PingjianCard;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("PingjianCard");
    }
};

class Pingjian : public TriggerSkill
{
public:
    Pingjian() : TriggerSkill("pingjian")
    {
        events << Damaged << EventPhaseStart;
        view_as_skill = new PingjianVS;
    }

    void getpingjianskill(ServerPlayer *source, const QString &str, const QString &strr, TriggerEvent event, QVariant &data) const
    {
        Room *room = source->getRoom();
        QStringList pingjian_skills = source->property("pingjian_has_used_skills").toStringList();
        QStringList ava_generals;
        QStringList general_names = Sanguosha->getLimitedGeneralNames();
        foreach (QString general_name, general_names) {
            if (ava_generals.contains(general_name)) continue;
            const General *general = Sanguosha->getGeneral(general_name);
            foreach (const Skill *skill, general->getSkillList()) {
                if (!skill->isVisible() || skill->objectName() == "pingjian") continue;
                if (pingjian_skills.contains(skill->objectName())) continue;

                if (!skill->inherits("TriggerSkill")) continue;
                const TriggerSkill *triggerskill = Sanguosha->getTriggerSkill(skill->objectName());
                if (!triggerskill) continue;
                bool has_event = false;
                if (triggerskill->getTriggerEvents().contains(event))
                    has_event = true;
                else {
                    foreach (const Skill *related_sk, Sanguosha->getRelatedSkills(skill->objectName())) {
                        if (!related_sk || !related_sk->inherits("TriggerSkill")) continue;
                        const TriggerSkill *related_trigger = Sanguosha->getTriggerSkill(related_sk->objectName());
                        if (!related_trigger || !related_trigger->getTriggerEvents().contains(event)) continue;
                        has_event = true;
                    }
                }
                if (!has_event) continue;

                QString translation = skill->getDescription();
                if (!translation.contains(str) && !strr.isEmpty() && !translation.contains(strr)) continue;
                if (str == "结束阶段" && (translation.contains("的结束阶段") || translation.contains("结束阶段结束"))) continue;
                ava_generals << general_name;
            }
        }

        QStringList generals;
        for (int i = 1; i <= 3; i++) {
            if (ava_generals.isEmpty()) break;
            QString general = ava_generals.at(qrand() % ava_generals.length());
            generals << general;
            ava_generals.removeOne(general);
        }
        if (generals.isEmpty()) return;

        QString general = room->askForGeneral(source, generals);

        QStringList skills;
        foreach (const Skill *skill, Sanguosha->getGeneral(general)->getSkillList()) {
            if (!skill->isVisible() || skill->objectName() == "pingjian") continue;
            if (pingjian_skills.contains(skill->objectName())) continue;

            if (!skill->inherits("TriggerSkill")) continue;
            const TriggerSkill *triggerskill = Sanguosha->getTriggerSkill(skill->objectName());
            if (!triggerskill) continue;
            bool has_event = false;
            if (triggerskill->getTriggerEvents().contains(event))
                has_event = true;
            else {
                foreach (const Skill *related_sk, Sanguosha->getRelatedSkills(skill->objectName())) {
                    if (!related_sk || !related_sk->inherits("TriggerSkill")) continue;
                    const TriggerSkill *related_trigger = Sanguosha->getTriggerSkill(related_sk->objectName());
                    if (!related_trigger || !related_trigger->getTriggerEvents().contains(event)) continue;
                    has_event = true;
                }
            }
            if (!has_event) continue;

            QString translation = skill->getDescription();
            if (!translation.contains(str) && !strr.isEmpty() && !translation.contains(strr)) continue;
            if (str == "结束阶段" && (translation.contains("的结束阶段") || translation.contains("结束阶段结束"))) continue;
            skills << skill->objectName();
        }
        if (skills.isEmpty()) return;

        QString skill_name = room->askForChoice(source, "pingjian", skills.join("+"));
        pingjian_skills << skill_name;
        room->setPlayerProperty(source, "pingjian_has_used_skills", pingjian_skills);

        const TriggerSkill *triggerskill = Sanguosha->getTriggerSkill(skill_name);
        if (!triggerskill) return;
        bool has_event = false;
        if (triggerskill->getTriggerEvents().contains(event))
            has_event = true;
        else {
            foreach (const Skill *related_sk, Sanguosha->getRelatedSkills(skill_name)) {
                if (!related_sk || !related_sk->inherits("TriggerSkill")) continue;
                const TriggerSkill *related_trigger = Sanguosha->getTriggerSkill(related_sk->objectName());
                if (!related_trigger || !related_trigger->getTriggerEvents().contains(event)) continue;
                has_event = true;
                triggerskill = related_trigger;
                break;
            }
        }
        if (!has_event) return;

        room->setPlayerProperty(source, "pingjian_triggerskill", skill_name);
        room->getThread()->addTriggerSkill(triggerskill);
        triggerskill->trigger(event, room, source, data);
        room->setPlayerProperty(source, "pingjian_triggerskill", QString());
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == Damaged) {
            if (!player->askForSkillInvoke(this)) return false;
            room->broadcastSkillInvoke(objectName());
            getpingjianskill(player, "你受到伤害后", "你受到1点伤害后", event, data);
        } else if (event == EventPhaseStart) {
            if (player->getPhase() != Player::Finish) return false;
            if (!player->askForSkillInvoke(this)) return false;
            room->broadcastSkillInvoke(objectName());
            getpingjianskill(player, "结束阶段", "结束阶段开始", event, data);
        }
        return false;
    }
};

class Huqi : public MasochismSkill
{
public:
    Huqi() : MasochismSkill("huqi")
    {
        frequency = Compulsory;
    }

    void onDamaged(ServerPlayer *target, const DamageStruct &damage) const
    {
        if (target->getPhase() != Player::NotActive) return;
        Room *room = target->getRoom();
        room->sendCompulsoryTriggerLog(target, objectName(), true, true);
        JudgeStruct judge;
        judge.who = target;
        judge.reason = objectName();
        judge.pattern = ".|red";
        room->judge(judge);

        if (!judge.card->isRed()) return;
        if (!damage.from || damage.from->isDead()) return;
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("_huqi");
        slash->deleteLater();
        if (!target->canSlash(damage.from, slash, false)) return;
        room->useCard(CardUseStruct(slash, target, damage.from));
    }
};

class HuqiDistance : public DistanceSkill
{
public:
    HuqiDistance() : DistanceSkill("#huqi-distance")
    {
    }

    int getCorrect(const Player *from, const Player *) const
    {
        if (from->hasSkill("huqi"))
            return -1;
        else
            return 0;
    }
};

ShoufuCard::ShoufuCard()
{
    target_fixed = true;
}

void ShoufuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    source->drawCards(1, "shoufu");
    if (source->isKongcheng()) return;

    QList<ServerPlayer *> targets;
    foreach (ServerPlayer *p, room->getAlivePlayers()) {
        if (!p->getPile("sflu").isEmpty()) continue;
        targets << p;
    }
    if (targets.isEmpty()) return;

    if (!room->askForUseCard(source, "@@shoufu!", "@shoufu", Card::MethodNone)) {
        int id = source->getRandomHandCardId();
        targets.at(qrand() % targets.length())->addToPile("sflu", id);
    }
}

ShoufuPutCard::ShoufuPutCard()
{
    will_throw = false;
    mute = true;
    handling_method = Card::MethodNone;
    m_skillName = "shoufu";
}

bool ShoufuPutCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const
{
    return targets.isEmpty() && to_select->getPile("sflu").isEmpty();
}

void ShoufuPutCard::onUse(Room *, const CardUseStruct &card_use) const
{
    card_use.to.first()->addToPile("sflu", getSubcards());
}

class ShoufuVS : public ViewAsSkill
{
public:
    ShoufuVS() : ViewAsSkill("shoufu")
    {
    }

    bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY)
            return false;
        else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE)
            return selected.isEmpty() && !to_select->isEquipped();
        return false;
    }

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY) {
            if (!cards.isEmpty()) return NULL;
            return new ShoufuCard;
        } else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
            if (cards.length() != 1) return NULL;
            ShoufuPutCard *c = new ShoufuPutCard;
            c->addSubcard(cards.first());
            return c;
        }
        return NULL;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("ShoufuCard");
    }

    bool isEnabledAtResponse(const Player *, const QString &pattern) const
    {
        return pattern == "@@shoufu!";
    }
};

class Shoufu : public TriggerSkill
{
public:
    Shoufu() : TriggerSkill("shoufu")
    {
        events << CardsMoveOneTime << DamageInflicted;
        view_as_skill = new ShoufuVS;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && !target->getPile("sflu").isEmpty();
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == DamageInflicted) {
            player->clearOnePrivatePile("sflu");
        } else {
            if (player->getPhase() != Player::Discard) return false;
            int basic = player->tag["shoufu_basic"].toInt();
            int trick = player->tag["shoufu_trick"].toInt();
            int equip = player->tag["shoufu_equip"].toInt();
            DummyCard *dummy = new DummyCard;

            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if ((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD) {
                foreach (int id, move.card_ids) {
                    const Card *card = Sanguosha->getCard(id);
                    if (card->isKindOf("BasicCard"))
                        basic++;
                    if (card->isKindOf("TrickCard"))
                        trick++;
                    if (card->isKindOf("EquipCard"))
                        equip++;
                }
                player->tag["shoufu_basic"] = basic >= 2 ? 0 : basic;
                player->tag["shoufu_trick"] = trick >= 2 ? 0 : trick;
                player->tag["shoufu_equip"] = equip >= 2 ? 0 : equip;

            }
            foreach (int id, player->getPile("sflu")) {
                const Card *card = Sanguosha->getCard(id);
                if ((card->isKindOf("BasicCard") && basic >= 2) || (card->isKindOf("TrickCard") && trick >= 2) ||
                        (card->isKindOf("EquipCard") && equip >= 2))
                    dummy->addSubcard(card);
            }
            if (dummy->subcardsLength() > 0) {
                CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, player->objectName(), "shoufu", QString());
                room->throwCard(dummy, reason, NULL);
            }
            delete dummy;
        }
        return false;
    }
};

class ShoufuMove : public TriggerSkill
{
public:
    ShoufuMove() : TriggerSkill("#shoufu-move")
    {
        events << CardsMoveOneTime;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.to && move.to->isAlive() && move.to_place == Player::PlaceSpecial && move.to_pile_name == "sflu") {
            ServerPlayer *to = room->findChild<ServerPlayer *>(move.to->objectName());
            if (!to || to->isDead()) return false;
            QStringList colors = to->tag["Shoufu_Colors"].toStringList();
            foreach (int id, move.card_ids) {
                const Card *card = Sanguosha->getCard(id);
                if (colors.contains(card->getType())) continue;
                colors << card->getType();
            }
            to->tag["Shoufu_Colors"] = colors;
            foreach (QString type, colors) {
                if (type == "basic")
                    room->setPlayerCardLimitation(to, "use,response", "BasicCard", false);
                else if (type == "trick")
                    room->setPlayerCardLimitation(to, "use,response", "TrickCard", false);
                else if (type == "equip")
                    room->setPlayerCardLimitation(to, "use,response", "EquipCard", false);
            }
        } else if (move.from && move.from_places.contains(Player::PlaceSpecial) && move.from_pile_names.contains("sflu")) {
            for (int i = 0; i < move.card_ids.length(); i++) {
                ServerPlayer *from = room->findChild<ServerPlayer *>(move.from->objectName());
                if (!from) return false;
                QStringList colors = from->tag["Shoufu_Colors"].toStringList();
                if  (move.from_places.at(i) == Player::PlaceSpecial && move.from_pile_names.at(i) == "sflu") {
                    const Card *card = Sanguosha->getCard(move.card_ids.at(i));
                    if (!colors.contains(card->getType())) continue;
                    colors.removeOne(card->getType());
                    from->tag["Shoufu_Colors"] = colors;

                    QString type = card->getType();
                    if (type == "basic")
                        room->removePlayerCardLimitation(from, "use,response", "BasicCard$0");
                    else if (type == "trick")
                        room->removePlayerCardLimitation(from, "use,response", "TrickCard$0");
                    else if (type == "equip")
                        room->removePlayerCardLimitation(from, "use,response", "EquipCard$0");
                }
            }
        }
        return false;
    }
};

TenyearSongciCard::TenyearSongciCard()
{
    mute = true;
}

bool TenyearSongciCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && to_select->getMark("tenyearsongci" + Self->objectName()) == 0;
}

void TenyearSongciCard::onEffect(const CardEffectStruct &effect) const
{
    int handcard_num = effect.to->getHandcardNum();
    int hp = effect.to->getHp();
    Room *room = effect.from->getRoom();
    room->setPlayerMark(effect.to, "@songci", 1);
    room->addPlayerMark(effect.to, "tenyearsongci" + effect.from->objectName());
    if (handcard_num > hp) {
        room->broadcastSkillInvoke("tenyearsongci", 2);
        room->askForDiscard(effect.to, "tenyearsongci", 2, 2, false, true);
    } else if (handcard_num <= hp) {
        room->broadcastSkillInvoke("tenyearsongci", 1);
        effect.to->drawCards(2, "tenyearsongci");
    }
}

class TenyearSongciViewAsSkill : public ZeroCardViewAsSkill
{
public:
    TenyearSongciViewAsSkill() : ZeroCardViewAsSkill("tenyearsongci")
    {
    }

    const Card *viewAs() const
    {
        return new TenyearSongciCard;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        if (player->getMark("tenyearsongci" + player->objectName()) == 0) return true;
        foreach(const Player *sib, player->getAliveSiblings())
            if (sib->getMark("tenyearsongci" + player->objectName()) == 0)
                return true;
        return false;
    }
};

class TenyearSongci : public TriggerSkill
{
public:
    TenyearSongci() : TriggerSkill("tenyearsongci")
    {
        events << CardUsed;
        view_as_skill = new TenyearSongciViewAsSkill;
    }


    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (!data.value<CardUseStruct>().card->isKindOf("BifaCard")) return false;
        foreach (ServerPlayer *p, room->getAlivePlayers()) {
            if (p->getMark("tenyearsongci" + player->objectName()) <= 0)
                return false;
        }

        room->sendCompulsoryTriggerLog(player, objectName(), true, true);
        player->drawCards(1, objectName());
        return false;
    }
};

YoulongDialog *YoulongDialog::getInstance(const QString &object)
{
    static YoulongDialog *instance;
    if (instance == NULL || instance->objectName() != object)
        instance = new YoulongDialog(object);

    return instance;
}

YoulongDialog::YoulongDialog(const QString &object)
    : GuhuoDialog(object)
{
}

bool YoulongDialog::isButtonEnabled(const QString &button_name) const
{
    const Card *card = map[button_name];
    if (Self->getChangeSkillState("youlong") == 1 && !card->isNDTrick()) return false;
    if (Self->getChangeSkillState("youlong") == 2 && !card->isKindOf("BasicCard")) return false;
    return Self->getMark(objectName() + "_" + button_name) <= 0 && button_name != "normal_slash"
            && !Self->isCardLimited(card, Card::MethodUse) && card->isAvailable(Self);
}

YoulongCard::YoulongCard()
{
    mute = true;
}

bool YoulongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
    }

    const Card *_card = Self->tag.value("youlong").value<const Card *>();
    if (_card == NULL)
        return false;

    Card *card = Sanguosha->cloneCard(_card);
    card->setCanRecast(false);
    card->deleteLater();
    if (card && card->targetFixed())  //因源码bug，不得已而为之
        return targets.isEmpty() && to_select == Self && !Self->isProhibited(to_select, card, targets);
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
}

/*bool YoulongCard::targetFixed() const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetFixed();
    }

    const Card *_card = Self->tag.value("youlong").value<const Card *>();
    if (_card == NULL)
        return false;

    Card *card = Sanguosha->cloneCard(_card);
    card->setCanRecast(false);
    card->deleteLater();
    return card && card->targetFixed();
}*/

bool YoulongCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetsFeasible(targets, Self);
    }

    const Card *_card = Self->tag.value("youlong").value<const Card *>();
    if (_card == NULL)
        return false;

    Card *card = Sanguosha->cloneCard(_card);
    card->setCanRecast(false);
    card->deleteLater();
    return card && card->targetsFeasible(targets, Self);
}

const Card *YoulongCard::validate(CardUseStruct &card_use) const
{
    ServerPlayer *source = card_use.from;
    Room *room = source->getRoom();

    QString tl = user_string;
    if (user_string == "slash"
        && Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        QStringList tl_list;
        if (card_use.from->getMark("youlong_slash") <= 0)
            tl_list << "slash";
        if (!Config.BanPackages.contains("maneuvering")) {
            if (card_use.from->getMark("youlong_thunder_slash") <= 0)
                tl_list << "thunder_slash";
            if (card_use.from->getMark("youlong_fire_slash") <= 0)
                tl_list << "fire_slash";
        }
        if (tl_list.isEmpty()) return NULL;
        tl = room->askForChoice(source, "youlong_slash", tl_list.join("+"));
    }
    if (card_use.from->getMark("youlong_" + tl) > 0) return NULL;

    QStringList areas;
    for (int i = 0; i < 5; i++){
        if (source->hasEquipArea(i))
            areas << QString::number(i);
    }
    if (areas.isEmpty()) return NULL;

    if (source->getChangeSkillState("youlong") == 1) {
        room->addPlayerMark(source, "youlong_trick_lun");
        room->setChangeSkillState(source, "youlong", 2);
    } else {
        room->addPlayerMark(source, "youlong_basic_lun");
        room->setChangeSkillState(source, "youlong", 1);
    }

    QString area = room->askForChoice(source, "youlong", areas.join("+"));
    source->throwEquipArea(area.toInt());

    Card *use_card = Sanguosha->cloneCard(tl);
    use_card->setSkillName("youlong");
    use_card->deleteLater();

    room->addPlayerMark(source, "youlong_" + tl);

    //if (use_card->targetFixed())
        //card_use.to.clear();

    return use_card;
}

const Card *YoulongCard::validateInResponse(ServerPlayer *source) const
{
    Room *room = source->getRoom();
    QString tl;
    if (user_string == "peach+analeptic") {
        QStringList tl_list;
        if (source->getMark("youlong_peach") <= 0)
            tl_list << "peach";
        if (!Config.BanPackages.contains("maneuvering") && source->getMark("youlong_analeptic") <= 0)
            tl_list << "analeptic";
        if (tl_list.isEmpty()) return NULL;
        tl = room->askForChoice(source, "youlong_saveself", tl_list.join("+"));
    } else if (user_string == "slash") {
        QStringList tl_list;
        if (source->getMark("youlong_slash") <= 0)
            tl_list << "slash";
        if (!Config.BanPackages.contains("maneuvering")) {
            if (source->getMark("youlong_thunder_slash") <= 0)
                tl_list << "thunder_slash";
            if (source->getMark("youlong_fire_slash") <= 0)
                tl_list << "fire_slash";
        }
        if (tl_list.isEmpty()) return NULL;
        tl = room->askForChoice(source, "youlong_slash", tl_list.join("+"));
    } else
        tl = user_string;

    if (source->getMark("youlong_" + tl) > 0) return NULL;

    QStringList areas;
    for (int i = 0; i < 5; i++){
        if (source->hasEquipArea(i))
            areas << QString::number(i);
    }
    if (areas.isEmpty()) return NULL;

    if (source->getChangeSkillState("youlong") == 1) {
        room->addPlayerMark(source, "youlong_trick_lun");
        room->setChangeSkillState(source, "youlong", 2);
    } else {
        room->addPlayerMark(source, "youlong_basic_lun");
        room->setChangeSkillState(source, "youlong", 1);
    }

    QString area = room->askForChoice(source, "youlong", areas.join("+"));
    source->throwEquipArea(area.toInt());

    Card *use_card = Sanguosha->cloneCard(tl);
    use_card->setSkillName("youlong");
    use_card->deleteLater();

    room->addPlayerMark(source, "youlong_" + tl);

    return use_card;
}

class Youlong : public ZeroCardViewAsSkill
{
public:
    Youlong() : ZeroCardViewAsSkill("youlong")
    {
        change_skill = true;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        if (!player->hasEquipArea()) return false;
        return (player->getChangeSkillState(objectName()) == 1 && player->getMark("youlong_trick_lun") <= 0) ||
                (player->getChangeSkillState(objectName()) == 2 && player->getMark("youlong_basic_lun") <= 0);
    }

    bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE)
            return false;
        if (!player->hasEquipArea()) return false;
        Card *card = Sanguosha->cloneCard(pattern);
        if (!card) return false;
        card->deleteLater();
        return (player->getMark("youlong_trick_lun") <= 0 && card->isNDTrick() && player->getChangeSkillState("youlong") == 1) ||
                (player->getMark("youlong_basic_lun") <= 0 && card->isKindOf("BasicCard") && player->getChangeSkillState("youlong") == 2);
    }

    bool isEnabledAtNullification(const ServerPlayer *player) const
    {
        if (player->getMark("youlong_trick_lun") > 0) return false;
        return player->getMark("youlong_nullification") <= 0 && player->hasEquipArea() && player->getChangeSkillState("youlong") == 1;
    }

    const Card *viewAs() const
    {
        if (Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
            YoulongCard *card = new YoulongCard;
            card->setUserString(Sanguosha->getCurrentCardUsePattern());
            return card;
        }

        const Card *c = Self->tag.value("youlong").value<const Card *>();
        if (c && c->isAvailable(Self)) {
            YoulongCard *card = new YoulongCard;
            card->setUserString(c->objectName());
            return card;
        } else
            return NULL;
        return NULL;
    }

    QDialog *getDialog() const
    {
        return YoulongDialog::getInstance("youlong");
    }
};

class Luanfeng : public TriggerSkill
{
public:
    Luanfeng() : TriggerSkill("luanfeng")
    {
        events << Dying;
        frequency = Limited;
        limit_mark = "@luanfengMark";
    }


    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (player->getMark("@luanfengMark") <= 0) return false;
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.who->getMaxHp() < player->getMaxHp()) return false;
        if (!player->askForSkillInvoke(this, dying.who)) return false;

        room->broadcastSkillInvoke(objectName());
        room->doSuperLightbox("wolongfengchu", "luanfeng");
        room->removePlayerMark(player, "@luanfengMark");

        int recover = qMin(3, dying.who->getMaxHp()) - dying.who->getHp();
        room->recover(dying.who, RecoverStruct(player, NULL, recover));
        QList<int> list;
        for (int i = 0; i < 5; i++) {
            if (!dying.who->hasEquipArea(i))
                list << i;
        }
        if (!list.isEmpty())
            dying.who->obtainEquipArea(list);
        int n = 6 - list.length() - dying.who->getHandcardNum();
        if (n > 0)
            dying.who->drawCards(n, objectName());
        if (dying.who == player) {
            foreach (QString mark, player->getMarkNames()) {
                if (mark.startsWith("youlong_") && !mark.endsWith("_lun") && player->getMark(mark) > 0)
                    room->setPlayerMark(player, mark, 0);
            }
        }
        return false;
    }
};

SecondZhanyiViewAsBasicCard::SecondZhanyiViewAsBasicCard()
{
    m_skillName = "secondzhanyi";
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool SecondZhanyiViewAsBasicCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
    } else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return false;
    }

    const Card *card = Self->tag.value("secondzhanyi").value<const Card *>();
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
}

bool SecondZhanyiViewAsBasicCard::targetFixed() const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetFixed();
    } else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return true;
    }

    const Card *card = Self->tag.value("secondzhanyi").value<const Card *>();
    return card && card->targetFixed();
}

bool SecondZhanyiViewAsBasicCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        const Card *card = NULL;
        if (!user_string.isEmpty())
            card = Sanguosha->cloneCard(user_string.split("+").first());
        return card && card->targetsFeasible(targets, Self);
    } else if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        return true;
    }

    const Card *card = Self->tag.value("secondzhanyi").value<const Card *>();
    return card && card->targetsFeasible(targets, Self);
}

const Card *SecondZhanyiViewAsBasicCard::validate(CardUseStruct &card_use) const
{
    ServerPlayer *zhuling = card_use.from;
    Room *room = zhuling->getRoom();

    QString to_zhanyi = user_string;
    if (user_string == "slash" && Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
        QStringList guhuo_list;
        guhuo_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            guhuo_list << "normal_slash" << "thunder_slash" << "fire_slash";
        to_zhanyi = room->askForChoice(zhuling, "secondzhanyi_slash", guhuo_list.join("+"));
    }

    const Card *card = Sanguosha->getCard(subcards.first());
    QString user_str;
    if (to_zhanyi == "slash") {
        if (card->isKindOf("Slash"))
            user_str = card->objectName();
        else
            user_str = "slash";
    } else if (to_zhanyi == "normal_slash")
        user_str = "slash";
    else
        user_str = to_zhanyi;
    Card *use_card = Sanguosha->cloneCard(user_str, card->getSuit(), card->getNumber());
    use_card->setSkillName("secondzhanyi");
    use_card->addSubcard(subcards.first());
    use_card->deleteLater();
    return use_card;
}

const Card *SecondZhanyiViewAsBasicCard::validateInResponse(ServerPlayer *zhuling) const
{
    Room *room = zhuling->getRoom();

    QString to_zhanyi;
    if (user_string == "peach+analeptic") {
        QStringList guhuo_list;
        guhuo_list << "peach";
        if (!Config.BanPackages.contains("maneuvering"))
            guhuo_list << "analeptic";
        to_zhanyi = room->askForChoice(zhuling, "secondzhanyi_saveself", guhuo_list.join("+"));
    } else if (user_string == "slash") {
        QStringList guhuo_list;
        guhuo_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            guhuo_list << "normal_slash" << "thunder_slash" << "fire_slash";
        to_zhanyi = room->askForChoice(zhuling, "secondzhanyi_slash", guhuo_list.join("+"));
    } else
        to_zhanyi = user_string;

    const Card *card = Sanguosha->getCard(subcards.first());
    QString user_str;
    if (to_zhanyi == "slash") {
        if (card->isKindOf("Slash"))
            user_str = card->objectName();
        else
            user_str = "slash";
    } else if (to_zhanyi == "normal_slash")
        user_str = "slash";
    else
        user_str = to_zhanyi;
    Card *use_card = Sanguosha->cloneCard(user_str, card->getSuit(), card->getNumber());
    use_card->setSkillName("secondzhanyi");
    use_card->addSubcard(subcards.first());
    use_card->deleteLater();
    return use_card;
}

SecondZhanyiCard::SecondZhanyiCard()
{
    target_fixed = true;
}

void SecondZhanyiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    room->loseHp(source);
    if (source->isAlive()) {
        const Card *c = Sanguosha->getCard(subcards.first());
        if (c->getTypeId() == Card::TypeBasic) {
            room->setPlayerMark(source, "ViewAsSkill_secondzhanyiEffect-PlayClear", 1);
            room->setPlayerMark(source, "Secondzhanyieffect-PlayClear", 1);
        } else if (c->getTypeId() == Card::TypeEquip)
            room->setPlayerMark(source, "secondzhanyiEquip-PlayClear", 1);
        else if (c->getTypeId() == Card::TypeTrick) {
            source->drawCards(3, "secondzhanyi");
            room->setPlayerMark(source, "secondzhanyiTrick-PlayClear", 1);
        }
    }
}

class SecondZhanyiVS : public OneCardViewAsSkill
{
public:
    SecondZhanyiVS() : OneCardViewAsSkill("secondzhanyi")
    {

    }

    bool isResponseOrUse() const
    {
        return Self->getMark("ViewAsSkill_secondzhanyiEffect-PlayClear") > 0;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        if (!player->hasUsed("SecondZhanyiCard"))
            return true;

        if (player->getMark("ViewAsSkill_secondzhanyiEffect-PlayClear") > 0)
            return true;

        return false;
    }

    bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        if (player->getMark("ViewAsSkill_secondzhanyiEffect-PlayClear") == 0) return false;
        if (pattern.startsWith(".") || pattern.startsWith("@")) return false;
        if (pattern == "peach" && player->getMark("Global_PreventPeach") > 0) return false;
        for (int i = 0; i < pattern.length(); i++) {
            QChar ch = pattern[i];
            if (ch.isUpper() || ch.isDigit()) return false; // This is an extremely dirty hack!! For we need to prevent patterns like 'BasicCard'
        }
        return !(pattern == "nullification");
    }

    bool viewFilter(const Card *to_select) const
    {
        if (Self->getMark("ViewAsSkill_secondzhanyiEffect-PlayClear") > 0)
            return to_select->isKindOf("BasicCard");
        else
            return true;
    }

    const Card *viewAs(const Card *originalCard) const
    {
        if (Self->getMark("ViewAsSkill_secondzhanyiEffect-PlayClear") == 0) {
            SecondZhanyiCard *zy = new SecondZhanyiCard;
            zy->addSubcard(originalCard);
            return zy;
        }

        if (Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE
            || Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) {
            SecondZhanyiViewAsBasicCard *card = new SecondZhanyiViewAsBasicCard;
            card->setUserString(Sanguosha->getCurrentCardUsePattern());
            card->addSubcard(originalCard);
            return card;
        }

        const Card *c = Self->tag.value("secondzhanyi").value<const Card *>();
        if (c && c->isAvailable(Self)) {
            SecondZhanyiViewAsBasicCard *card = new SecondZhanyiViewAsBasicCard;
            card->setUserString(c->objectName());
            card->addSubcard(originalCard);
            return card;
        } else
            return NULL;
    }
};

class SecondZhanyi : public TriggerSkill
{
public:
    SecondZhanyi() : TriggerSkill("secondzhanyi")
    {
        events << PreHpRecover << ConfirmDamage << PreCardUsed << PreCardResponded << TrickCardCanceling;
        view_as_skill = new SecondZhanyiVS;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    QDialog *getDialog() const
    {
        return GuhuoDialog::getInstance("secondzhanyi", true, false);
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == TrickCardCanceling) {
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if (!effect.card->isKindOf("TrickCard")) return false;
            if (!effect.from || effect.from->getMark("secondzhanyiTrick-PlayClear") <= 0) return false;
            return true;
        } else if (event == PreHpRecover) {
            RecoverStruct recover = data.value<RecoverStruct>();
            if (!recover.card || !recover.card->hasFlag("secondzhanyi_effect")) return false;
            int old = recover.recover;
            ++recover.recover;
            int now = qMin(recover.recover, player->getMaxHp() - player->getHp());
            if (now <= 0)
                return true;
            if (recover.who && now > old) {
                LogMessage log;
                log.type = "#NewlonghunRecover";
                log.from = recover.who;
                log.to << player;
                log.arg = objectName();
                log.arg2 = QString::number(now);
                room->sendLog(log);
            }
            recover.recover = now;
            data = QVariant::fromValue(recover);
        } else if (event == ConfirmDamage) {
            DamageStruct damage = data.value<DamageStruct>();
            if (!damage.card || !damage.card->hasFlag("secondzhanyi_effect") || !damage.by_user) return false;

            LogMessage log;
            log.type = "#NewlonghunDamage";
            log.from = player;
            log.to << damage.to;
            log.arg = objectName();
            log.arg2 = QString::number(++damage.damage);
            room->sendLog(log);

            data = QVariant::fromValue(damage);
        } else {
            if (player->getMark("Secondzhanyieffect-PlayClear") <= 0) return false;
            const Card *card = NULL;
            if (event == PreCardUsed)
                card = data.value<CardUseStruct>().card;
            else {
                CardResponseStruct res = data.value<CardResponseStruct>();
                if (!res.m_isUse) return false;
                card = res.m_card;
            }
            if (!card || !card->isKindOf("BasicCard")) return false;
            room->setPlayerMark(player, "Secondzhanyieffect-PlayClear", 0);
            room->setCardFlag(card, "secondzhanyi_effect");
        }
        return false;
    }
};

class SecondZhanyiEquip : public TriggerSkill
{
public:
    SecondZhanyiEquip() : TriggerSkill("#secondzhanyi-equip")
    {
        events << TargetSpecified;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (player->getMark("secondzhanyiEquip-PlayClear") <= 0) return false;
        CardUseStruct use = data.value<CardUseStruct>();
        if (!use.card->isKindOf("Slash") || use.to.isEmpty()) return false;
        room->sendCompulsoryTriggerLog(player, "secondzhanyi", true, true);
        foreach (ServerPlayer *p, use.to) {
            if (p->isDead() || !p->canDiscard(p, "he")) continue;
            const Card *c = room->askForDiscard(p, "secondzhanyi", 2, 2, false, true);
            if (!c || player->isDead()) continue;
            room->fillAG(c->getSubcards(), player);
            int id = room->askForAG(player, c->getSubcards(), false, "secondzhanyi");
            room->clearAG(player);
            room->obtainCard(player, id);
        }
        return false;
    }
};

class Pianchong : public DrawCardsSkill
{
public:
    Pianchong() : DrawCardsSkill("pianchong")
    {
        frequency = Frequent;
    }

    int getDrawNum(ServerPlayer *player, int n) const
    {
        Room *room = player->getRoom();
        if (room->askForSkillInvoke(player, objectName())) {
            room->broadcastSkillInvoke("pianchong");
            QList<int> red, black;
            foreach (int id, room->getDrawPile()) {
                if (Sanguosha->getCard(id)->isBlack())
                    black << id;
                else if (Sanguosha->getCard(id)->isRed())
                    red << id;
            }
            DummyCard *dummy = new DummyCard;
            if (!red.isEmpty())
                dummy->addSubcard(red.at(qrand() % red.length()));
            if (!black.isEmpty())
                dummy->addSubcard(black.at(qrand() % black.length()));
            if (dummy->subcardsLength() > 0)
                room->obtainCard(player, dummy);
            delete dummy;
            QString choice = room->askForChoice(player, objectName(), "red+black");
            if (choice == "red")
                room->addPlayerMark(player, "&pianchong+red");
            else
                room->addPlayerMark(player, "&pianchong+black");
            return -n;
        } else
            return n;
    }
};

class PianchongEffect : public TriggerSkill
{
public:
    PianchongEffect() : TriggerSkill("#pianchong-effect")
    {
        events << CardsMoveOneTime << EventPhaseStart;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target && target->isAlive();
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == EventPhaseStart) {
            if (player->getPhase() == Player::RoundStart) {
                room->setPlayerMark(player, "&pianchong+red", 0);
                room->setPlayerMark(player, "&pianchong+black", 0);
            }
        } else {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (player != move.from) return false;
            if (!move.from_places.contains(Player::PlaceHand) && !move.from_places.contains(Player::PlaceEquip)) return false;
            QList<int> red, black;
            foreach (int id, room->getDrawPile()) {
                if (Sanguosha->getCard(id)->isBlack())
                    black << id;
                else if (Sanguosha->getCard(id)->isRed())
                    red << id;
            }
            DummyCard *dummy = new DummyCard;
            for (int i = 0; i < move.card_ids.length(); i++) {
                if (move.from_places.at(i) == Player::PlaceHand || move.from_places.at(i) == Player::PlaceEquip) {
                    const Card *card = Sanguosha->getCard(move.card_ids.at(i));
                    if (card->isRed()) {
                        int mark = move.from->getMark("&pianchong+red");
                        for (int j = 0; j < mark; j++) {
                            if (black.isEmpty()) break;
                            int id = black.at(qrand() % black.length());
                            black.removeOne(id);
                            dummy->addSubcard(id);
                        }
                    } else if (card->isBlack()) {
                        int mark = move.from->getMark("&pianchong+black");
                        for (int j = 0; j < mark; j++) {
                            if (red.isEmpty()) break;
                            int id = red.at(qrand() % red.length());
                            red.removeOne(id);
                            dummy->addSubcard(id);
                        }
                    }
                }
            }
            if (dummy->subcardsLength() > 0)
                room->obtainCard(player, dummy);
            delete dummy;
        }
        return false;
    }
};

ZunweiCard::ZunweiCard()
{
}

bool ZunweiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    QList<const Player *> all = QList<const Player *>(), al = Self->getAliveSiblings();
    foreach (const Player *p, al) {
        if (!Self->property("zunwei_draw").toBool() && p->getHandcardNum() > Self->getHandcardNum())
            all << p;
        else if (!Self->property("zunwei_equip").toBool() && p->getEquips().length() > Self->getEquips().length() && Self->hasEquipArea())
            all << p;
        else if (!Self->property("zunwei_recover").toBool() && p->getHp() > Self->getHp() && Self->getLostHp() > 0)
            all << p;
    }
    return targets.isEmpty() && all.contains(to_select);
}

void ZunweiCard::onEffect(const CardEffectStruct &effect) const
{
    QStringList choices;
    if (!effect.from->property("zunwei_draw").toBool() && effect.to->getHandcardNum() > effect.from->getHandcardNum())
        choices << "draw";
    if (!effect.from->property("zunwei_equip").toBool() && effect.to->getEquips().length() > effect.from->getEquips().length() && effect.from->hasEquipArea())
        choices << "equip";
    if (!effect.from->property("zunwei_recover").toBool() && effect.to->getHp() > effect.from->getHp() && effect.from->getLostHp() > 0)
        choices << "recover";
    if (choices.isEmpty()) return;

    Room *room = effect.from->getRoom();
    QString choice = room->askForChoice(effect.from, "zunwei", choices.join("+"), QVariant::fromValue(effect.to));
    if (choice == "draw") {
        room->setPlayerProperty(effect.from, "zunwei_draw", true);
        int num = effect.to->getHandcardNum() - effect.from->getHandcardNum();
        num = qMin(num, 5);
        effect.from->drawCards(num, "zunwei");
    } else if (choice == "recover") {
        room->setPlayerProperty(effect.from, "zunwei_recover", true);
        int recover = effect.to->getHp() - effect.from->getHp();
        recover = qMin(recover, effect.from->getMaxHp() - effect.from->getHp());
        room->recover(effect.from, RecoverStruct(effect.from, NULL, recover));
    } else {
       room->setPlayerProperty(effect.from, "zunwei_equip", true);
        QList<const Card *> equips;
        foreach (int id, room->getDrawPile()) {
            const Card *card = Sanguosha->getCard(id);
            if (card->isKindOf("EquipCard") && effect.from->canUse(card))
                equips << card;
        }
        if (equips.isEmpty()) return;
        while (effect.to->getEquips().length() > effect.from->getEquips().length()) {
            if (effect.from->isDead()) break;
            if (equips.isEmpty()) break;
            const Card *equip = equips.at(qrand() % equips.length());
            equips.removeOne(equip);
            if (effect.from->canUse(equip))
                room->useCard(CardUseStruct(equip, effect.from, effect.from));
        }
    }
}

class Zunwei : public ZeroCardViewAsSkill
{
public:
    Zunwei() : ZeroCardViewAsSkill("zunwei")
    {
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->property("zunwei_draw").toBool() || (!player->property("zunwei_recover").toBool() && player->getLostHp() > 0) ||
                (!player->property("zunwei_equip").toBool() && player->hasEquipArea());
    }

    const Card *viewAs() const
    {
        ZunweiCard *card = new ZunweiCard;
        return card;
    }
};

class Juliao : public DistanceSkill
{
public:
    Juliao() : DistanceSkill("juliao")
    {
    }

    int getCorrect(const Player *, const Player *to) const
    {
        if (to->hasSkill(this)) {
            int extra = 0;
            QSet<QString> kingdom_set;
            if (to->parent()) {
                foreach(const Player *player, to->parent()->findChildren<const Player *>())
                {
                    if (player->isAlive())
                        kingdom_set << player->getKingdom();
                }
            }
            extra = kingdom_set.size();
            return qMax(0, extra - 1);
        } else
            return 0;
    }
};

class Taomie : public TriggerSkill
{
public:
    Taomie() : TriggerSkill("taomie")
    {
        events << Damage << Damaged << DamageCaused;
    }

    bool transferMark(ServerPlayer *to, Room *room) const
    {
        int n = 0;
        foreach (ServerPlayer *p, room->getOtherPlayers(to)) {
            if (to->isDead()) break;
            if (p->isAlive() && p->getMark("&taomie") > 0) {
                n++;
                int mark = p->getMark("&taomie");
                p->loseAllMarks("&taomie");
                to->gainMark("&taomie", mark);
            }
        }
        return n > 0;
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (event == Damage) {
            if (damage.to->isDead() || damage.to->getMark("&taomie") > 0 || !player->askForSkillInvoke(this, damage.to)) return false;
            room->broadcastSkillInvoke(objectName());
            if (transferMark(damage.to, room)) return false;
            damage.to->gainMark("&taomie", 1);
        } else if (event == Damaged) {
            if (!damage.from || damage.from->isDead() || damage.from->getMark("&taomie") > 0 ||
                    !player->askForSkillInvoke(this, damage.from)) return false;
            room->broadcastSkillInvoke(objectName());
            if (transferMark(damage.from, room)) return false;
            damage.from->gainMark("&taomie", 1);
        } else {
            if (damage.to->isDead() || damage.to->getMark("&taomie") <= 0) return false;
            room->sendCompulsoryTriggerLog(player, objectName(), true, true);
            QStringList choices;
            choices << "damage";
            if (!damage.to->isAllNude())
                choices << "get";
            choices << "all";
            QString choice = room->askForChoice(player, objectName(), choices.join("+"), data);
            LogMessage log;
            log.type = "#FumianFirstChoice";
            log.from = player;
            log.arg = "taomie:" + choice;
            room->sendLog(log);
            if (choice == "damage") {
                ++damage.damage;
                data = QVariant::fromValue(damage);
            } else if (choice == "get") {
                if (damage.to->isAllNude()) return false;
                int id = room->askForCardChosen(player, damage.to, "hej", objectName());
                room->obtainCard(player, id, false);
                if (player->isDead() || room->getCardPlace(id) != Player::PlaceHand || room->getCardOwner(id) != player) return false;
                QList<int> list;
                list << id;
                room->askForYiji(player, list, objectName());
            } else {
                damage.to->setMark("taomie_throwmark", 1);
                ++damage.damage;
                data = QVariant::fromValue(damage);
                if (damage.to->isAllNude()) return false;
                int id = room->askForCardChosen(player, damage.to, "hej", objectName());
                room->obtainCard(player, id, false);
                if (player->isDead() || room->getCardPlace(id) != Player::PlaceHand || room->getCardOwner(id) != player) return false;
                QList<int> list;
                list << id;
                room->askForYiji(player, list, objectName());
            }
        }
        return false;
    }
};

class TaomieMark : public TriggerSkill
{
public:
    TaomieMark() : TriggerSkill("#taomie-mark")
    {
        events << DamageComplete;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive() && target->getMark("&taomie") > 0 && target->getMark("taomie_throwmark") > 0;
    }

    bool trigger(TriggerEvent, Room *, ServerPlayer *player, QVariant &) const
    {
        player->setMark("taomie_throwmark", 0);
        player->loseAllMarks("&taomie");
        return false;
    }
};

class Cangchu : public TriggerSkill
{
public:
    Cangchu() : TriggerSkill("cangchu")
    {
        events << GameStart << CardsMoveOneTime;
        frequency = Compulsory;
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == GameStart) {
            room->sendCompulsoryTriggerLog(player, objectName(), true, true);
            player->gainMark("&ccliang", 3);
        } else {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.to && move.to->isAlive() && move.to_place == Player::PlaceHand && move.to->getMark("cangchu-Clear") <= 0 &&
                    move.to->getPhase() == Player::NotActive && move.to == player && room->hasCurrent()) {
                int mark = player->getMark("&ccliang");
                if (mark >= room->alivePlayerCount()) return false;
                room->sendCompulsoryTriggerLog(player, objectName(), true, true);
                room->addPlayerMark(player, "cangchu-Clear");
                player->gainMark("&ccliang");
            }
        }
        return false;
    }
};

class CangchuKeep : public MaxCardsSkill
{
public:
    CangchuKeep() : MaxCardsSkill("#cangchu-keep")
    {
    }

    int getExtra(const Player *target) const
    {
        if (target->hasSkill("cangchu"))
            return target->getMark("&ccliang");
        else
            return 0;
    }
};

class Liangying : public PhaseChangeSkill
{
public:
    Liangying() : PhaseChangeSkill("liangying")
    {
    }

    bool onPhaseChange(ServerPlayer *player) const
    {
        if (player->getPhase() != Player::Discard) return false;
        int mark = player->getMark("&ccliang");
        if (mark <= 0 || !player->askForSkillInvoke(objectName())) return false;
        Room *room = player->getRoom();
        room->broadcastSkillInvoke(objectName());

        QStringList nums;
        for (int i = 1; i <= mark; i++)
            nums << QString::number(i);

        QString num = room->askForChoice(player, objectName(), nums.join("+"));
        player->drawCards(num.toInt(), objectName());
        if (player->isDead() || player->isKongcheng()) return false;

        QList<ServerPlayer *> alives = room->getAlivePlayers(), tos;
        while (!alives.isEmpty()) {
            if (player->isDead() || player->isKongcheng() || alives.isEmpty()) break;
            if (tos.length() >= num.toInt()) break;
            QList<int> hands = player->handCards();
            ServerPlayer *to = room->askForYiji(player, hands, objectName(), false, false, false, 1, alives, CardMoveReason(), "liangying-give");
            tos << to;
            alives.removeOne(to);
            room->addPlayerMark(to, "liangying-Clear");
        }

        foreach (ServerPlayer *p, room->getAllPlayers(true)) {
            if (p->getMark("liangying-Clear") > 0)
                room->setPlayerMark(p, "liangying-Clear", 0);
        }
        return false;
    }
};

class Shishou : public TriggerSkill
{
public:
    Shishou() : TriggerSkill("shishou")
    {
        events << CardUsed << Damaged << EventPhaseStart;
        frequency = Compulsory;
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == CardUsed) {
            const Card *card = data.value<CardUseStruct>().card;
            if (!card->isKindOf("Analeptic")) return false;
            if (player->getMark("&ccliang") <= 0) return false;
            room->sendCompulsoryTriggerLog(player, objectName(), true, true);
            player->loseMark("&ccliang");
        } else if (event == Damaged) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.nature != DamageStruct::Fire) return false;
            int lose = qMin(player->getMark("&ccliang"), damage.damage);
            if (lose <= 0) return false;
            room->sendCompulsoryTriggerLog(player, objectName(), true, true);
            player->loseMark("&ccliang");
        } else {
            if (player->getPhase() != Player::Start) return false;
            if (player->getMark("&ccliang") > 0) return false;
            room->sendCompulsoryTriggerLog(player, objectName(), true, true);
            room->loseHp(player);
        }
        return false;
    }
};

BazhanCard::BazhanCard()
{
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool BazhanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    int n = Self->getChangeSkillState("bazhan");
    if (n == 1)
        return targets.isEmpty() && to_select != Self;
    else if (n == 2)
        return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
    return false;
}

void BazhanCard::BazhanEffect(ServerPlayer *from, ServerPlayer *to) const
{
    if (from->isDead() || to->isDead()) return;
    QStringList choices;
    if (to->getLostHp() > 0)
        choices << "recover";
    choices << "reset" << "cancel";
    Room *room = from->getRoom();
    QString choice = room->askForChoice(from, "bazhan", choices.join("+"), QVariant::fromValue(to));
    if (choice == "cancel") return;
    if (choice == "recover")
        room->recover(to, RecoverStruct(from));
    else {
        if (to->isChained())
            room->setPlayerChained(to);

        if (!to->faceUp())
            to->turnOver();
    }
}

void BazhanCard::onEffect(const CardEffectStruct &effect) const
{
    ServerPlayer *from= effect.from;
    ServerPlayer *to = effect.to;
    Room *room = from->getRoom();
    int n = from->getChangeSkillState("bazhan");
    const Card *card = NULL;

    if (n == 1) {
        room->setChangeSkillState(from, "bazhan", 2);
        room->giveCard(from, to, subcards, "bazhan");
        card = Sanguosha->getCard(subcards.first());
        if (card && (card->isKindOf("Analeptic") || card->getSuit() == Card::Heart))
            BazhanEffect(from, to);
    } else if (n == 2) {
        room->setChangeSkillState(from, "bazhan", 1);
        if (to->isKongcheng()) return;
        int id = room->askForCardChosen(from, to, "h", "bazhan");
        room->obtainCard(from, id);
        card = Sanguosha->getCard(id);
        if (card && (card->isKindOf("Analeptic") || card->getSuit() == Card::Heart))
            BazhanEffect(from, from);
    }
}

class Bazhan : public ViewAsSkill
{
public:
    Bazhan() : ViewAsSkill("bazhan")
    {
        change_skill = true;
    }

    bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        int n = Self->getChangeSkillState("bazhan");
        //if (n != 1 && n != 2) return false;
        if (n == 1)
            return selected.isEmpty() && !to_select->isEquipped();
        return false;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("BazhanCard");
    }

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        int n = Self->getChangeSkillState("bazhan");
        if (n != 1 && n != 2) return NULL;
        if (n == 1 && cards.length() != 1) return NULL;
        if (n == 2 && cards.length() != 0) return NULL;

        BazhanCard *card = new BazhanCard;
        if (n == 1)
            card->addSubcards(cards);
        return card;
    }
};

class Jiaoying : public TriggerSkill
{
public:
    Jiaoying() : TriggerSkill("jiaoying")
    {
        events << EventPhaseChanging << CardUsed << CardResponded;
        frequency = Compulsory;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == EventPhaseChanging) {
            if (data.value<PhaseChangeStruct>().to != Player::NotActive) return false;
            foreach (ServerPlayer *fanyufeng, room->getAllPlayers()) {
                QStringList names = fanyufeng->property("jiaoying_targets").toStringList();
                room->setPlayerProperty(fanyufeng, "jiaoying_targets", QStringList());
                if (fanyufeng->isDead() || !fanyufeng->hasSkill(this)) continue;

                QList<ServerPlayer *> targets;
                foreach (QString name, names) {
                    ServerPlayer *target = room->findChild<ServerPlayer *>(name);
                    if (!target || target->isDead() || targets.contains(target)) continue;
                    targets << target;
                }
                if (targets.isEmpty()) continue;
                room->sortByActionOrder(targets);

                foreach (ServerPlayer *p, targets) {
                    if (p->getMark("jiaoying_card-Clear") > 0) continue;
                    ServerPlayer *drawer = room->askForPlayerChosen(fanyufeng, room->getAlivePlayers(), objectName(), "@jiaoying-invoke", false, true);
                    room->broadcastSkillInvoke(objectName());
                    int num = qMin(5, drawer->getMaxHp()) - drawer->getHandcardNum();
                    if (num > 0)
                        drawer->drawCards(num, objectName());
                }
            }
            foreach (ServerPlayer *p, room->getAllPlayers(true)) {
                QStringList colors = p->property("jiaoying_colors").toStringList();
                room->setPlayerProperty(p, "jiaoying_colors", QStringList());
                if (colors.isEmpty()) continue;
                foreach (QString color, colors)
                    room->removePlayerCardLimitation(p, "use,response", ".|" + color + "|.|.$1");
            }
        } else {
            if (!room->hasCurrent()) return false;
            const Card *card = NULL;
            if (event == CardUsed)
                card = data.value<CardUseStruct>().card;
            else
                card = data.value<CardResponseStruct>().m_card;
            if (!card || card->isKindOf("SkillCard")) return false;
            if (player->getMark("jiaoying_effect-Clear") <= 0) return false;
            room->addPlayerMark(player, "jiaoying_card-Clear");
        }
        return false;
    }
};

class JiaoyingMove : public TriggerSkill
{
public:
    JiaoyingMove() : TriggerSkill("#jiaoying-move")
    {
        events << CardsMoveOneTime;
        frequency = Compulsory;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (!move.to || !move.from || move.from != player || move.to == player || !move.from_places.contains(Player::PlaceHand) ||
                move.to_place != Player::PlaceHand) return false;

        QStringList names = player->property("jiaoying_targets").toStringList();
        if (!names.contains(move.to->objectName())) {
            names << move.to->objectName();
            room->setPlayerProperty(player, "jiaoying_targets", names);
            room->addPlayerMark((ServerPlayer *)move.to, "jiaoying_effect-Clear");
        }

        for (int i = 0; i < move.card_ids.length(); i++) {
            const Card *card = Sanguosha->getCard(move.card_ids.at(i));
            QString color;
            if (card->isRed())
                color = "red";
            else if (card->isBlack())
                color = "black";
            else
                continue;
            QStringList colors = move.to->property("jiaoying_colors").toStringList();
            if (colors.contains(color)) continue;
            colors << color;
            room->setPlayerProperty((ServerPlayer *)move.to, "jiaoying_colors", colors);
            room->setPlayerCardLimitation((ServerPlayer *)move.to, "use,response", ".|" + color + "|.|.", true);
        }
        return false;
    }
};

SP3Package::SP3Package()
    : Package("sp3")
{
    General *tenyear_sunluyu = new General(this, "tenyear_sunluyu", "wu", 3, false);
    tenyear_sunluyu->addSkill(new TenyearMeibu);
    tenyear_sunluyu->addSkill(new TenyearMumu);
    tenyear_sunluyu->addRelateSkill("tenyearzhixi");

    General *liuhong = new General(this, "liuhong", "qun", 4);
    liuhong->addSkill(new Yujue);
    liuhong->addSkill(new Tuxing);
    liuhong->addRelateSkill("zhihu");

    General *second_liuhong = new General(this, "second_liuhong", "qun", 4);
    second_liuhong->addSkill(new SecondYujue);
    second_liuhong->addSkill("tuxing");
    second_liuhong->addRelateSkill("secondzhihu");

    General *sp_hansui = new General(this, "sp_hansui", "qun", 4);
    sp_hansui->addSkill(new SpNiluan);
    sp_hansui->addSkill(new Weiwu);

    General *zhujun = new General(this, "zhujun", "qun", 4);
    zhujun->addSkill(new Gongjian);
    zhujun->addSkill(new GongjianRecord);
    zhujun->addSkill(new FakeMoveSkill("gongjian"));
    zhujun->addSkill(new Kuimang);
    zhujun->addSkill(new KuimangRecord);
    related_skills.insertMulti("gongjian", "#gongjian-record");
    related_skills.insertMulti("gongjian", "#gongjian-fake-move");
    related_skills.insertMulti("kuimang", "#kuimang-record");

    General *tenyear_quyi = new General(this, "tenyear_quyi", "qun", 4);
    tenyear_quyi->addSkill(new TenyearFuqi);
    tenyear_quyi->addSkill("jiaozi");

    General *tenyear_dingyuan = new General(this, "tenyear_dingyuan", "qun", 4);
    tenyear_dingyuan->addSkill(new Cixiao);
    tenyear_dingyuan->addSkill(new CixiaoSkill);
    tenyear_dingyuan->addSkill(new Xianshuai);
    tenyear_dingyuan->addRelateSkill("panshi");
    related_skills.insertMulti("cixiao", "#cixiao-skill");

    General *hanfu = new General(this, "hanfu", "qun", 4);
    hanfu->addSkill(new Jieyingh);
    hanfu->addSkill(new JieyinghInvoke);
    hanfu->addSkill(new JieyinghTargetMod);
    hanfu->addSkill(new Weipo);
    related_skills.insertMulti("jieyingh", "#jieyingh-invoke");
    related_skills.insertMulti("jieyingh", "#jieyingh-target");

    General *wangrong = new General(this, "wangrong", "qun", 3, false);
    wangrong->addSkill(new Minsi);
    wangrong->addSkill(new MinsiEffect);
    wangrong->addSkill(new MinsiTargetMod);
    wangrong->addSkill(new Jijing);
    wangrong->addSkill(new Zhuide);
    related_skills.insertMulti("minsi", "#minsi-effect");
    related_skills.insertMulti("minsi", "#minsi-target");

    General *liubian = new General(this, "liubian$", "qun", 3);
    liubian->addSkill(new Shiyuan);
    liubian->addSkill(new SpDushi);
    liubian->addSkill(new SpDushiDying);
    liubian->addSkill(new SpDushiPro);
    liubian->addSkill(new Skill("yuwei$", Skill::Compulsory));
    related_skills.insertMulti("spdushi", "#spdushi-dying");
    related_skills.insertMulti("spdushi", "#spdushi-pro");

    General *hucheer = new General(this, "hucheer", "qun", 4);
    hucheer->addSkill(new Daoji);

    General *mobile_hansui = new General(this, "mobile_hansui", "qun", 4);
    mobile_hansui->addSkill(new MobileNiluan);
    mobile_hansui->addSkill(new MobileNiluanLog);
    mobile_hansui->addSkill(new MobileNiluanDamage);
    mobile_hansui->addSkill(new MobileXiaoxi);
    related_skills.insertMulti("mobileniluan", "#mobileniluan-log");
    related_skills.insertMulti("mobileniluan", "#mobileniluan-damage");

    General *xushao = new General(this, "xushao", "qun", 4);
    xushao->addSkill(new Pingjian);

    General *zhangling = new General(this, "zhangling", "qun", 3);
    zhangling->addSkill(new Huqi);
    zhangling->addSkill(new HuqiDistance);
    zhangling->addSkill(new Shoufu);
    zhangling->addSkill(new ShoufuMove);
    related_skills.insertMulti("huqi", "#huqi-distance");
    related_skills.insertMulti("shoufu", "#shoufu-move");

    General *tenyear_chenlin = new General(this, "tenyear_chenlin", "wei", 3);
    tenyear_chenlin->addSkill("bifa");
    tenyear_chenlin->addSkill(new TenyearSongci);

    General *wolongfengchu = new General(this, "wolongfengchu", "shu", 4);
    wolongfengchu->addSkill(new Youlong);
    wolongfengchu->addSkill(new Luanfeng);

    General *second_zhuling = new General(this, "second_zhuling", "wei", 4);
    second_zhuling->addSkill(new SecondZhanyi);
    second_zhuling->addSkill(new SecondZhanyiEquip);
    related_skills.insertMulti("secondzhanyi", "#secondzhanyi-equip");

    General *guozhao = new General(this, "guozhao", "wei", 3, false);
    guozhao->addSkill(new Pianchong);
    guozhao->addSkill(new PianchongEffect);
    guozhao->addSkill(new Zunwei);
    related_skills.insertMulti("pianchong", "#pianchong-effect");

    General *gongsunkang = new General(this, "gongsunkang", "qun", 4);
    gongsunkang->addSkill(new Juliao);
    gongsunkang->addSkill(new Taomie);
    gongsunkang->addSkill(new TaomieMark);
    related_skills.insertMulti("taomie", "#taomie-mark");

    General *chunyuqiong = new General(this, "chunyuqiong", "qun", 4);
    chunyuqiong->addSkill(new Cangchu);
    chunyuqiong->addSkill(new CangchuKeep);
    chunyuqiong->addSkill(new Liangying);
    chunyuqiong->addSkill(new Shishou);
    related_skills.insertMulti("cangchu", "#cangchu-keep");

    General *fanyufeng = new General(this, "fanyufeng", "qun", 3, false);
    fanyufeng->addSkill(new Bazhan);
    fanyufeng->addSkill(new Jiaoying);
    fanyufeng->addSkill(new JiaoyingMove);
    related_skills.insertMulti("jiaoying", "#jiaoying-move");

    addMetaObject<YujueCard>();
    addMetaObject<SecondYujueCard>();
    addMetaObject<SpNiluanCard>();
    addMetaObject<WeiwuCard>();
    addMetaObject<CixiaoCard>();
    addMetaObject<JieyinghCard>();
    addMetaObject<MinsiCard>();
    addMetaObject<JijingCard>();
    addMetaObject<DaojiCard>();
    addMetaObject<PingjianCard>();
    addMetaObject<ShoufuCard>();
    addMetaObject<ShoufuPutCard>();
    addMetaObject<TenyearSongciCard>();
    addMetaObject<YoulongCard>();
    addMetaObject<SecondZhanyiViewAsBasicCard>();
    addMetaObject<SecondZhanyiCard>();
    addMetaObject<ZunweiCard>();
    addMetaObject<BazhanCard>();

    skills << new TenyearZhixi << new Zhihu << new SecondZhihu << new Panshi;
}

ADD_PACKAGE(SP3)

#include "guo.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "engine.h"
#include "settings.h"
#include "standard-skillcards.h"
#include "util.h"
#include "wrapped-card.h"
#include "room.h"
#include "roomthread.h"

class JinTuishi : public TriggerSkill
{
public:
    JinTuishi() : TriggerSkill("jintuishi")
    {
        events << Appear;
        hide_skill = true;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        if (!room->hasCurrent() || room->getCurrent() == player) return false;
        room->setPlayerMark(player, "&jintuishi-Clear", 1);
        return false;
    }
};

class JinTuishiEffect : public TriggerSkill
{
public:
    JinTuishiEffect() : TriggerSkill("#jintuishi-effect")
    {
        events << EventPhaseChanging;
        hide_skill = true;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (data.value<PhaseChangeStruct>().to != Player::NotActive) return false;
        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            if (player->isDead()) return false;
            if (p->isDead() || p->getMark("&jintuishi-Clear") <= 0) return false;
            QList<ServerPlayer *> targets;
            foreach (ServerPlayer *pl, room->getOtherPlayers(player)) {
                if (player->inMyAttackRange(pl) && player->canSlash(pl, NULL, true))
                    targets << pl;
            }
            if (targets.isEmpty()) return false;
            p->tag["jintuishi_from"] = QVariant::fromValue(player);
            ServerPlayer * target = room->askForPlayerChosen(p, targets, "jintuishi", "@jintuishi-invoke:" + player->objectName(), true);
            p->tag.remove("jintuishi_from");
            if (!target) continue;

            room->doAnimate(1, p->objectName(), target->objectName());
            LogMessage log;
            log.type = "#JinTuishiSlash";
            log.from = p;
            log.arg = "jintuishi";
            log.arg2 = "slash";
            log.to << target;
            room->sendLog(log);
            room->broadcastSkillInvoke(objectName());
            room->notifySkillInvoked(p, objectName());

            if (room->askForUseSlashTo(player, target, "@jintuishi_slash:" + target->objectName(), true, false, true, p)) continue;
            room->damage(DamageStruct("jintuishi", p, player));
        }

        return false;
    }
};

JinChoufaCard::JinChoufaCard()
{
}

bool JinChoufaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void JinChoufaCard::onEffect(const CardEffectStruct &effect) const
{
    if (effect.to->isKongcheng()) return;
    const Card *card = effect.to->getRandomHandCard();
    Room *room = effect.from->getRoom();
    room->showCard(effect.to, card->getEffectiveId());

    room->addPlayerMark(effect.to, "jinchoufa_target");
    foreach (const Card *c, effect.to->getCards("h")) {
        if (c->getTypeId() == card->getTypeId()) continue;
        Slash *slash = new Slash(c->getSuit(), c->getNumber());
        slash->setSkillName("jinchoufa");
        WrappedCard *card = Sanguosha->getWrappedCard(c->getId());
        card->takeOver(slash);
        room->notifyUpdateCard(effect.to, c->getEffectiveId(), card);
    }
}

class JinChoufaVS : public ZeroCardViewAsSkill
{
public:
    JinChoufaVS() : ZeroCardViewAsSkill("jinchoufa")
    {
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("JinChoufaCard");
    }

    const Card *viewAs() const
    {
        return new JinChoufaCard;
    }
};

class JinChoufa : public TriggerSkill
{
public:
    JinChoufa() : TriggerSkill("jinchoufa")
    {
        events << EventPhaseChanging;
        view_as_skill = new JinChoufaVS;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (data.value<PhaseChangeStruct>().to != Player::NotActive) return false;
        if (player->getMark("jinchoufa_target") <= 0) return false;
        room->setPlayerMark(player, "jinchoufa_target", 0);
        room->filterCards(player, player->getCards("he"), true);
        return false;
    }
};

class JinShiren : public TriggerSkill
{
public:
    JinShiren() : TriggerSkill("jinshiren")
    {
        events << Appear;
        hide_skill = true;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        if (!room->hasCurrent() || room->getCurrent() == player) return false;
        if (room->getCurrent()->isKongcheng()) return false;
        if (!player->askForSkillInvoke(this, room->getCurrent())) return false;
        room->broadcastSkillInvoke(objectName());
        JinYanxiCard *yanxi_card = new JinYanxiCard;
        room->useCard(CardUseStruct(yanxi_card, player, room->getCurrent()), true);
        return false;
    }
};

JinYanxiCard::JinYanxiCard()
{
}

bool JinYanxiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void JinYanxiCard::onEffect(const CardEffectStruct &effect) const
{
    if (effect.to->isKongcheng()) return;
    Room *room = effect.from->getRoom();
    int hand_id = effect.to->getRandomHandCardId();
    QList<int> list = room->getNCards(2, false);
    QList<int> new_list;
    room->returnToTopDrawPile(list);
    list << hand_id;
    //qShuffle(list);
    for (int i = 0; i < 3; i++) {
        int id = list.at(qrand() % list.length());
        new_list << id;
        list.removeOne(id);
        if (list.isEmpty()) break;
    }
    if (new_list.isEmpty()) return;

    room->fillAG(new_list, effect.from);
    int id = room->askForAG(effect.from, new_list, false, "jinyanxi");
    room->clearAG(effect.from);

    CardMoveReason reason1(CardMoveReason::S_REASON_UNKNOWN, effect.from->objectName(), "jinyanxi", QString());
    CardMoveReason reason2(CardMoveReason::S_REASON_EXTRACTION, effect.from->objectName(), "jinyanxi", QString());
    if (id == hand_id) {
        QList<CardsMoveStruct> exchangeMove;
        new_list.removeOne(hand_id);
        CardsMoveStruct move1(QList<int>() << hand_id, effect.to,  effect.from, Player::PlaceHand, Player::PlaceHand, reason2);
        CardsMoveStruct move2(new_list, effect.from, Player::PlaceHand, reason1);
        exchangeMove.append(move1);
        exchangeMove.append(move2);
        room->moveCardsAtomic(exchangeMove, false);
    } else {
        DummyCard dummy;
        dummy.addSubcard(id);
        room->obtainCard(effect.from, &dummy, reason1, false);
    }
}

class JinYanxiVS : public ZeroCardViewAsSkill
{
public:
    JinYanxiVS() : ZeroCardViewAsSkill("jinyanxi")
    {
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("JinYanxiCard");
    }

    const Card *viewAs() const
    {
        return new JinYanxiCard;
    }
};

class JinYanxi : public TriggerSkill
{
public:
    JinYanxi() : TriggerSkill("jinyanxi")
    {
        events << CardsMoveOneTime;
        view_as_skill = new JinYanxiVS;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.reason.m_skillName != objectName()) return false;
        if (!move.to || move.to != player || move.to_place != Player::PlaceHand || move.to->getPhase() == Player::NotActive) return false;
        room->ignoreCards(player, move.card_ids);
        return false;
    }
};

JinSanchenCard::JinSanchenCard()
{
}

bool JinSanchenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const
{
    return targets.isEmpty() && to_select->getMark("jinsanchen_target-Clear") <= 0;
}

void JinSanchenCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.from->getRoom();
    room->addPlayerMark(effect.from, "&jinsanchen");
    room->addPlayerMark(effect.to, "jinsanchen_target-Clear");
    effect.to->drawCards(3, "jinsanchen");
    if (effect.to->isDead() || !effect.to->canDiscard(effect.to, "he")) return;
    const Card *card = room->askForDiscard(effect.to, "jinsanchen", 3, 3, false, true, "jinsanchen-discard");
    if (!card) return;
    QList<int> types;
    bool flag = true;
    foreach (int id, card->getSubcards()) {
        int type_id = Sanguosha->getCard(id)->getTypeId();
        if (!types.contains(type_id))
            types << type_id;
        else {
            flag = false;
            break;
        }
    }
    if (!flag) return;
    effect.to->drawCards(1, "jinsanchen");
    room->addPlayerMark(effect.from, "jinsanchen_times-PlayClear");
}

class JinSanchen : public ZeroCardViewAsSkill
{
public:
    JinSanchen() : ZeroCardViewAsSkill("jinsanchen")
    {
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return player->usedTimes("JinSanchenCard") < 1 + player->getMark("jinsanchen_times-PlayClear");
    }

    const Card *viewAs() const
    {
        return new JinSanchenCard;
    }
};

class JinZhaotao : public PhaseChangeSkill
{
public:
    JinZhaotao() : PhaseChangeSkill("jinzhaotao")
    {
        frequency = Wake;
    }

    bool onPhaseChange(ServerPlayer *player) const
    {
        if (player->getPhase() != Player::RoundStart || player->getMark(objectName()) > 0 || player->getMark("&jinsanchen") < 3) return false;
        Room *room = player->getRoom();
        LogMessage log;
        log.type = "#JinZhaotaoWake";
        log.from = player;
        log.arg = QString::number(player->getMark("&jinsanchen"));
        log.arg2 = objectName();
        room->sendLog(log);

        room->broadcastSkillInvoke(objectName());
        room->doSuperLightbox("jin_duyu", "jinzhaotao");
        room->setPlayerMark(player, "jinzhaotao", 1);

        if (room->changeMaxHpForAwakenSkill(player))
            room->handleAcquireDetachSkills(player, "jinpozhu");
        return false;
    }
};

JinPozhuCard::JinPozhuCard()
{
}

bool JinPozhuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const
{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void JinPozhuCard::onEffect(const CardEffectStruct &effect) const
{
    if (effect.to->isKongcheng()) return;
    int hand_id = effect.to->getRandomHandCardId();
    Room *room = effect.from->getRoom();
    room->showCard(effect.to, hand_id);

    if (Sanguosha->getCard(hand_id)->getSuit() != Sanguosha->getCard(subcards.first())->getSuit()) {
        try {
            room->damage(DamageStruct("jinpozhu", effect.from, effect.to));
        }
        catch (TriggerEvent triggerEvent) {
            if (triggerEvent == TurnBroken || triggerEvent == StageChange)
                room->setPlayerMark(effect.from, "jinpozhu_damage-Clear", 0);
            throw triggerEvent;
        }
    }

    if (effect.from->getMark("jinpozhu_damage-Clear") <= 0)
        room->addPlayerMark(effect.from, "jinpozhu_wuxiao-Clear");
    else
        room->setPlayerMark(effect.from, "jinpozhu_damage-Clear", 0);
}

class JinPozhuVS : public OneCardViewAsSkill
{
public:
    JinPozhuVS() : OneCardViewAsSkill("jinpozhu")
    {
        filter_pattern = ".|.|.|hand";
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return player->getMark("jinpozhu_wuxiao-Clear") <= 0;
    }

    const Card *viewAs(const Card *card) const
    {
        JinPozhuCard *c = new JinPozhuCard;
        c->addSubcard(card);
        return c;
    }
};

class JinPozhu : public TriggerSkill
{
public:
    JinPozhu() : TriggerSkill("jinpozhu")
    {
        events << DamageDone;
        view_as_skill = new JinPozhuVS;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.reason != "jinpozhu" || !damage.from || damage.from->isDead()) return false;
        room->addPlayerMark(damage.from, "jinpozhu_damage-Clear");
        return false;
    }
};

class JinZhaoran : public PhaseChangeSkill
{
public:
    JinZhaoran() : PhaseChangeSkill("jinzhaoran")
    {
    }

    bool onPhaseChange(ServerPlayer *player) const
    {
        if (player->getPhase() != Player::Play) return false;
        if (!player->askForSkillInvoke(this)) return false;
        Room *room = player->getRoom();
        room->broadcastSkillInvoke(objectName());
        room->addPlayerMark(player, "HandcardVisible_ALL-PlayClear");
        room->addPlayerMark(player, "jinzhaoran-PlayClear");
        if (!player->isKongcheng())
            room->showAllCards(player);
        return false;
    }
};

class JinZhaoranEffect : public TriggerSkill
{
public:
    JinZhaoranEffect() : TriggerSkill("#jinzhaoran-effect")
    {
        events << BeforeCardsMove;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    int getPriority(TriggerEvent) const
    {
        return 4;
    }

    bool isLastSuit(ServerPlayer *player, const Card *card) const
    {
        foreach (const Card *c, player->getHandcards()) {
            if (c == card) continue;
            if (c->getSuit() == card->getSuit())
                return false;
        }
        return true;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (!move.from || move.from != player || move.from->getPhase() != Player::Play || !move.from_places.contains(Player::PlaceHand)) return false;
        if (player->getMark("jinzhaoran-PlayClear") <= 0) return false;

        for (int i = 0; i< move.card_ids.length(); i++) {
            if (player->isDead()) return false;
            const Card *card = Sanguosha->getCard(move.card_ids.at(i));
            if (move.from_places.at(i) != Player::PlaceHand) continue;
            if (player->getMark("jinzhaoran_suit" + card->getSuitString() + "-PlayClear") > 0) continue;
            if (!isLastSuit(player, card)) continue;

            room->sendCompulsoryTriggerLog(player, "jinzhaoran", true, true);
            room->addPlayerMark(player, "jinzhaoran_suit" + card->getSuitString() + "-PlayClear");

            QList<ServerPlayer *> targets;
            foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                if (player->canDiscard(p, "he"))
                    targets << p;
            }
            if (targets.isEmpty())
                player->drawCards(1, "jinzhaoran");
            else {
                ServerPlayer *target = room->askForPlayerChosen(player, targets, "jinzhaoran", "@jinzhaoran-discard", true);
                if (!target)
                    player->drawCards(1, "jinzhaoran");
                else {
                    room->doAnimate(1, player->objectName(), target->objectName());
                    if (!player->canDiscard(target, "he")) continue;
                    int id = room->askForCardChosen(player, target, "he", "jinzhaoran", false, Card::MethodDiscard);
                    room->throwCard(Sanguosha->getCard(id), target, player);
                }
            }
        }
        return false;
    }
};

GuoPackage::GuoPackage()
    : Package("guo")
{

    General *jin_simazhao = new General(this, "jin_simazhao$", "jin", 3);
    jin_simazhao->addSkill(new JinTuishi);
    jin_simazhao->addSkill(new JinTuishiEffect);
    jin_simazhao->addSkill(new JinChoufa);
    jin_simazhao->addSkill(new JinZhaoran);
    jin_simazhao->addSkill(new JinZhaoranEffect);
    jin_simazhao->addSkill(new Skill("jinchengwu$", Skill::Compulsory));
    related_skills.insertMulti("jintuishi", "#jintuishi-effect");
    related_skills.insertMulti("jinzhaoran", "#jinzhaoran-effect");

    General *jin_wangyuanji = new General(this, "jin_wangyuanji", "jin", 3, false);
    jin_wangyuanji->addSkill(new JinShiren);
    jin_wangyuanji->addSkill(new JinYanxi);

    General *jin_duyu = new General(this, "jin_duyu", "jin", 4);
    jin_duyu->addSkill(new JinSanchen);
    jin_duyu->addSkill(new JinZhaotao);
    jin_duyu->addRelateSkill("jinpozhu");

    addMetaObject<JinChoufaCard>();
    addMetaObject<JinYanxiCard>();
    addMetaObject<JinSanchenCard>();
    addMetaObject<JinPozhuCard>();

    skills << new JinPozhu;
}

ADD_PACKAGE(Guo)

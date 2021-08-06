#include "bei.h"
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

class JinXijue : public TriggerSkill
{
public:
    JinXijue() : TriggerSkill("jinxijue")
    {
        events << GameStart << EventPhaseChanging << DrawNCards;
    }

    int getPriority(TriggerEvent event) const
    {
        if (event == DrawNCards)
            return 1;
        return TriggerSkill::getPriority(event);
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == GameStart) {
            room->sendCompulsoryTriggerLog(player, objectName(), true, true);
            player->gainMark("&jxjjue", 4);
        } else if (event == EventPhaseChanging) {
            if (data.value<PhaseChangeStruct>().to != Player::NotActive) return false;
            int damage = player->getMark("damage_point_round");
            if (damage <= 0) return false;
            room->sendCompulsoryTriggerLog(player, objectName(), true, true);
            player->gainMark("&jxjjue", damage);
        } else {
            if (player->getMark("&jxjjue") <= 0) return false;
            QList<ServerPlayer *> targets;
            foreach(ServerPlayer *p, room->getOtherPlayers(player))
                if (p->getHandcardNum() >= player->getHandcardNum())
                    targets << p;
            int num = qMin(targets.length(), data.toInt());
            foreach(ServerPlayer *p, room->getOtherPlayers(player))
                p->setFlags("-TuxiTarget");

            if (num > 0) {
                room->setPlayerMark(player, "tuxi", num);
                int count = 0;
                if (room->askForUseCard(player, "@@tuxi", "@tuxi-card:::" + QString::number(num))) {
                    player->loseMark("&jxjjue");
                    foreach(ServerPlayer *p, room->getOtherPlayers(player))
                        if (p->hasFlag("TuxiTarget")) count++;
                } else {
                    room->setPlayerMark(player, "tuxi", 0);
                }
                data = data.toInt() - count;
            }
        }
        return false;
    }
};

class JinXijueEffect : public TriggerSkill
{
public:
    JinXijueEffect() : TriggerSkill("#jinxijue-effect")
    {
        events << AfterDrawNCards << EventPhaseStart;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &) const
    {
        if (event == EventPhaseStart) {
            if (player->getPhase() != Player::Finish) return false;
            Room *room = player->getRoom();
            foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                if (player->isDead()) return false;
                if (p->isDead() || !p->hasSkill("jinxijue") || p->getMark("&jxjjue") <= 0 || !p->canDiscard(p, "h")) continue;
                if (room->askForCard(p, ".Basic", "@xiaoguo", QVariant(), "xiaoguo")) {
                    p->loseMark("&jxjjue");
                    room->broadcastSkillInvoke("xiaoguo", 1);
                    if (!room->askForCard(player, ".Equip", "@xiaoguo-discard", QVariant())) {
                        room->broadcastSkillInvoke("xiaoguo", 2);
                        room->damage(DamageStruct("xiaoguo", p, player));
                    } else {
                        room->broadcastSkillInvoke("xiaoguo", 3);
                        if (p->isAlive())
                            p->drawCards(1, "xiaoguo");
                    }
                }
            }
        } else {
            if (player->getMark("tuxi") == 0) return false;
            room->setPlayerMark(player, "tuxi", 0);

            QList<ServerPlayer *> targets;
            foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                if (p->hasFlag("TuxiTarget")) {
                    p->setFlags("-TuxiTarget");
                    targets << p;
                }
            }
            foreach (ServerPlayer *p, targets) {
                if (!player->isAlive())
                    break;
                if (p->isAlive() && !p->isKongcheng()) {
                    int card_id = room->askForCardChosen(player, p, "h", "tuxi");

                    CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, player->objectName());
                    room->obtainCard(player, Sanguosha->getCard(card_id), reason, false);
                }
            }
        }
        return false;
    }
};

class JinBaoQie : public TriggerSkill
{
public:
    JinBaoQie() : TriggerSkill("jinbaoqie")
    {
        events << Appear;
        frequency = Compulsory;
        hide_skill = true;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        room->sendCompulsoryTriggerLog(player, objectName(), true, true);
        QList<int> treasure, list = room->getDrawPile() + room->getDiscardPile();
        foreach (int id, list) {
            if (!Sanguosha->getCard(id)->isKindOf("Treasure")) continue;
            treasure << id;
        }
        if (treasure.isEmpty()) return false;
        int id = treasure.at(qrand() % treasure.length());
        room->obtainCard(player, id);
        if (room->getCardOwner(id) != player || room->getCardPlace(id) != Player::PlaceHand) return false;
        const Card *card = Sanguosha->getCard(id);
        if (!card->isKindOf("Treasure") || !player->canUse(card)) return false;
        if (!player->askForSkillInvoke(this, QString("jinbaoqie_use:%1").arg(card->objectName())), false) return false;
        room->useCard(CardUseStruct(card, player, player));
        return false;
    }
};

JinYishiCard::JinYishiCard()
{
    target_fixed = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}

void JinYishiCard::onUse(Room *, const CardUseStruct &) const
{
}

class JinYishiVS : public OneCardViewAsSkill
{
public:
    JinYishiVS() : OneCardViewAsSkill("jinyishi")
    {
        response_pattern = "@@jinyishi";
        expand_pile = "#jinyishi";
    }

    bool viewFilter(const QList<const Card *> &, const Card *to_select) const
    {
        return Self->getPile("#jinyishi").contains(to_select->getEffectiveId());
    }

    const Card *viewAs(const Card *originalCard) const
    {
        JinYishiCard *c = new JinYishiCard;
        c->addSubcard(originalCard);
        return c;
    }
};

class JinYishi : public TriggerSkill
{
public:
    JinYishi() : TriggerSkill("jinyishi")
    {
        events << CardsMoveOneTime;
        view_as_skill = new JinYishiVS;
    }


    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (!room->hasCurrent() || room->getCurrent()->isDead() || player->getMark("jinyishi-Clear") > 0) return false;
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (!move.from || move.from == player || move.from->getPhase() != Player::Play || !move.from_places.contains(Player::PlaceHand))
            return false;

        if ((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD) {
            int i = 0;
            QVariantList dis;
            foreach (int card_id, move.card_ids) {
                if (move.from_places[i] == Player::PlaceHand && room->getCardPlace(card_id) == Player::DiscardPile)
                    dis << card_id;
                i++;
            }
            if (dis.isEmpty()) return false;
            QList<int> discard = VariantList2IntList(dis);
            player->tag["jinyishi_ids"] = dis;
            room->notifyMoveToPile(player, discard, objectName(), Player::DiscardPile, true);
            const Card *card = room->askForUseCard(player, "@@jinyishi", "@jinyishi:" + move.from->objectName(), -1, Card::MethodNone);
            room->notifyMoveToPile(player, discard, objectName(), Player::DiscardPile, false);
            player->tag.remove("jinyishi_ids");
            if (!card) return false;

            LogMessage log;
            log.type = "#InvokeSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);
            room->broadcastSkillInvoke(objectName());
            room->notifySkillInvoked(player, objectName());

            room->addPlayerMark(player, "jinyishi-Clear");
            room->obtainCard((ServerPlayer *)move.from, card);
            if (player->isAlive()) {
                discard.removeOne(card->getSubcards().first());
                if (discard.isEmpty()) return false;
                DummyCard get(discard);
                room->obtainCard(player, &get);
            }
        }
        return false;
    }
};

JinShiduCard::JinShiduCard()
{
    will_throw = false;
    handling_method = Card::MethodPindian;
}

bool JinShiduCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && Self->canPindian(to_select);
}

void JinShiduCard::onEffect(const CardEffectStruct &effect) const
{
    if (!effect.from->canPindian(effect.to, false)) return;
    bool pindian = effect.from->pindian(effect.to, "jinshidu");
    if (!pindian) return;

    Room *room = effect.from->getRoom();
    DummyCard *handcards = effect.to->wholeHandCards();
    room->obtainCard(effect.from, handcards, false);
    delete handcards;

    if (effect.from->isDead() || effect.to->isDead()) return;
    int give = floor(effect.from->getHandcardNum() / 2);
    if (give <= 0) return;
    const Card *card = room->askForExchange(effect.from, "jinshidu", give, give, false, QString("jinshidu-give:%1::%2").arg(effect.to->objectName())
                                            .arg(QString::number(give)));
    room->giveCard(effect.from, effect.to, card, "jinshidu");
    delete card;
}

class JinShidu : public ZeroCardViewAsSkill
{
public:
    JinShidu() : ZeroCardViewAsSkill("jinshidu")
    {
    }

    const Card *viewAs() const
    {
        return new JinShiduCard;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return player->canPindian() && !player->hasUsed("JinShiduCard");
    }
};

class JinTaoyin : public TriggerSkill
{
public:
    JinTaoyin() : TriggerSkill("jintaoyin")
    {
        events << Appear;
        hide_skill = true;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        if (!room->hasCurrent() || room->getCurrent() == player) return false;
        if (!player->askForSkillInvoke(this, room->getCurrent())) return false;
        room->broadcastSkillInvoke(objectName());
        room->addPlayerMark(room->getCurrent(), "&jintaoyin-Clear");
        room->addMaxCards(room->getCurrent(), -2);
        return false;
    }
};

class JinYimie : public TriggerSkill
{
public:
    JinYimie() : TriggerSkill("jinyimie")
    {
        events << DamageCaused;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (player->getMark("jinyimie-Clear") > 0) return false;
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.to == player || damage.to->isDead()) return false;
        int n = damage.to->getHp() - damage.damage;
        if (n < 0)
            n = 0;
        if (!player->askForSkillInvoke(this, QString("jinyimie:%1::%2").arg(damage.to->objectName()).arg(QString::number(n)))) return false;
        room->addPlayerMark(player, "jinyimie-Clear");
        QString reason;
        if (damage.card)
            reason = damage.card->toString();
        else
            reason = damage.reason;
        damage.to->tag["jinyimie_" + reason] = n + 1;
        damage.to->tag["jinyimie_from"] = QVariant::fromValue(damage.from);
        room->loseHp(player);
        damage.damage += n;
        data = QVariant::fromValue(damage);
        return false;
    }
};

class JinYimieRecover : public TriggerSkill
{
public:
    JinYimieRecover() : TriggerSkill("#jinyimie-recover")
    {
        events << DamageComplete;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        QString reason;
        if (damage.card)
            reason = damage.card->toString();
        else
            reason = damage.reason;
        int n = player->tag["jinyimie_" + reason].toInt() - 1;
        if (n < 0) return false;
        player->tag.remove("jinyimie_" + reason);
        ServerPlayer *from = player->tag["jinyimie_from"].value<ServerPlayer *>();
        player->tag.remove("jinyimie_from");
        int recover = qMin(player->getMaxHp() - player->getHp(), n);
        if (recover <= 0) return false;
        room->recover(player, RecoverStruct(from, NULL, recover));
        return false;
    }
};

class JinTairan : public TriggerSkill
{
public:
    JinTairan() : TriggerSkill("jintairan")
    {
        events << EventPhaseStart << EventPhaseChanging;
        frequency = Compulsory;
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == EventPhaseStart) {
            if (player->getPhase() != Player::Play) return false;
            int mark = player->getMark("&jintairan");
            QVariantList draws = player->tag["jintairan_ids"].toList();
            if (mark > 0 || !draws.isEmpty())
                room->sendCompulsoryTriggerLog(player, objectName(), true, true);

            room->setPlayerMark(player, "&jintairan", 0);
            if (mark > 0)
                room->loseHp(player, mark);
            if (player->isDead()) return false;

            if (draws.isEmpty()) return false;
            player->tag.remove("jintairan_ids");
            DummyCard dummy;
            foreach (int id, player->handCards()) {
                if (!draws.contains(QVariant(id))) continue;
                dummy.addSubcard(id);
            }
            if (dummy.subcardsLength() > 0)
                room->throwCard(&dummy, player);
        } else {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive) return false;
            if (player->getLostHp() > 0 || player->getHandcardNum() < player->getMaxCards())
                room->sendCompulsoryTriggerLog(player, objectName(), true, true);

            if (player->getLostHp() > 0){
                int lost = player->getMaxHp() - player->getHp();
                room->addPlayerMark(player, "&jintairan", lost);
                room->recover(player, RecoverStruct(player, NULL, lost));
            }
            if (player->getHandcardNum() < player->getMaxCards()) {
                QList<int> draws = room->drawCardsList(player, player->getMaxCards() - player->getHandcardNum());
                player->tag["jintairan_ids"] = IntList2VariantList(draws);
            }
        }
        return false;
    }
};

class JinTairanClear : public TriggerSkill
{
public:
    JinTairanClear() : TriggerSkill("#jintairan-clear")
    {
        events << EventLoseSkill;
        frequency = Compulsory;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (data.toString() != "jintairan") return false;
        room->setPlayerMark(player, "&jintairan", 0);
        player->tag.remove("jintairan_ids");
        return false;
    }
};

JinRuilveGiveCard::JinRuilveGiveCard()
{
    m_skillName = "jinruilve_give";
    will_throw = false;
    mute = true;
}

bool JinRuilveGiveCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && Self != to_select && to_select->hasLordSkill("jinruilve") && to_select->getMark("jinruilve-PlayClear") <= 0;
}

void JinRuilveGiveCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.from->getRoom();
    room->addPlayerMark(effect.to, "jinruilve-PlayClear");
    if (effect.to->isWeidi()) {
        room->broadcastSkillInvoke("weidi");
        room->notifySkillInvoked(effect.to, "weidi");
    }
    else {
        room->broadcastSkillInvoke("jinruilve");
        room->notifySkillInvoked(effect.to, "jinruilve");
    }
    room->giveCard(effect.from, effect.to, this, "jinruilve", true);
}

class JinRuilveGive : public OneCardViewAsSkill
{
public:
    JinRuilveGive() : OneCardViewAsSkill("jinruilve_give")
    {
        attached_lord_skill = true;
    }

    bool viewFilter(const Card *to_select) const
    {
        return to_select->isKindOf("Slash") || (to_select->isDamageCard() && to_select->isKindOf("TrickCard"));
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return hasTarget(player) && player->getKingdom() == "jin";
    }

    bool hasTarget(const Player *player) const
    {
        QList<const Player *> as = player->getAliveSiblings();
        foreach (const Player *p, as) {
            if (p->hasLordSkill("jinruilve") && p->getMark("jinruilve-PlayClear") <= 0)
                return true;
        }
        return false;
    }

    const Card *viewAs(const Card *card) const
    {
        JinRuilveGiveCard *c = new JinRuilveGiveCard;
        c->addSubcard(card);
        return c;
    }
};

class JinRuilve : public TriggerSkill
{
public:
    JinRuilve() : TriggerSkill("jinruilve$")
    {
        events << GameStart << EventAcquireSkill;
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if ((event == GameStart && player->isLord()) || (event == EventAcquireSkill && data.toString() == objectName())) {
            QList<ServerPlayer *> lords;
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->hasLordSkill(this))
                    lords << p;
            }
            if (lords.isEmpty()) return false;

            QList<ServerPlayer *> players;
            if (lords.length() > 1)
                players = room->getAlivePlayers();
            else
                players = room->getOtherPlayers(lords.first());
            foreach (ServerPlayer *p, players) {
                if (!p->hasSkill("jinruilve_give"))
                    room->attachSkillToPlayer(p, "jinruilve_give");
            }
        }
        return false;
    }
};

class JinHuirong : public TriggerSkill
{
public:
    JinHuirong() : TriggerSkill("jinhuirong")
    {
        events << Appear;
        frequency = Compulsory;
        hide_skill = true;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName(), "@jinhuirong-invoke", false, true);
        room->broadcastSkillInvoke(objectName());
        if (target->isDead()) return false;
        if (target->getHandcardNum() > target->getHp())
            room->askForDiscard(target, objectName(), target->getHandcardNum() - target->getHp(), target->getHandcardNum() - target->getHp());
        else if (target->getHandcardNum() < target->getHp()) {
            int num = qMin(5, target->getHp()) - target->getHandcardNum();
            target->drawCards(num, objectName());
        }
        return false;
    }
};

class JinCiwei : public TriggerSkill
{
public:
    JinCiwei() : TriggerSkill("jinciwei")
    {
        events << CardUsed << CardResponded << JinkEffect << NullificationEffect;
        global = true;
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == JinkEffect || event == NullificationEffect) {
            if (!player->hasFlag("jinciwei_wuxiao")) return false;
            room->setPlayerFlag(player, "-jinciwei_wuxiao");
            return true;
        }

        if (player->getPhase() == Player::NotActive) return false;
        const Card *card = NULL;
        if (event == CardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.card->isKindOf("SkillCard"))
                room->addPlayerMark(player, "jinciwei_use_time-Clear");
            if (use.card->isKindOf("BasicCard") || use.card->isNDTrick())
                card = use.card;
        } else {
            CardResponseStruct res = data.value<CardResponseStruct>();
            if (!res.m_isUse) return false;
            if (!res.m_card->isKindOf("SkillCard"))
                room->addPlayerMark(player, "jinciwei_use_time-Clear");
            if (res.m_card->isKindOf("BasicCard") || res.m_card->isNDTrick())
                card = res.m_card;
        }
        if (!card || card->isKindOf("SkillCard") || !(card->isKindOf("BasicCard") || card->isNDTrick())) return false;

        if (player->getMark("jinciwei_use_time-Clear") != 2) return false;
        foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
            if (player->isDead()) return false;
            if (p->isDead() || !p->hasSkill(this) || !p->canDiscard(p, "he")) continue;
            if (!room->askForCard(p, "..", QString("@jinciwei-discard:%1::%2").arg(player->objectName()).arg(card->objectName()), data, objectName()))
                continue;
            room->broadcastSkillInvoke(objectName());
            if (event == CardUsed) {
                CardUseStruct use = data.value<CardUseStruct>();
                if (use.card->isKindOf("Nullification"))
                    room->setPlayerFlag(player, "jinciwei_wuxiao");
                else {
                    use.nullified_list << "_ALL_TARGETS";
                    data = QVariant::fromValue(use);
                }

            } else
                room->setPlayerFlag(player, "jinciwei_wuxiao");
        }

        return false;
    }
};

class JinCaiyuan : public TriggerSkill
{
public:
    JinCaiyuan() : TriggerSkill("jincaiyuan")
    {
        events << HpChanged << EventPhaseStart << EventPhaseChanging;
        global = true;
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == HpChanged)
            room->setPlayerMark(player, "jincaiyuan_hpchanged", 1);
        else if (event == EventPhaseStart) {
            if (player->getPhase() != Player::Finish) return false;
            if (player->getMark("jincaiyuan_hpchanged") > 0) return false;
            if (!player->hasSkill(this) || !player->tag["FengjiLastTurn"].toBool()) return false;
            room->sendCompulsoryTriggerLog(player, objectName(), true, true);
            player->drawCards(2, objectName());
        } else {
            if (data.value<PhaseChangeStruct>().to != Player::NotActive) return false;
            room->setPlayerMark(player, "jincaiyuan_hpchanged", 0);
        }
        return false;
    }
};

BeiPackage::BeiPackage()
    : Package("bei")
{
    General *jin_zhanghuyuechen = new General(this, "jin_zhanghuyuechen", "jin", 4);
    jin_zhanghuyuechen->addSkill(new JinXijue);
    jin_zhanghuyuechen->addSkill(new JinXijueEffect);
    related_skills.insertMulti("jinxijue", "#jinxijue-effect");

    General *jin_xiahouhui = new General(this, "jin_xiahouhui", "jin", 3, false);
    jin_xiahouhui->addSkill(new JinBaoQie);
    jin_xiahouhui->addSkill(new JinYishi);
    jin_xiahouhui->addSkill(new JinShidu);

    General *jin_simashi = new General(this, "jin_simashi$", "jin", 4, true, false, false, 3);
    jin_simashi->addSkill(new JinTaoyin);
    jin_simashi->addSkill(new JinYimie);
    jin_simashi->addSkill(new JinYimieRecover);
    jin_simashi->addSkill(new JinTairan);
    jin_simashi->addSkill(new JinTairanClear);
    jin_simashi->addSkill(new JinRuilve);
    related_skills.insertMulti("jinyimie", "#jinyimie-recover");
    related_skills.insertMulti("jintairan", "#jintairan-clear");

    General *jin_yanghuiyu = new General(this, "jin_yanghuiyu", "jin", 3, false);
    jin_yanghuiyu->addSkill(new JinHuirong);
    jin_yanghuiyu->addSkill(new JinCiwei);
    jin_yanghuiyu->addSkill(new JinCaiyuan);

    addMetaObject<JinYishiCard>();
    addMetaObject<JinShiduCard>();
    addMetaObject<JinRuilveGiveCard>();

    skills << new JinRuilveGive;
}

ADD_PACKAGE(Bei)

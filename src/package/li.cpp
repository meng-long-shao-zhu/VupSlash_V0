#include "li.h"
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

class JinBuchen : public TriggerSkill
{
public:
    JinBuchen() : TriggerSkill("jinbuchen")
    {
        events << Appear;
        hide_skill = true;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        if (!room->hasCurrent() || room->getCurrent() == player || room->getCurrent()->isNude()) return false;
        if (!player->askForSkillInvoke(this, room->getCurrent())) return false;
        room->broadcastSkillInvoke(objectName());
        int card_id = room->askForCardChosen(player, room->getCurrent(), "he", "jinbuchen");
        CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, player->objectName());
        room->obtainCard(player, Sanguosha->getCard(card_id),
            reason, room->getCardPlace(card_id) != Player::PlaceHand);
        return false;
    }
};

JinYingshiCard::JinYingshiCard()
{
    target_fixed = true;
}

void JinYingshiCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    int maxhp = card_use.from->getMaxHp();
    if (maxhp <= 0) return;
    QList<int> list = room->getNCards(maxhp, false);
    room->returnToTopDrawPile(list);
    room->fillAG(list, card_use.from);
    room->askForAG(card_use.from, list, true, "jinyingshi");
    room->clearAG(card_use.from);
}

class JinYingshi : public ZeroCardViewAsSkill
{
public:
    JinYingshi() : ZeroCardViewAsSkill("jinyingshi")
    {
        frequency = Compulsory;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return player->getMaxHp() > 0;
    }

    const Card *viewAs() const
    {
        return new JinYingshiCard;
    }
};

JinXiongzhiCard::JinXiongzhiCard()
{
    target_fixed = true;
}

void JinXiongzhiCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    room->removePlayerMark(source, "@jinxiongzhiMark");
    room->doSuperLightbox("jin_simayi", "jinxiongzhi");

    while (true) {
        if (source->isDead()) return;

        QList<int> list = room->getNCards(1, false);
        room->returnToTopDrawPile(list);
        const Card *card = Sanguosha->getCard(list.first());

        room->notifyMoveToPile(source, list, "jinxiongzhi", Player::DrawPile, true);

        LogMessage log;
        log.type = "$ViewDrawPile";
        log.from = source;
        log.card_str = card->toString();
        room->sendLog(log, source);

        if (!source->canUse(card)) {
            room->notifyMoveToPile(source, list, "jinxiongzhi", Player::DrawPile, false);
            room->fillAG(list, source);
            room->askForAG(source, list, true, "jinxiongzhi");
            room->clearAG(source);
            break;
        } else {
            if (card->targetFixed()) {
                room->useCard(CardUseStruct(card, source, source), true);
            } else {
                QList<ServerPlayer *> targets;
                room->setPlayerMark(source, "jinxiongzhi_id-PlayClear", list.first() + 1);
                if (room->askForUseCard(source, "@@jinxiongzhi!", "@jinxiongzhi:" + card->objectName())) {
                    room->notifyMoveToPile(source, list, "jinxiongzhi", Player::DrawPile, false);
                    foreach (ServerPlayer *p, room->getAlivePlayers()) {
                        if (!p->hasFlag("jinxiongzhi_target")) continue;
                        room->setPlayerFlag(p, "-jinxiongzhi_target");
                        targets << p;
                    }
                } else {
                    room->notifyMoveToPile(source, list, "jinxiongzhi", Player::DrawPile, false);
                    foreach (ServerPlayer *p, room->getAlivePlayers()) {
                        if (source->canUse(card, QList<ServerPlayer *>() << p))
                            targets << p;
                    }
                }
                if (targets.isEmpty()) break;
                room->useCard(CardUseStruct(card, source, targets), true);
            }
        }
    }
}

JinXiongzhiUseCard::JinXiongzhiUseCard()
{
    m_skillName = "jinxiongzhi";
    handling_method = Card::MethodUse;
}

bool JinXiongzhiUseCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    int id = Self->getMark("jinxiongzhi_id-PlayClear") - 1;
    if (id < 0) return false;
    const Card *card = Sanguosha->getCard(id);
    return card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card);
}

bool JinXiongzhiUseCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    int id = Self->getMark("jinxiongzhi_id-PlayClear") - 1;
    if (id < 0) return false;
    const Card *card = Sanguosha->getCard(id);
    return card->targetsFeasible(targets, Self);
}

void JinXiongzhiUseCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    foreach (ServerPlayer *p, card_use.to)
        room->setPlayerFlag(p, "jinxiongzhi_target");
}

class JinXiongzhi : public ViewAsSkill
{
public:
    JinXiongzhi() : ViewAsSkill("jinxiongzhi")
    {
        response_pattern = "@@jinxiongzhi!";
        frequency = Limited;
        limit_mark = "@jinxiongzhiMark";
        expand_pile = "#jinxiongzhi";
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return player->getMark("@jinxiongzhiMark") > 0;
    }

    bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        if (Sanguosha->getCurrentCardUsePattern() == "@@jinxiongzhi!" && selected.isEmpty())
            return to_select->getId() == Self->getMark("jinxiongzhi_id-PlayClear") - 1;
        return false;
    }

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (Sanguosha->getCurrentCardUsePattern() == "@@jinxiongzhi!" && cards.length() == 1) {
            JinXiongzhiUseCard *c = new JinXiongzhiUseCard;
            c->addSubcards(cards);
            return c;
        }
        if (cards.isEmpty())
            return new JinXiongzhiCard;
        return NULL;
    }
};

class JinQuanbian : public TriggerSkill
{
public:
    JinQuanbian() : TriggerSkill("jinquanbian")
    {
        events << CardUsed << CardResponded << EventPhaseEnd << EventLoseSkill << MaxHpChanged << EventAcquireSkill;
        global = true;
    }

    int getPriority(TriggerEvent triggerEvent) const
    {
        if (triggerEvent == EventPhaseEnd)
            return 0;
        return TriggerSkill::getPriority(triggerEvent);
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (player->getPhase() != Player::Play) return false;
        if (event == EventPhaseEnd) {
            if (player->getMark("jinquanbian_limit-PlayClear") <= 0) return false;
            room->setPlayerMark(player, "jinquanbian_limit-PlayClear", 0);
            room->removePlayerCardLimitation(player, "use", ".|.|.|hand$1");
        } else if (event == EventLoseSkill) {
            if (data.toString() != objectName() || player->hasSkill(this)) return false;
            if (player->getMark("jinquanbian_limit-PlayClear") <= 0) return false;
            room->setPlayerMark(player, "jinquanbian_limit-PlayClear", 0);
            room->removePlayerCardLimitation(player, "use", ".|.|.|hand$1");
        } else if (event == EventAcquireSkill) {
            if (data.toString() != objectName() || !player->hasSkill(this)) return false;
            if (player->getMark("jinquanbian_used-PlayClear") < player->getMaxHp()) return false;
            room->addPlayerMark(player, "jinquanbian_limit-PlayClear");
            room->setPlayerCardLimitation(player, "use", ".|.|.|hand", true);
        } else if (event == MaxHpChanged) {
            if (player->getMark("jinquanbian_used-PlayClear") < player->getMaxHp()) {
                room->setPlayerMark(player, "jinquanbian_limit-PlayClear", 0);
                room->removePlayerCardLimitation(player, "use", ".|.|.|hand$1");
            } else {
                room->addPlayerMark(player, "jinquanbian_limit-PlayClear");
                room->setPlayerCardLimitation(player, "use", ".|.|.|hand", true);
            }
        } else {
            const Card *card;
            if (event == CardUsed) {
                CardUseStruct use = data.value<CardUseStruct>();
                if (!use.m_isHandcard || use.card->isKindOf("SkillCard")) return false;
                card = use.card;
                room->addPlayerMark(player, "jinquanbian_used-PlayClear");
                if (player->getMark("jinquanbian_used-PlayClear") >= player->getMaxHp() && player->hasSkill(this)) {
                    room->addPlayerMark(player, "jinquanbian_limit-PlayClear");
                    room->setPlayerCardLimitation(player, "use", ".|.|.|hand", true);
                }
            } else {
                CardResponseStruct res = data.value<CardResponseStruct>();
                if (!res.m_isHandcard || res.m_card->isKindOf("SkillCard")) return false;
                card = res.m_card;
                if (res.m_isUse) {
                    room->addPlayerMark(player, "jinquanbian_used-PlayClear");
                    if (player->getMark("jinquanbian_used-PlayClear") >= player->getMaxHp() && player->hasSkill(this)) {
                        room->addPlayerMark(player, "jinquanbian_limit-PlayClear");
                        room->setPlayerCardLimitation(player, "use", ".|.|.|hand", true);
                    }
                }
            }
            if (!card || card->isKindOf("SkillCard")) return false;

            QString suitstring = card->getSuitString();
            if (suitstring == "no_suit_black" || suitstring == "no_suit_red")
                suitstring = "no_suit";
            if (player->getMark("jinquanbian_" + suitstring + "-PlayClear") > 0) return false;
            room->addPlayerMark(player, "jinquanbian_" + suitstring + "-PlayClear");

            if (!player->hasSkill(this) || player->getMaxHp() <= 0 || !player->askForSkillInvoke(this)) return false;
            room->broadcastSkillInvoke(objectName());
            QList<int> list = room->getNCards(player->getMaxHp(), false);
            //room->returnToTopDrawPile(list);  guanxing会放回去，这里不能return回去
            QList<int> enabled, disabled;
            foreach (int id, list) {
                QString suitstr = Sanguosha->getCard(id)->getSuitString();
                if (suitstr == "no_suit_black" || suitstr == "no_suit_red")
                    suitstr = "no_suit";
                if (suitstr == suitstring)
                    disabled << id;
                else
                    enabled << id;
            }
            if (enabled.isEmpty()) {
                room->fillAG(list, player);
                room->askForAG(player, list, true, objectName());
                room->clearAG(player);
                room->askForGuanxing(player, list, Room::GuanxingUpOnly);
                return false;
            }
            room->fillAG(list, player, disabled);
            int id = room->askForAG(player, enabled, false, objectName());
            room->clearAG(player);
            room->obtainCard(player, id, true);
            list.removeOne(id);
            if (player->isDead()) {
                room->returnToTopDrawPile(list);
                return false;
            }
            room->askForGuanxing(player, list, Room::GuanxingUpOnly);
        }
        return false;
    }
};

class JinHuishi : public DrawCardsSkill
{
public:
    JinHuishi() : DrawCardsSkill("jinhuishi")
    {
    }

    int getDrawNum(ServerPlayer *player, int n) const
    {
        Room *room = player->getRoom();
        int draw_num = room->getDrawPile().length();
        if (draw_num >= 10)
            draw_num = draw_num % 10;
        if (!player->askForSkillInvoke(this, QString("jinhuishi_invoke:%1").arg(QString::number(draw_num)))) return n;
        room->broadcastSkillInvoke(objectName());
        if (draw_num <= 0) return -n;
        int get_num = floor(draw_num / 2);
        if (get_num <= 0) {
            QList<int> list = room->getNCards(draw_num, false);
            room->returnToEndDrawPile(list);
            LogMessage log;
            log.type = "$ViewDrawPile";
            log.from = player;
            log.card_str = IntList2StringList(list).join("+");
            room->sendLog(log, player);
            room->fillAG(list, player);
            room->askForAG(player, list, true, objectName());
            room->clearAG(player);
            return -n;
        }

        QList<int> list = room->getNCards(draw_num, false);
        LogMessage log;
        log.type = "$ViewDrawPile";
        log.from = player;
        log.card_str = IntList2StringList(list).join("+");
        room->sendLog(log, player);

        QList<int> enabled = list, disabled;
        while (disabled.length() < get_num) {
            if (player->isDead()) break;
            room->fillAG(list, player, disabled);
            int id = room->askForAG(player, enabled, false, objectName());
            room->clearAG(player);
            enabled.removeOne(id);
            disabled << id;
            if (enabled.isEmpty()) break;
        }
        if (player->isAlive()) {
            DummyCard get(disabled);
            room->obtainCard(player, &get, false);
            room->returnToEndDrawPile(enabled);
        } else
            room->returnToTopDrawPile(list);

        return -n;
    }
};

JinQinglengCard::JinQinglengCard()
{
    will_throw = false;
    target_fixed = true;
}

void JinQinglengCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    QString name = card_use.from->property("jinqingleng_now_target").toString();
    if (name.isEmpty()) return;
    ServerPlayer *target = room->findChild<ServerPlayer *>(name);
    if (!target || target->isDead()) return;

    const Card *card = Sanguosha->getCard(subcards.first());
    Slash *slash = new Slash(card->getSuit(), card->getNumber());
    slash->addSubcard(card);
    slash->deleteLater();
    slash->setSkillName("jinqingleng");
    if (!card_use.from->canSlash(target, slash, false)) return;

    room->useCard(CardUseStruct(slash, card_use.from, target));
}

class JinQinglengVS : public OneCardViewAsSkill
{
public:
    JinQinglengVS() : OneCardViewAsSkill("jinqingleng")
    {
        response_pattern = "@@jinqingleng";
        response_or_use = true;
    }

    bool viewFilter(const Card *to_select) const
    {
        QString name = Self->property("jinqingleng_now_target").toString();
        if (name.isEmpty()) return false;
        //Player *player = Self->findChild<Player *>(name);
        const Player *player = NULL;
        foreach (const Player *p, Self->getAliveSiblings()) {
            if (p->objectName() == name) {
                player = p;
                break;
            }
        }
        if (player == NULL || player->isDead()) return false;
        Slash *slash = new Slash(to_select->getSuit(), to_select->getNumber());
        slash->addSubcard(to_select);
        slash->deleteLater();
        slash->setSkillName("jinqingleng");
        return Self->canSlash(player, slash, false);
    }

    const Card *viewAs(const Card *originalCard) const
    {
        JinQinglengCard *c = new JinQinglengCard;
        c->addSubcard(originalCard);
        return c;
    }
};

class JinQingleng : public TriggerSkill
{
public:
    JinQingleng() : TriggerSkill("jinqingleng")
    {
        events << CardUsed << EventPhaseChanging << DamageCaused;
        view_as_skill = new JinQinglengVS;
    }

    bool triggerable(const ServerPlayer *target) const
    {
        return target != NULL && target->isAlive();
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == CardUsed) {
            if (!player->hasSkill(this)) return false;
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("SkillCard") || use.card->getSkillName() != objectName()) return false;
            QStringList target_names = player->property("jinqingleng_targets").toStringList();
            int num = 0;
            foreach (ServerPlayer *p, use.to) {
                if (target_names.contains(p->objectName())) continue;
                num++;
                target_names << p->objectName();
            }
            room->setPlayerProperty(player, "jinqingleng_targets", target_names);
            if (num > 0) {
                room->sendCompulsoryTriggerLog(player, objectName(), true, true);
                player->drawCards(num, objectName());
            }
        } else if (event == EventPhaseChanging) {
            if (data.value<PhaseChangeStruct>().to != Player::NotActive) return false;
            foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                if (player->isDead()) return false;
                if (p->isDead() || !p->hasSkill(this)) continue;
                int num = player->getHp() + player->getHandcardNum();
                int draw_num = room->getDrawPile().length();
                if (draw_num >= 10)
                    draw_num = draw_num % 10;
                if (num < draw_num) return false;
                room->setPlayerProperty(p, "jinqingleng_now_target", player->objectName());
                room->askForUseCard(p, "@@jinqingleng", "@jinqingleng:" + player->objectName());
                room->setPlayerProperty(p, "jinqingleng_now_target", QString());
            }
        } else {
            DamageStruct damage = data.value<DamageStruct>();
            if (!damage.card || !damage.card->isKindOf("Slash") || damage.card->getSkillName() != objectName()) return false;
            if (damage.to->isDead() || !player->canDiscard(damage.to, "he")) return false;
            if (!player->askForSkillInvoke(this, QString("prevent:%1").arg(damage.to->objectName()))) return false;
            room->broadcastSkillInvoke(objectName());
            int id = room->askForCardChosen(player, damage.to, "he", objectName(), false, Card::MethodDiscard);
            room->throwCard(Sanguosha->getCard(id), damage.to, player);
            if (player->isAlive() && damage.to->isAlive() && player->canDiscard(damage.to, "he")) {
                id = room->askForCardChosen(player, damage.to, "he", objectName(), false, Card::MethodDiscard);
                room->throwCard(Sanguosha->getCard(id), damage.to, player);
            }
            return true;
        }
        return false;
    }
};

class JinXuanmu : public TriggerSkill
{
public:
    JinXuanmu() : TriggerSkill("jinxuanmu")
    {
        events << Appear << DamageInflicted;
        frequency = Compulsory;
        hide_skill = true;
    }

    bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == Appear) {
            if (!room->hasCurrent() || room->getCurrent() == player) return false;
            room->setPlayerMark(player, "&jinxuanmu-Clear", 1);
        } else {
            if (player->getMark("&jinxuanmu-Clear") <= 0) return false;
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.damage <= 0) return false;
            LogMessage log;
            log.type = "#MobilejinjiuPrevent";
            log.from = player;
            log.arg = objectName();
            log.arg2 = QString::number(damage.damage);
            room->sendLog(log);
            room->notifySkillInvoked(player, objectName());
            room->broadcastSkillInvoke(objectName());
            return true;
        }
        return false;
    }
};

LiPackage::LiPackage()
    : Package("li")
{
    new General(this, "yinni_hide", "jin", 1, true, true, true);

    General *jin_simayi = new General(this, "jin_simayi", "jin", 3);
    jin_simayi->addSkill(new JinBuchen);
    jin_simayi->addSkill(new JinYingshi);
    jin_simayi->addSkill(new JinXiongzhi);
    jin_simayi->addSkill(new JinQuanbian);

    General *jin_zhangchunhua = new General(this, "jin_zhangchunhua", "jin", 3, false);
    jin_zhangchunhua->addSkill(new JinHuishi);
    jin_zhangchunhua->addSkill(new JinQingleng);
    jin_zhangchunhua->addSkill(new JinXuanmu);

    addMetaObject<JinYingshiCard>();
    addMetaObject<JinXiongzhiCard>();
    addMetaObject<JinXiongzhiUseCard>();
    addMetaObject<JinQinglengCard>();
}

ADD_PACKAGE(Li)

#include "couple-scenario.h"
#include "skill.h"
#include "engine.h"
#include "room.h"
#include "roomthread.h"
#include "util.h"

QStringList start_cps;

class CoupleScenarioRule : public ScenarioRule
{
public:
    CoupleScenarioRule(Scenario *scenario)
        : ScenarioRule(scenario)
    {
        events << GameReady << GameOverJudge << BuryVictim;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        const CoupleScenario *scenario = qobject_cast<const CoupleScenario *>(parent());

        switch (triggerEvent) {
        case GameReady: {
            if (player != NULL) return false;
            foreach (ServerPlayer *player, room->getPlayers()) {
                if (player->isLord()) {
                    room->acquireSkill(player, "xuanfei");
                    continue;
                } else {
                    /*QMap<QString, QString> OH_map = scenario->getOriginalMap(true);
                    QMap<QString, QString> OW_map = scenario->getOriginalMap(false);
                    QMap<QString, QStringList> H_map = scenario->getMap(true);
                    QMap<QString, QStringList> W_map = scenario->getMap(false);
                    if (OH_map.contains(player->getGeneralName())) {
                        QStringList h_list = W_map.value(OH_map.value(player->getGeneralName()));
                        if (h_list.length() > 1) {
                            if (player->askForSkillInvoke("reselect")) {
                                h_list.removeOne(player->getGeneralName());
                                QString general_name = room->askForGeneral(player, h_list);
                                room->changeHero(player, general_name, true, false);
                            }
                        }
                    } else if (OW_map.contains(player->getGeneralName())) {
                        QStringList w_list = H_map.value(OW_map.value(player->getGeneralName()));
                        if (w_list.length() > 1) {
                            if (player->askForSkillInvoke("reselect")) {
                                w_list.removeOne(player->getGeneralName());
                                QString general_name = room->askForGeneral(player, w_list);
                                room->changeHero(player, general_name, true, false);
                            }
                        }
                    }*/
                }
            }
            scenario->marryAll(room);
            room->setTag("SkipNormalDeathProcess", true);
            break;
        }
        case GameOverJudge: {
            if (player) {
                ServerPlayer *ex_pair = scenario->getSpouse(player);
                if (ex_pair && ex_pair->isAlive()) {
                    player->setProperty("cp_before_died", ex_pair->objectName());
                }
            }
            if (player->isLord()) {
                ServerPlayer *ex_pair = scenario->getSpouse(player);
                if (ex_pair) {
                    room->setPlayerMark(player, "&CP+"+ex_pair->getGeneralName()+"+-Keep", 0);
                    room->setPlayerMark(ex_pair, "&CP+"+player->getGeneralName()+"+-Keep", 0);
                }
                scenario->marryAll(room);
            } else if (player->getRoleEnum() == Player::Loyalist) {
                room->setPlayerProperty(player, "role", "renegade");

                QList<ServerPlayer *> players = room->getAllPlayers();
                QList<ServerPlayer *> widows;
                foreach (ServerPlayer *player, players) {
                    if (scenario->isWidow(player))
                        widows << player;
                }

                if (!widows.isEmpty() && room->getLord()->isAlive()) {
                    room->sendCompulsoryTriggerLog(room->getLord(), "xuanfei");
                    ServerPlayer *new_wife = room->askForPlayerChosen(room->getLord(), widows, "remarry", "#xuanfei", false, false);
                    if (new_wife) {
                        room->doAnimate(1, room->getLord()->objectName(), new_wife->objectName());
                        scenario->remarry(room->getLord(), new_wife);
                    }
                }
            } else {
                ServerPlayer *loyalist = NULL;
                foreach (ServerPlayer *player, room->getAlivePlayers()) {
                    if (player->getRoleEnum() == Player::Loyalist) {
                        loyalist = player;
                        break;
                    }
                }
                ServerPlayer *widow = scenario->getSpouse(player);
                if (widow && widow->isAlive() && widow->isFemale() && room->getLord()->isAlive() && loyalist == NULL) {
                    room->sendCompulsoryTriggerLog(room->getLord(), "xuanfei");
                    room->doAnimate(1, room->getLord()->objectName(), widow->objectName());
                    scenario->remarry(room->getLord(), widow);
                }
            }

            QList<ServerPlayer *> players = room->getAlivePlayers();
            if (players.length() == 1) {
                ServerPlayer *survivor = players.first();
                ServerPlayer *spouse = scenario->getSpouse(survivor);
                if (spouse)
                    room->gameOver(QString("%1+%2").arg(survivor->objectName()).arg(spouse->objectName()));
                else
                    room->gameOver(survivor->objectName());

                return true;
            } else if (players.length() == 2) {
                ServerPlayer *first = players.at(0);
                ServerPlayer *second = players.at(1);
                if (scenario->getSpouse(first) == second) {
                    room->gameOver(QString("%1+%2").arg(first->objectName()).arg(second->objectName()));
                    return true;
                }
            }

            return true;
        }

        case BuryVictim: {
            DeathStruct death = data.value<DeathStruct>();
            player->bury();

            //find ex_pair by player property
            ServerPlayer *ex_pair;
            QString ex_pair_objname = player->property("cp_before_died").toString();
            if (ex_pair_objname != "") {
                foreach (ServerPlayer *p, room->getAlivePlayers()) {
                    if (p->objectName() == ex_pair_objname) {
                        ex_pair = p;
                        break;
                    }
                }
            }

            // reward and punishment
            if (death.damage && death.damage->from) {
                ServerPlayer *killer = death.damage->from;
                if (killer && killer != player){
                    if (killer->objectName() == ex_pair_objname)
                        killer->throwAllHandCardsAndEquips();
                    else
                        killer->drawCards(2, "kill");
                }
            }

            if (ex_pair && ex_pair->isAlive()) {
                LogMessage log;
                log.type = "#cp_died";
                log.from = ex_pair;
                log.to << player;
                room->sendLog(log);

                if (ex_pair->getCardCount(true) < 2 || !room->askForDiscard(ex_pair, "cp_died", 2, 2, true, true, "@cp_died"))
                    room->loseHp(ex_pair, 1);
            }
            player->setProperty("cp_before_died", "");

            break;
        }
        default:
            break;
        }

        return false;
    }
};

CoupleScenario::CoupleScenario()
    : Scenario("couple")
{
    lua_State *lua = Sanguosha->getLuaState();
    QStringList lords_list = GetConfigFromLuaState(lua, "couple_lord").toString().split("|");
    lord = lords_list.at(qrand() % lords_list.length());
    loadCoupleMap();

    rule = new CoupleScenarioRule(this);
}

void CoupleScenario::loadCoupleMap()
{
    QStringList couple_list = GetConfigFromLuaState(Sanguosha->getLuaState(), "couple_couples").toStringList();
    foreach (QString couple, couple_list) {
        /*QStringList husbands = couple.split("+").first().split("|");
        QStringList wifes = couple.split("+").last().split("|");
        foreach(QString husband, husbands)
            husband_map[husband] = wifes;
        original_husband_map[husbands.first()] = wifes.first();
        foreach(QString wife, wifes)
            wife_map[wife] = husbands;
        original_wife_map[wifes.first()] = husbands.first();*/

        QStringList cp_group = couple.split(",");
        foreach(QString cp1, cp_group)
            foreach(QString cp2, cp_group)
                if (cp1 != cp2) {
                    if (!cp_map[cp1].contains(cp2))
                        cp_map[cp1].append(cp2);
                    if (!cp_map[cp2].contains(cp1))
                        cp_map[cp2].append(cp1);
                }
    }
}

void CoupleScenario::marryAll(Room *room) const
{
    /*foreach (QString husband_name, husband_map.keys()) {
        ServerPlayer *husband = room->findPlayer(husband_name, true);
        if (husband == NULL)
            continue;

        QStringList wife_names = husband_map.value(husband_name, QStringList());
        if (!wife_names.isEmpty()) {
            foreach (QString wife_name, wife_names) {
                ServerPlayer *wife = room->findPlayer(wife_name, true);
                if (wife != NULL) {
                    marry(husband, wife);
                    break;
                }
            }
        }
    }*/
    foreach (QString cp_names, start_cps){
        QString from_name = cp_names.split("+").first();
        ServerPlayer *from = room->findPlayer(from_name, true);
        if (from == NULL)
            continue;
        QString to_name = cp_names.split("+").last();
        ServerPlayer *to = room->findPlayer(to_name, true);
        if (to != NULL){
            marry(from, to);
        }
    }
}

void CoupleScenario::setSpouse(ServerPlayer *player, ServerPlayer *spouse) const
{
    if (spouse)
        player->tag["spouse"] = QVariant::fromValue(spouse);
    else
        player->tag.remove("spouse");
}

void CoupleScenario::marry(ServerPlayer *husband, ServerPlayer *wife) const
{
    if (getSpouse(husband) == wife)
        return;
    Room *room = husband->getRoom();

    LogMessage log;
    log.type = "#Marry";
    log.from = husband;
    log.to << wife;
    room->sendLog(log);

    room->setPlayerMark(husband, "&CP+"+wife->getGeneralName()+"+-Keep", 1);
    room->setPlayerMark(wife, "&CP+"+husband->getGeneralName()+"+-Keep", 1);

    setSpouse(husband, wife);
    setSpouse(wife, husband);
}

void CoupleScenario::remarry(ServerPlayer *enkemann, ServerPlayer *widow) const
{
    Room *room = enkemann->getRoom();

    ServerPlayer *ex_husband = getSpouse(widow);
    setSpouse(ex_husband, NULL);
    LogMessage log;
    log.type = "#Divorse";
    log.from = widow;
    log.to << ex_husband;
    room->sendLog(log);

    room->setPlayerMark(widow, "&CP+"+ex_husband->getGeneralName()+"+-Keep", 0);

    ServerPlayer *ex_wife = getSpouse(enkemann);
    if (ex_wife) {
        setSpouse(ex_wife, NULL);
        LogMessage log;
        log.type = "#Divorse";
        log.from = enkemann;
        log.to << ex_wife;
        room->sendLog(log);

        room->setPlayerMark(enkemann, "&CP+"+ex_wife->getGeneralName()+"+-Keep", 0);
    }

    marry(enkemann, widow);
    room->setPlayerProperty(widow, "role", "loyalist");
    room->resetAI(widow);
}

ServerPlayer *CoupleScenario::getSpouse(const ServerPlayer *player) const
{
    return player->tag["spouse"].value<ServerPlayer *>();
}

bool CoupleScenario::isWidow(ServerPlayer *player) const
{
    if (!player->isFemale() || player->isLord())
        return false;

    ServerPlayer *spouse = getSpouse(player);
    return spouse && spouse->isDead();
}

bool CoupleScenario::isSingle(const ServerPlayer *player) const
{
    ServerPlayer *spouse = getSpouse(player);
    return !spouse || spouse->isDead();
}

bool CoupleScenario::AllSingle(const ServerPlayer *player) const
{
    Room *room = player->getRoom();
    foreach (ServerPlayer *player, room->getAlivePlayers()) {
        if (!isSingle(player))
            return false;
    }
    return true;
}

void CoupleScenario::assign(QStringList &generals, QStringList &roles) const
{
    generals << lord;

    /*QStringList husbands = original_husband_map.keys();
    qShuffle(husbands);
    husbands = husbands.mid(0, 3);

    QStringList others;
    foreach(QString husband, husbands)
        others << husband << original_husband_map.value(husband);

    generals << others;
    qShuffle(generals);*/

    QMap<QString, QStringList> filter_map = cp_map;
    while (generals.length() < 7){
        //QStringList froms = filter_map.keys();

        foreach(QString general, generals)
            if (filter_map.keys().contains(general)) {
                foreach(QString to, filter_map[general]) {
                    if (filter_map[to].contains(general)) {
                        filter_map[to].removeAll(general);
                        if (filter_map[to].isEmpty())
                            filter_map.remove(to);
                    }
                }
                filter_map.remove(general);
            }

        /*QMap<QString, QStringList> filter_map;
        foreach(QString from, froms){
            QStringList tos = cp_map[from];
            foreach(QString general, generals)
                if (tos.contains(general))
                    tos.removeOne(general);
            if (!tos.isEmpty())
                filter_map[from] = tos;
        }*/

        QStringList rand_froms = filter_map.keys();
        qShuffle(rand_froms);
        QString rand_from = rand_froms.first();

        QStringList rand_tos = filter_map[rand_from];
        qShuffle(rand_tos);
        QString rand_to = rand_tos.first();

        if (!generals.contains(rand_from) &&!generals.contains(rand_from)) {    //keep it safe
            start_cps.append(rand_from+"+"+rand_to);
            generals << rand_from << rand_to;
        }
    }
    qShuffle(generals);

    // roles
    for (int i = 0; i < 7; i++) {
        if (generals.at(i) == lord)
            roles << "lord";
        else
            roles << "renegade";
    }
}

int CoupleScenario::getPlayerCount() const
{
    return 7;
}

QString CoupleScenario::getRoles() const
{
    return "ZNNNNNN";
}

void CoupleScenario::onTagSet(Room *, const QString &) const
{
}

AI::Relation CoupleScenario::relationTo(const ServerPlayer *a, const ServerPlayer *b) const
{
    if (getSpouse(a) == b)
        return AI::Friend;

    if (!AllSingle(a) && isSingle(a) && isSingle(b))    //Temporary alliance
        return AI::Friend;

    if ((a->isLord() && b->isFemale()) || (b->isLord() && a->isFemale()))
        return AI::Neutrality;

    return AI::Enemy;
}

/*QMap<QString, QStringList> CoupleScenario::getMap(bool isHusband) const
{
    return isHusband ? husband_map : wife_map;
}

QMap<QString, QString> CoupleScenario::getOriginalMap(bool isHusband) const
{
    return isHusband ? original_husband_map : original_wife_map;
}*/

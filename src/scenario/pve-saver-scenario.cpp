#include "pve-saver-scenario.h"
#include "skill.h"
#include "engine.h"
#include "room.h"
#include "roomthread.h"
#include "util.h"
#include "generalselector.h"


class PveSaverScenarioRule : public ScenarioRule
{
public:
    PveSaverScenarioRule(Scenario *scenario)
        : ScenarioRule(scenario)
    {
        events << GameReady << GameOverJudge << BuryVictim;
    }

    bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        const PveSaverScenario *scenario = qobject_cast<const PveSaverScenario *>(parent());

        switch (triggerEvent) {
        case GameReady: {
            if (player != NULL) return false;
            /*foreach (ServerPlayer *player, room->getPlayers()) {
                if (player->isLord()) {

                    continue;
                } else {

                }
            }*/

            QList<ServerPlayer *> online_players;
            QList<ServerPlayer *> leaders;
            ServerPlayer *leader;
            foreach (ServerPlayer *player, room->getPlayers()) {
                if (player->isOnline())
                    online_players.append(player);
            }
            foreach (ServerPlayer *player, online_players) {
                if (player->askForSkillInvoke("ask_for_leader"))
                    leaders.append(player);
            }
            if (leaders.isEmpty())
                if (online_players.isEmpty())
                    leader = room->getPlayers().at(2);
                else{
                    qShuffle(online_players);
                    leader = online_players.first();
                }
            else {
                qShuffle(leaders);
                leader = leaders.first();
            }

            QStringList general_names = Sanguosha->getLimitedGeneralNames();

            qShuffle(general_names);
            general_names = general_names.mid(0, 16);

            foreach (QString name, general_names){
                leader->addToSelected(name);
            }
            room->startArrangeForPve(leader);

            room->setTag("SkipNormalDeathProcess", true);
            break;
        }
        case GameOverJudge: {
            QList<ServerPlayer *> players = room->getAlivePlayers();
            if (players.length() == 1) {
                ServerPlayer *survivor = players.first();
                room->gameOver(survivor->getRole());

                return true;
            } else if (room->getLord()->isDead()) {
                room->gameOver("rebel");
                return true;
            }

            return true;
        }

        case BuryVictim: {
            //DeathStruct death = data.value<DeathStruct>();
            player->bury();

            // reward and punishment
            foreach (ServerPlayer *player, room->getAlivePlayers()) {
                if (player->getRole() == "rebel")
                    player->drawCards(1);
            }

            break;
        }
        default:
            break;
        }

        return false;
    }
};

PveSaverScenario::PveSaverScenario()
    : Scenario("pve-saver")
{
    lua_State *lua = Sanguosha->getLuaState();
    QStringList lords_list = GetConfigFromLuaState(lua, "pve_saver_lord").toString().split("|");
    lord = lords_list.at(qrand() % lords_list.length());

    rule = new PveSaverScenarioRule(this);
}

void PveSaverScenario::assign(QStringList &generals, QStringList &roles) const
{
    generals << lord << "anjiang" << "anjiang" << "anjiang";
    roles << "lord" << "rebel" << "rebel" << "rebel";
}

int PveSaverScenario::getPlayerCount() const
{
    return 4;
}

QString PveSaverScenario::getRoles() const
{
    return "ZFFF";
}

void PveSaverScenario::onTagSet(Room *, const QString &) const
{
}

AI::Relation PveSaverScenario::relationTo(const ServerPlayer *a, const ServerPlayer *b) const
{
    if (!a->isLord() && !b->isLord())
        return AI::Friend;

    return AI::Enemy;
}

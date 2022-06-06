#include "roomthreadif.h"
#include "room.h"
#include "engine.h"
#include "settings.h"
#include "generalselector.h"
#include "json.h"
#include "package.h"
#include "util.h"
#include "roomthread.h"

using namespace QSanProtocol;

RoomThreadif::RoomThreadif(Room *room)
    : room(room)
{
    room->getRoomState()->reset();
}

void RoomThreadif::run()
{
    // initialize the random seed for this thread
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
    int total_num = 12;

    //QSet<QString> banset = Config.value("Banlist/1v1").toStringList().toSet();
    //general_names = Sanguosha->getRandomGenerals(total_num);
    QStringList all_generals = GetConfigFromLuaState(Sanguosha->getLuaState(), "if_generals").toStringList();

    qShuffle(all_generals);

    general_names = all_generals.mid(0, total_num);

    room->doBroadcastNotify(S_COMMAND_FILL_GENERAL, JsonUtils::toJsonArray(general_names));

    //int index = qrand() % 2;
    ServerPlayer *first = room->getPlayers().at(2), *next = room->getPlayers().at(0);

    room->doAnimate(2,"skill=Conversation:", "if_iget_1");
    askForTakeGeneral(first);
    while (general_names.length() > 1) {
        qSwap(first, next);

        room->doAnimate(2,"skill=Conversation:", first == room->getPlayers().at(2) ? "if_iget_2" : "if_fget_2");

        askForTakeGeneral(first);
        askForTakeGeneral(first);
    }
    //room->doAnimate(2,"skill=Conversation:", "if_iget_1");
    askForTakeGeneral(next);

    //if (rule == "2013")
    //    askForFirstGeneral(QList<ServerPlayer *>() << first << next);
    //else
    //    startArrange(QList<ServerPlayer *>() << first << next);

    room->getPlayers().at(1)->setGeneralName("tianxixi_leader");
    room->broadcastProperty(room->getPlayers().at(1), "general");
    room->getPlayers().at(3)->setGeneralName("xiaoxiayu_leader");
    room->broadcastProperty(room->getPlayers().at(3), "general");

    room->doAnimate(2,"skill=Conversation:", "if_iselect_1");
    QStringList arranged_n = next->getSelected();
    QString general_name_n1 = room->askForGeneral(next, arranged_n);
    arranged_n.removeOne(general_name_n1);
    room->doAnimate(2,"skill=Conversation:", "if_iselect_2");
    QString general_name_n2 = room->askForGeneral(next, arranged_n);
    arranged_n.removeOne(general_name_n2);
    QString general_name_n3 = room->askForGeneral(next, arranged_n);

    QStringList all_selected_n;
    all_selected_n.append(general_name_n1);
    all_selected_n.append(general_name_n2);
    all_selected_n.append(general_name_n3);
    next->tag["if_selected"] = QVariant::fromValue(all_selected_n);


    room->doAnimate(2,"skill=Conversation:", "if_fselect_1");
    QStringList arranged_f = first->getSelected();
    QString general_name_f1 = room->askForGeneral(first, arranged_f);
    arranged_f.removeOne(general_name_f1);
    room->doAnimate(2,"skill=Conversation:", "if_fselect_2");
    QString general_name_f2 = room->askForGeneral(first, arranged_f);
    arranged_f.removeOne(general_name_f2);
    QString general_name_f3 = room->askForGeneral(first, arranged_f);

    QStringList all_selected_f;
    all_selected_f.append(general_name_f1);
    all_selected_f.append(general_name_f2);
    all_selected_f.append(general_name_f3);
    first->tag["if_selected"] = QVariant::fromValue(all_selected_f);


    next->setGeneralName(general_name_n1);
    room->broadcastProperty(next, "general");
    next->setKingdom("team_ice");
    room->broadcastProperty(next, "kingdom");

    first->setGeneralName(general_name_f1);
    room->broadcastProperty(first, "general");
    first->setKingdom("team_fire");
    room->broadcastProperty(first, "kingdom");

    room->doBroadcastNotify(S_COMMAND_REVEAL_GENERAL, JsonUtils::toJsonArray(QStringList() << next->objectName() << general_name_n1));
    room->doBroadcastNotify(S_COMMAND_REVEAL_GENERAL, JsonUtils::toJsonArray(QStringList() << next->objectName() << general_name_n2));
    room->doBroadcastNotify(S_COMMAND_REVEAL_GENERAL, JsonUtils::toJsonArray(QStringList() << next->objectName() << general_name_n3));
    room->doBroadcastNotify(S_COMMAND_REVEAL_GENERAL, JsonUtils::toJsonArray(QStringList() << first->objectName() << general_name_f1));
    room->doBroadcastNotify(S_COMMAND_REVEAL_GENERAL, JsonUtils::toJsonArray(QStringList() << first->objectName() << general_name_f2));
    room->doBroadcastNotify(S_COMMAND_REVEAL_GENERAL, JsonUtils::toJsonArray(QStringList() << first->objectName() << general_name_f3));

    next->clearSelected();
    next->addToSelected(general_name_n1);
    next->addToSelected(general_name_n2);
    next->addToSelected(general_name_n3);
    first->clearSelected();
    first->addToSelected(general_name_f1);
    first->addToSelected(general_name_f2);
    first->addToSelected(general_name_f3);
}

void RoomThreadif::askForTakeGeneral(ServerPlayer *player)
{
    room->tryPause();

    QString name;
    if (general_names.length() == 1)
        name = general_names.first();
    else if (player->getState() != "online")
        name = GeneralSelector::getInstance()->select1v1(general_names);

    if (name.isNull()) {
        bool success = room->doRequest(player, S_COMMAND_ASK_GENERAL, QVariant(), true);
        QVariant clientReply = player->getClientReply();
        if (success && JsonUtils::isString(clientReply)) {
            name = clientReply.toString();
            takeGeneral(player, name);
        } else {
            GeneralSelector *selector = GeneralSelector::getInstance();
            name = selector->select1v1(general_names);
            takeGeneral(player, name);
        }
    } else {
        msleep(Config.AIDelay);
        takeGeneral(player, name);
    }
}

void RoomThreadif::takeGeneral(ServerPlayer *player, const QString &name)
{
    QString rule = Config.value("1v1/Rule", "2013").toString();
    QString group = player->getRole() == "loyalist" ? "warm" : "cool";
    room->doBroadcastNotify(room->getOtherPlayers(player, true), S_COMMAND_TAKE_GENERAL, JsonUtils::toJsonArray(QStringList() << group << name << rule));

    QRegExp unknown_rx("x(\\d)");
    QString general_name = name;
    if (unknown_rx.exactMatch(name)) {
        int index = unknown_rx.capturedTexts().at(1).toInt();
        general_name = unknown_list.at(index);

        JsonArray arg;
        arg << index << general_name;
        room->doNotify(player, S_COMMAND_RECOVER_GENERAL, arg);
    }

    room->doNotify(player, S_COMMAND_TAKE_GENERAL, JsonUtils::toJsonArray(QStringList() << group << general_name << rule));

    QString namearg = unknown_rx.exactMatch(name) ? "anjiang" : name;
    foreach (ServerPlayer *p, room->getPlayers()) {
        LogMessage log;
        log.type = "#VsTakeGeneral";
        log.arg = group;
        log.arg2 = (p == player) ? general_name : namearg;
        room->sendLog(log, p);
    }

    general_names.removeOne(name);
    player->addToSelected(general_name);
}

void RoomThreadif::startArrange(QList<ServerPlayer *> players)
{
    room->tryPause();
    QList<ServerPlayer *> online = players;
    foreach (ServerPlayer *player, players) {
        if (!player->isOnline()) {
            GeneralSelector *selector = GeneralSelector::getInstance();
            arrange(player, selector->arrange1v1(player));
            online.removeOne(player);
        }
    }
    if (online.isEmpty()) return;

    foreach(ServerPlayer *player, online)
        player->m_commandArgs = QVariant();

    room->doBroadcastRequest(online, S_COMMAND_ARRANGE_GENERAL);

    foreach (ServerPlayer *player, online) {
        JsonArray clientReply = player->getClientReply().value<JsonArray>();
        if (player->m_isClientResponseReady && clientReply.size() == 3) {
            QStringList arranged;
            JsonUtils::tryParse(clientReply, arranged);
            arrange(player, arranged);
        } else {
            GeneralSelector *selector = GeneralSelector::getInstance();
            arrange(player, selector->arrange1v1(player));
        }
    }
}

void RoomThreadif::askForFirstGeneral(QList<ServerPlayer *> players)
{
    room->tryPause();
    QList<ServerPlayer *> online = players;
    foreach (ServerPlayer *player, players) {
        if (!player->isOnline()) {
            GeneralSelector *selector = GeneralSelector::getInstance();
            QStringList arranged = player->getSelected();
            QStringList selected = selector->arrange1v1(player);
            selected.append(arranged);
            selected.removeDuplicates();
            arrange(player, selected);
            online.removeOne(player);
        }
    }
    if (online.isEmpty()) return;

    foreach(ServerPlayer *player, online)
        player->m_commandArgs = JsonUtils::toJsonArray(player->getSelected());

    room->doBroadcastRequest(online, S_COMMAND_CHOOSE_GENERAL);

    foreach (ServerPlayer *player, online) {
        QVariant clientReply = player->getClientReply();
        if (player->m_isClientResponseReady && JsonUtils::isString(clientReply) && player->getSelected().contains(clientReply.toString())) {
            QStringList arranged = player->getSelected();
            QString first_gen = clientReply.toString();
            arranged.removeOne(first_gen);
            arranged.prepend(first_gen);
            arrange(player, arranged);
        } else {
            GeneralSelector *selector = GeneralSelector::getInstance();
            QStringList arranged = player->getSelected();
            QStringList selected = selector->arrange1v1(player);
            selected.append(arranged);
            selected.removeDuplicates();
            arrange(player, selected);
        }
    }
}

void RoomThreadif::arrange(ServerPlayer *player, const QStringList &arranged)
{
    QString rule = Config.value("1v1/Rule", "2013").toString();
    Q_ASSERT(arranged.length() == ((rule == "2013") ? 6 : 3));

    QStringList left = arranged.mid(1);
    player->tag["1v1Arrange"] = QVariant::fromValue(left);
    player->setGeneralName(arranged.first());

    foreach (QString general, arranged) {
        room->doNotify(player, S_COMMAND_REVEAL_GENERAL, JsonUtils::toJsonArray(QStringList() << player->objectName() << general));
        if (rule != "Classical") break;
    }
}


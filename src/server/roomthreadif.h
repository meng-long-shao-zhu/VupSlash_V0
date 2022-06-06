#ifndef _ROOM_THREAD_IF_H
#define _ROOM_THREAD_IF_H


class Room;
class ServerPlayer;

class RoomThreadif : public QThread
{
    Q_OBJECT

public:
    explicit RoomThreadif(Room *room);
    void takeGeneral(ServerPlayer *player, const QString &name);
    void arrange(ServerPlayer *player, const QStringList &arranged);

protected:
    virtual void run();

private:
    Room *room;
    QStringList general_names;
    QStringList unknown_list;

    void askForTakeGeneral(ServerPlayer *player);
    void startArrange(QList<ServerPlayer *> players);
    void askForFirstGeneral(QList<ServerPlayer *> players);
};

#endif


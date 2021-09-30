#ifndef _PVE_SAVER_SCENARIO_H
#define _PVE_SAVER_SCENARIO_H

#include "scenario.h"

class PveSaverScenario : public Scenario
{
    Q_OBJECT

public:
    explicit PveSaverScenario();

    void assign(QStringList &generals, QStringList &roles) const;
    int getPlayerCount() const;
    QString getRoles() const;
    void onTagSet(Room *room, const QString &key) const;
    AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;

private:

};

#endif


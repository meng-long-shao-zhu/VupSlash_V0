#ifndef LI_H
#define LI_H

#include "package.h"
#include "card.h"
#include "skill.h"

class LiPackage : public Package
{
    Q_OBJECT

public:
    LiPackage();
};

class JinYingshiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JinYingshiCard();
    void onUse(Room *room, const CardUseStruct &card_use) const;
};

class JinXiongzhiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JinXiongzhiCard();
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class JinXiongzhiUseCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JinXiongzhiUseCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
};

class JinQinglengCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JinQinglengCard();
    void onUse(Room *room, const CardUseStruct &card_use) const;
};

#endif

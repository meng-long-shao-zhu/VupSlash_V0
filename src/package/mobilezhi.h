#ifndef MOBILEZHI_H
#define MOBILEZHI_H

#include "package.h"
#include "card.h"
#include "skill.h"

class MobileZhiPackage : public Package
{
    Q_OBJECT

public:
    MobileZhiPackage();
};

class MobileZhiQiaiCard : public SkillCard
{
    Q_OBJECT
public:
    Q_INVOKABLE MobileZhiQiaiCard();
    void onEffect(const CardEffectStruct &effect) const;
};

class MobileZhiShamengCard : public SkillCard
{
    Q_OBJECT
public:
    Q_INVOKABLE MobileZhiShamengCard();
    void onEffect(const CardEffectStruct &effect) const;
};

class MobileZhiDuojiCard : public SkillCard
{
    Q_OBJECT
public:
    Q_INVOKABLE MobileZhiDuojiCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class MobileZhiJianzhanCard : public SkillCard
{
    Q_OBJECT
public:
    Q_INVOKABLE MobileZhiJianzhanCard();
    void onEffect(const CardEffectStruct &effect) const;
};

#endif
#ifndef MOBILEZHI_H
#define MOBILEZHI_H

#include "package.h"
#include "card.h"
#include "skill.h"

class MobileZhiPackage : public Package
{
    Q_OBJECT

public:
    MobileZhiPackage();
};

class MobileZhiQiaiCard : public SkillCard
{
    Q_OBJECT
public:
    Q_INVOKABLE MobileZhiQiaiCard();
    void onEffect(const CardEffectStruct &effect) const;
};

#endif

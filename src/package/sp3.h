#ifndef SP3_H
#define SP3_H

#include "package.h"
#include "card.h"
#include "standard.h"
#include "wind.h"

class SP3Package : public Package
{
    Q_OBJECT

public:
    SP3Package();
};

class YujueCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YujueCard(QString zhihu = "zhihu");
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
private:
    QString zhihu;
};

class SecondYujueCard : public YujueCard
{
    Q_OBJECT

public:
    Q_INVOKABLE SecondYujueCard();
};

class SpNiluanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE SpNiluanCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
};

class WeiwuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE WeiwuCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
};

class CixiaoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE CixiaoCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *) const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
    void use(Room *, ServerPlayer *, QList<ServerPlayer *> &targets) const;
};

class JieyinghCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JieyinghCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
};

class MinsiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE MinsiCard();
    void use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class JijingCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JijingCard();
    void onUse(Room *, const CardUseStruct &) const;
};

class DaojiCard : public SkillCard
{
    Q_OBJECT
public:
    Q_INVOKABLE DaojiCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class PingjianCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE PingjianCard();
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class ShoufuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShoufuCard();
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const;
};

class ShoufuPutCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShoufuPutCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const;
    void onUse(Room *, const CardUseStruct &card_use) const;
};

class TenyearSongciCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE TenyearSongciCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class YoulongCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YoulongCard();
    //bool targetFixed() const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    const Card *validate(CardUseStruct &card_use) const;
    const Card *validateInResponse(ServerPlayer *source) const;
};

class YoulongDialog : public GuhuoDialog
{
    Q_OBJECT

public:
    static YoulongDialog *getInstance(const QString &object);

protected:
    explicit YoulongDialog(const QString &object);
    bool isButtonEnabled(const QString &button_name) const;
};

class SecondZhanyiViewAsBasicCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE SecondZhanyiViewAsBasicCard();
    bool targetFixed() const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    const Card *validate(CardUseStruct &cardUse) const;
    const Card *validateInResponse(ServerPlayer *user) const;
};

class SecondZhanyiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE SecondZhanyiCard();
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ZunweiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZunweiCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class BazhanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE BazhanCard();
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void BazhanEffect(ServerPlayer *from, ServerPlayer *to) const;
    void onEffect(const CardEffectStruct &effect) const;
};

#endif

#include "general.h"
#include "engine.h"
#include "skill.h"
#include "package.h"
#include "client.h"
#include "clientstruct.h"

General::General(Package *package, const QString &name, const QString &kingdom,
    int max_hp, bool male, bool hidden, bool never_shown, int start_hp)
    : QObject(package), kingdom(kingdom), max_hp(max_hp), gender(male ? Male : Female),
    hidden(hidden), never_shown(never_shown), start_hp(start_hp)
{
    static QChar lord_symbol('$');
    if (name.endsWith(lord_symbol)) {
        QString copy = name;
        copy.remove(lord_symbol);
        lord = true;
        setObjectName(copy);
    } else {
        lord = false;
        setObjectName(name);
    }
    is_bonus = false;
}

int General::getMaxHp() const
{
    return max_hp;
}

QString General::getKingdom() const
{
    return kingdom;
}

bool General::isMale() const
{
    return gender == Male;
}

bool General::isFemale() const
{
    return gender == Female;
}

bool General::isNeuter() const
{
    return gender == Neuter;
}

bool General::isSexless() const
{
    return gender == Sexless;
}

void General::setGender(Gender gender)
{
    this->gender = gender;
}

General::Gender General::getGender() const
{
    return gender;
}

bool General::isLord() const
{
    return lord;
}

bool General::isHidden() const
{
    return hidden;
}

bool General::isTotallyHidden() const
{
    return never_shown;
}

int General::getStartHp() const
{
    //return qMin(start_hp, max_hp);
    if (start_hp && start_hp != -1)
        return start_hp;
    else
        return max_hp;
}

void General::addSkill(Skill *skill)
{
    if (!skill) {
        QMessageBox::warning(NULL, "", tr("Invalid skill added to general %1").arg(objectName()));
        return;
    }
    if (!skillname_list.contains(skill->objectName())) {
        skill->setParent(this);
        skillname_list << skill->objectName();
    }
}

void General::addSkill(const QString &skill_name)
{
    if (!skillname_list.contains(skill_name)) {
        extra_set.insert(skill_name);
        skillname_list << skill_name;
    }
}

bool General::hasSkill(const QString &skill_name) const
{
    return skillname_list.contains(skill_name);
}

QList<const Skill *> General::getSkillList() const
{
    QList<const Skill *> skills;
    foreach (QString skill_name, skillname_list) {
        if (skill_name == "mashu" && ServerInfo.DuringGame
            && ServerInfo.GameMode == "02_1v1" && ServerInfo.GameRuleMode != "Classical")
            skill_name = "xiaoxi";
        const Skill *skill = Sanguosha->getSkill(skill_name);
        skills << skill;
    }
    return skills;
}

QList<const Skill *> General::getVisibleSkillList() const
{
    QList<const Skill *> skills;
    foreach (const Skill *skill, getSkillList()) {
        if (skill->isVisible())
            skills << skill;
    }

    return skills;
}

QSet<const Skill *> General::getVisibleSkills() const
{
    return getVisibleSkillList().toSet();
}

QSet<const TriggerSkill *> General::getTriggerSkills() const
{
    QSet<const TriggerSkill *> skills;
    foreach (QString skill_name, skillname_list) {
        const TriggerSkill *skill = Sanguosha->getTriggerSkill(skill_name);
        if (skill)
            skills << skill;
    }
    return skills;
}

void General::addRelateSkill(const QString &skill_name)
{
    related_skills << skill_name;
}

QStringList General::getRelatedSkillNames() const
{
    return related_skills;
}

QString General::getPackage() const
{
    QObject *p = parent();
    if (p)
        return p->objectName();
    else
        return QString(); // avoid null pointer exception;
}

QString General::getSkillDescription(bool include_name, bool include_difficulty) const
{
    QString description;

    QList<const Skill *> skills = getVisibleSkillList();
    QList<const Skill *> relatedskills;
    foreach (const QString &skill_name, getRelatedSkillNames()) {
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if (skill && (skill->isVisible() || skill_name.startsWith("characteristic"))) {
            skills << skill;
            relatedskills << skill;
        }
    }

    foreach (const Skill *skill, skills) {
        QString skill_name = Sanguosha->translate(skill->objectName());
        QString desc = skill->getDescription();
        desc.replace("\n", "<br/>");
        if (!relatedskills.contains(skill))
            description.append(QString("<b>%1</b>: %2 <br/> <br/>").arg(skill_name).arg(desc));
        else
            description.append(QString("<font color=\"#01A5AF\"><b>%1</b></font>: <font color=\"#01A5AF\">%2</font> <br/> <br/>").arg(skill_name).arg(desc));
    }

    if (include_name) {
        QString color_str = Sanguosha->getKingdomColor(kingdom).name();
        QString name = QString("<font color=%1><b>%2</b></font>     ").arg(color_str).arg(Sanguosha->translate(objectName()));
        if (isBonus())
            name.prepend("<img src='image/system/bonus_star.png' height = 17/>");
        name.prepend(QString("<img src='image/kingdom/icon/%1.png' height=32/>    ").arg(kingdom));

        QString gender("  <img src='image/gender/%1.png' height=17 />");
        if (isMale())
            name.append(gender.arg("male"));
        else if (isFemale())
            name.append(gender.arg("female"));
        else if (isNeuter())
            name.append(gender.arg("neuter"));
        else if (isSexless())
            name.append(gender.arg("sexless"));

        int start_hp = getStartHp();
        //start_hp = qMin(start_hp, max_hp);
        /*if (start_hp < max_hp) {
            for (int i = 0; i < start_hp; i++)
                name.append("<img src='image/system/magatamas/5.png' height = 12/>");
            for (int i = 0; i < max_hp - start_hp; i++)
                name.append("<img src='image/system/magatamas/0.png' height = 12/>");
        } else if (start_hp > max_hp) {
                for (int i = 0; i < max_hp; i++)
                    name.append("<img src='image/system/magatamas/5.png' height = 12/>");
                for (int i = 0; i < start_hp - max_hp; i++)
                    name.append("<img src='image/system/magatamas/6.png' height = 12/>");
        } else {
            for (int i = 0; i < max_hp; i++)
                name.append("<img src='image/system/magatamas/5.png' height = 12/>");
        }*/
        name.append("  <img src='image/system/magatama_icon.png' height = 15/>");
        if (start_hp == max_hp) {
            if (start_hp >= 5) {
                name.append(QString("<b>%1</b>").arg(start_hp));
            } else {
                for (int i = 1; i < start_hp; i++)
                    name.append("<img src='image/system/magatama_icon.png' height = 14/>");
            }
        } else {
            name.append(QString("<b>%1/%2</b>").arg(start_hp).arg(max_hp));
        }

        if (include_difficulty) {
            name.prepend("<table width='100%'><tr valign='baseline'><th align='left'>");
            name.append("</th>");
            QString diff_str = (Sanguosha->translate("$" + objectName()) == "$" + objectName()) ? Sanguosha->translate("default_difficulty") : Sanguosha->translate("$" + objectName());
            name.append("<th align='right'>" + Sanguosha->translate("CHARACTER_DIFFICULTY") + ": " + diff_str + "</th></tr></table> <br/> <br/>");
        } else {
            name.append("<br/> <br/>");
        }
        description.prepend(name);
    }

    return description;
}

QString General::getBriefName() const
{
    QString name = Sanguosha->translate("&" + objectName());
    if (name.startsWith("&"))
        name = Sanguosha->translate(objectName());

    return name;
}

void General::lastWord() const
{
    QString filename = QString("audio/death/%1.ogg").arg(objectName());
    bool fileExists = QFile::exists(filename);
    if (!fileExists) {
        QStringList origin_generals = objectName().split("_");
        if (origin_generals.length() > 1)
            filename = QString("audio/death/%1.ogg").arg(origin_generals.last());
    }
    Sanguosha->playAudioEffect(filename);
}

bool General::hasHideSkill() const
{
    foreach (const Skill *skill, getSkillList()) {
        if (skill->isHideSkill())
            return true;
    }
    return false;
}

void General::setHidden(bool val)
{
    hidden = val;
    return;
}

void General::setTotallyHidden(bool val)
{
    never_shown = val;
    return;
}

bool General::isBonus() const
{
    return is_bonus;
}

void General::setBonus(bool val)
{
    is_bonus = val;
    return;
}

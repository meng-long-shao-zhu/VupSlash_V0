#include "knowncardviewdialog.h"

#include "client.h"
#include "clientplayer.h"
#include "engine.h"
#include "roomscene.h"
#include "skin-bank.h"

static QLayout *HLay(QWidget *left, QWidget *right, QWidget *mid = NULL,
    QWidget *rear = NULL, bool is_vertically = false)
{
    QBoxLayout *layout;
    if (is_vertically) layout = new QVBoxLayout;
    else layout = new QHBoxLayout;

    layout->addWidget(left);
    if (mid)
        layout->addWidget(mid);
    layout->addWidget(right);
    if (rear) layout->addWidget(rear);

    return layout;
}

class KnowncardViewDialogUI
{
public:
    KnowncardViewDialogUI()
    {
        from = new QComboBox;
        hand_list = new QListWidget;
        set_overt = new QCheckBox(Sanguosha->translate("KCV_OVERT_ONLY"));
    }
    QComboBox *from;
    QListWidget *hand_list;
    QCheckBox *set_overt;
};

KnowncardViewDialog::KnowncardViewDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(Sanguosha->translate("KNOWNCARD_VIEW"));

    ui = new KnowncardViewDialogUI;

    QVBoxLayout *main_layout = new QVBoxLayout();
    QVBoxLayout *hands_group_layout = new QVBoxLayout();
    QGroupBox *hands_group = new QGroupBox(Sanguosha->translate("KCV_HANDCARD"));
    QVBoxLayout *hand_lay = new QVBoxLayout();
    hands_group_layout->addWidget(hands_group);
    hands_group->setLayout(hand_lay);
    hand_lay->addWidget(ui->hand_list);
    QPushButton *refresh_button = new QPushButton(Sanguosha->translate("KCV_REFRESH"));
    hand_lay->addLayout(HLay(ui->set_overt, refresh_button));
    QFormLayout *layout = new QFormLayout;

    RoomScene::FillPlayerNames(ui->from, false);

    connect(ui->from, SIGNAL(currentIndexChanged(int)), this, SLOT(refreshCards()));
    connect(refresh_button, SIGNAL(clicked()), this, SLOT(refreshCards()));
    connect(ui->set_overt, SIGNAL(toggled(bool)), this, SLOT(refreshCards()));

    layout->addRow(Sanguosha->translate("KCV_TARGET"), ui->from);
    main_layout->addLayout(layout);
    main_layout->addLayout(hands_group_layout);

    setLayout(main_layout);

    refreshCards();
}

KnowncardViewDialog::~KnowncardViewDialog()
{
    delete ui;
}

void KnowncardViewDialog::refreshCards()
{
    QString from_name = ui->from->itemData(ui->from->currentIndex()).toString();

    const ClientPlayer *from = ClientInstance->getPlayer(from_name);

    ui->hand_list->clear();

    if (!from) return;

    QList<const Card *> known = from->getHandcards();
    if (Self->canSeeHandcard(from) && from->getHandcardNum() > 0) {
        //known.clear();
        if (!from->property("My_Visible_HandCards").toString().isEmpty()) {
            QStringList handcard = from->property("My_Visible_HandCards").toString().split("+");
            foreach (QString id, handcard) {
                bool contains = false;
                foreach (const Card *card, known) {
                    if (card->getId() == id.toInt()) {
                        contains = true;
                        break;
                    }
                }
                if (!contains) {
                    known << Sanguosha->getEngineCard(id.toInt());
                }
            }
        }
    }
    if (!known.isEmpty()) {
        foreach (const Card *card, known) {
            const Card *engine_card = Sanguosha->getEngineCard(card->getId());
            QString card_name = Sanguosha->translate(engine_card->objectName());
            QIcon suit_icon = QIcon(QString("image/system/suit/%1.png").arg(engine_card->getSuitString()));
            QString point = engine_card->getNumberString();

            QString card_info = point + "  " + card_name;
            if (from->getPublicCards().contains(card))
                card_info += "\t" + Sanguosha->translate("OVERT");
            else if (ui->set_overt->checkState() == Qt::Checked)
                continue;
            QListWidgetItem *name_item = new QListWidgetItem(card_info, ui->hand_list);
            name_item->setIcon(suit_icon);
            name_item->setData(Qt::UserRole, card->getId());
        }
    }
}


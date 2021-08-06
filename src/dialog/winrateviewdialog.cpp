#include "winrateviewdialog.h"

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

class WinrateViewDialogUI
{
public:
    WinrateViewDialogUI()
    {
        tableWidget = new QTableWidget;
        totalLineEdit = new QLineEdit;
        totalwinLineEdit = new QLineEdit;
        totalrateLineEdit = new QLineEdit;
        counterLineEdit = new QLineEdit;
        set_ignore = new QCheckBox(Sanguosha->translate("WINRATE_IGNORE"));
        ignore_lessthan = new QSpinBox();
        set_ratemin = new QCheckBox(Sanguosha->translate("WINRATE_RATEMIN"));
        ratemin = new QSpinBox();
        set_ratemax = new QCheckBox(Sanguosha->translate("WINRATE_RATEMAX"));
        ratemax = new QSpinBox();
        skillTextEdit = new QTextEdit;
    }
    QTableWidget *tableWidget;
    QLineEdit *totalLineEdit;
    QLineEdit *totalwinLineEdit;
    QLineEdit *totalrateLineEdit;
    QLineEdit *counterLineEdit;
    QCheckBox *set_ignore;
    QSpinBox *ignore_lessthan;
    QCheckBox *set_ratemin;
    QSpinBox *ratemin;
    QCheckBox *set_ratemax;
    QSpinBox *ratemax;
    QTextEdit *skillTextEdit;
};

WinrateViewDialog::WinrateViewDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(Sanguosha->translate("WINRATE_VIEW"));
    setMinimumSize(393, 466);

    ui = new WinrateViewDialogUI;
    QHBoxLayout *main_layout = new QHBoxLayout();
    QVBoxLayout *layout = new QVBoxLayout();
    QVBoxLayout *layout_R = new QVBoxLayout();

    QLabel *state_label = new QLabel();
    state_label->setText(Sanguosha->translate("WINRATE_STATE"));
    QLabel *total_label = new QLabel();
    total_label->setText(Sanguosha->translate("WINRATE_TOTAL"));
    QLabel *totalwin_label = new QLabel();
    totalwin_label->setText(Sanguosha->translate("WINRATE_TOTALWIN"));
    QLabel *totalrate_label = new QLabel();
    totalrate_label->setText(Sanguosha->translate("WINRATE_TOTALRATE"));
    QLabel *counter_label = new QLabel();
    counter_label->setText(Sanguosha->translate("WINRATE_COUNTER"));

    ui->totalLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    ui->totalLineEdit->setReadOnly(true);
    ui->totalwinLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    ui->totalwinLineEdit->setReadOnly(true);
    ui->totalrateLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    ui->totalrateLineEdit->setReadOnly(true);
    ui->counterLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    ui->counterLineEdit->setReadOnly(true);

    ui->skillTextEdit->setReadOnly(true);
    ui->skillTextEdit->setProperty("description", true);    //can force the backgound_pic to null

    ui->set_ignore->setChecked(false);
    ui->ignore_lessthan->setValue(1);
    ui->ignore_lessthan->setRange(1, 9999);
    ui->set_ratemin->setChecked(false);
    ui->ratemin->setValue(50);
    ui->ratemin->setRange(0, 100);
    ui->ratemin->setSuffix("%");
    ui->set_ratemax->setChecked(false);
    ui->ratemax->setValue(50);
    ui->ratemax->setRange(0, 100);
    ui->ratemax->setSuffix("%");

    layout->addWidget(state_label);

    QHBoxLayout *total_layout = new QHBoxLayout();
    total_layout->addLayout(HLay(total_label,ui->totalLineEdit));
    total_layout->addLayout(HLay(totalwin_label,ui->totalwinLineEdit));
    total_layout->addLayout(HLay(totalrate_label,ui->totalrateLineEdit));
    QWidget* total_widget = new QWidget;    //in order to resize this layout
    total_widget->setLayout(total_layout);
    total_widget->setMaximumWidth(367);
    layout->addWidget(total_widget);

    layout->addWidget(ui->tableWidget);
    layout->setAlignment(Qt::AlignTop);

    QGroupBox *filter_area = new QGroupBox(Sanguosha->translate("WINRATE_FILTER"));
    QVBoxLayout *filter_layout = new QVBoxLayout();
    filter_layout->addLayout(HLay(ui->set_ignore, ui->ignore_lessthan));
    filter_layout->addLayout(HLay(ui->set_ratemin, ui->ratemin));
    filter_layout->addLayout(HLay(ui->set_ratemax, ui->ratemax));
    filter_layout->addLayout(HLay(counter_label, ui->counterLineEdit));
    filter_area->setLayout(filter_layout);

    QGroupBox *char_info_area = new QGroupBox(Sanguosha->translate("WINRATE_CHAR_INFO"));
    QVBoxLayout *char_info_layout = new QVBoxLayout();
    char_info_layout->addWidget(ui->skillTextEdit);
    char_info_area->setLayout(char_info_layout);

    layout_R->addWidget(filter_area);
    layout_R->addWidget(char_info_area);

    main_layout->addLayout(layout);
    main_layout->addLayout(layout_R);
    setLayout(main_layout);

    connect(ui->set_ignore, SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(ui->ignore_lessthan, SIGNAL(valueChanged(int)), this, SLOT(refresh()));
    connect(ui->set_ratemin, SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(ui->ratemin, SIGNAL(valueChanged(int)), this, SLOT(refresh()));
    connect(ui->set_ratemax, SIGNAL(toggled(bool)), this, SLOT(refresh()));
    connect(ui->ratemax, SIGNAL(valueChanged(int)), this, SLOT(refresh()));
    connect(ui->tableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(refresh_skill_text()));

    refresh();
}

WinrateViewDialog::~WinrateViewDialog()
{
    delete ui;
}

void WinrateViewDialog::refresh()
{
    QFile file("datacount.dll");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray byte_read = file.readAll();
    QString data_read = QString(byte_read);
    file.close();

    QStringList data_read_lines = data_read.split("\n");

    ui->tableWidget->clearContents();
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setRowCount(data_read_lines.size()-1);
    ui->tableWidget->setIconSize(QSize(20, 20));
    ui->tableWidget->setObjectName(QStringLiteral("tableWidget"));
    //ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSortingEnabled(true);

    QTableWidgetItem *header_0 = new QTableWidgetItem(Sanguosha->translate("WINRATE_NAME"));
    ui->tableWidget->setHorizontalHeaderItem(0, header_0);
    QTableWidgetItem *header_1 = new QTableWidgetItem(Sanguosha->translate("WINRATE_WIN"));
    ui->tableWidget->setHorizontalHeaderItem(1, header_1);
    QTableWidgetItem *header_2 = new QTableWidgetItem(Sanguosha->translate("WINRATE_PICK"));
    ui->tableWidget->setHorizontalHeaderItem(2, header_2);
    QTableWidgetItem *header_3 = new QTableWidgetItem(Sanguosha->translate("WINRATE_PERSENT"));
    ui->tableWidget->setHorizontalHeaderItem(3, header_3);

    int counter = 0;
    foreach (QString data_read_line, data_read_lines) {
        QString name = data_read_line.split("=").at(0);
        int win = data_read_line.split("=").at(1).split("/").at(0).toInt();
        int pick = data_read_line.split("=").at(1).split("/").at(1).toInt();
        int persent = (pick > 0) ? qRound(100.0*win/pick) : 0;

        if (name == "GameTimes") {
            ui->totalLineEdit->setText(QString::number(pick));
            ui->totalwinLineEdit->setText(QString::number(win));
            ui->totalrateLineEdit->setText(QString::number(persent)+"%");
        } else if ((ui->set_ignore->checkState() == Qt::Unchecked || pick >= ui->ignore_lessthan->value()) &&
                   (ui->set_ratemin->checkState() == Qt::Unchecked || persent >= ui->ratemin->value()) &&
                   (ui->set_ratemax->checkState() == Qt::Unchecked || persent <= ui->ratemax->value())) {
            QTableWidgetItem *name_item = new QTableWidgetItem(Sanguosha->translate(name));
            name_item->setTextAlignment(Qt::AlignCenter);
            QTableWidgetItem *win_item = new QTableWidgetItem();
            win_item->setTextAlignment(Qt::AlignCenter);
            QTableWidgetItem *pick_item = new QTableWidgetItem();
            pick_item->setTextAlignment(Qt::AlignCenter);
            QTableWidgetItem *persent_item = new QTableWidgetItem();
            persent_item->setTextAlignment(Qt::AlignCenter);

            //for transform data
            name_item->setData(Qt::UserRole, name);
            //for sort by number
            win_item->setData(Qt::DisplayRole,win);
            pick_item->setData(Qt::DisplayRole,pick);
            persent_item->setData(Qt::DisplayRole,persent);

            ui->tableWidget->setItem(counter, 0, name_item);
            ui->tableWidget->setItem(counter, 1, win_item);
            ui->tableWidget->setItem(counter, 2, pick_item);
            ui->tableWidget->setItem(counter, 3, persent_item);

            ui->tableWidget->setRowHeight(counter, 30);

            counter++;
        }
    }

    ui->counterLineEdit->setText(QString::number(counter));
    ui->tableWidget->setRowCount(counter);

    ui->tableWidget->setColumnWidth(0, 105);
    ui->tableWidget->setColumnWidth(1, 75);
    ui->tableWidget->setColumnWidth(2, 75);
    ui->tableWidget->setColumnWidth(3, 75);

    ui->tableWidget->setCurrentItem(ui->tableWidget->item(0, 0));
}

void WinrateViewDialog::refresh_skill_text()
{
    ui->skillTextEdit->clear();

    int row = ui->tableWidget->currentRow();
    QString name = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toString();
    if (Sanguosha->getGeneral(name))
        ui->skillTextEdit->append(Sanguosha->getGeneral(name)->getSkillDescription(true));
}

void WinrateViewDialog::move_to(QString name)
{
    int rows = ui->tableWidget->rowCount();
    for (int i = 0; i < rows; i++) {
        if (name == ui->tableWidget->item(i, 0)->data(Qt::UserRole).toString())
            ui->tableWidget->setCurrentItem(ui->tableWidget->item(i, 0));
    }
}

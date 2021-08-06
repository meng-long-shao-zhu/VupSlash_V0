#ifndef _KNOWNCARD_VIEW_DIALOG_H
#define _KNOWNCARD_VIEW_DIALOG_H

class ClientPlayer;

class KnowncardViewDialogUI;

class KnowncardViewDialog : public QDialog
{
    Q_OBJECT

public:
    KnowncardViewDialog(QWidget *parent = 0);
    ~KnowncardViewDialog();

private:
    KnowncardViewDialogUI *ui;

private slots:
    void refreshCards();
};

#endif


#ifndef EXITEDITWINDOW_H
#define EXITEDITWINDOW_H

#include <QDialog>
#include "level.h"
#include "hexspinbox.h"

namespace Ui {
class ExitEditWindow;
}

class ExitEditWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ExitEditWindow(QWidget *parent, exit_t *exit);
    ~ExitEditWindow();

protected:
    void accept();

private:
    Ui::ExitEditWindow *ui;
    HexSpinBox *spinBox_Level;

    exit_t *exit;
};

#endif // EXITEDITWINDOW_H

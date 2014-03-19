#include "level.h"
#include "hexspinbox.h"
#include "stuff.h"

#include "exiteditwindow.h"
#include "ui_exiteditwindow.h"

ExitEditWindow::ExitEditWindow(QWidget *parent, exit_t *exit) :
    QDialog(parent, Qt::CustomizeWindowHint
            | Qt::WindowTitleHint
            | Qt::WindowCloseButtonHint
            | Qt::MSWindowsFixedSizeDialogHint),
    ui(new Ui::ExitEditWindow),
    spinBox_Level(new HexSpinBox(this, 3)),
    exit(exit)
{
    ui->setupUi(this);

    this->spinBox_Level->setMaximum(NUM_LEVELS - 1);
    ui->gridLayout->addWidget(spinBox_Level, 0, 1, 1, 1);
    QWidget::setTabOrder(this->spinBox_Level, ui->spinBox_Screen);

    for (StringMap::const_iterator i = exitTypes.begin(); i != exitTypes.end(); i++) {
            ui->comboBox_Type->addItem(i->second, i->first);
    }

    this->spinBox_Level->setValue(exit->dest);
    ui->spinBox_Screen->setValue(exit->destScreen);
    ui->spinBox_X->setValue(exit->destX);
    ui->spinBox_Y->setValue(exit->destY);
    ui->comboBox_Type->setCurrentIndex(std::distance(exitTypes.begin(),
                                                     exitTypes.find(exit->type)));
}

ExitEditWindow::~ExitEditWindow()
{
    delete ui;
}

void ExitEditWindow::accept() {
    this->exit->dest = this->spinBox_Level->value();
    this->exit->destScreen = ui->spinBox_Screen->value();
    this->exit->destX = ui->spinBox_X->value();
    this->exit->destY = ui->spinBox_Y->value();
    this->exit->type = ui->comboBox_Type->itemData(ui->comboBox_Type->currentIndex()).toUInt();

    QDialog::accept();
}

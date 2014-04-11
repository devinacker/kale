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
    spinBox_BossLevel(new HexSpinBox(this, 3)),
    exit(exit)
{
    ui->setupUi(this);

    this->spinBox_Level->setMaximum(NUM_LEVELS - 1);
    ui->gridLayout->addWidget(spinBox_Level, 0, 1, 1, 1);
    QWidget::setTabOrder(this->spinBox_Level, ui->spinBox_Screen);
    QWidget::setTabOrder(ui->spinBox_Screen, ui->spinBox_X);
    QWidget::setTabOrder(ui->spinBox_X, ui->spinBox_Y);
    QWidget::setTabOrder(ui->spinBox_Y, ui->comboBox_Type);
    QWidget::setTabOrder(ui->comboBox_Type, this->spinBox_BossLevel);

    this->spinBox_BossLevel->setMaximum(NUM_LEVELS - 1);
    ui->gridLayout->addWidget(spinBox_BossLevel, 3, 1, 1, 1);
    QWidget::setTabOrder(this->spinBox_BossLevel, ui->spinBox_BossScreen);
    QWidget::setTabOrder(ui->spinBox_BossScreen, ui->spinBox_BossX);
    QWidget::setTabOrder(ui->spinBox_BossX, ui->spinBox_BossY);
    QWidget::setTabOrder(ui->spinBox_BossY, ui->buttonBox);

    QObject::connect(ui->comboBox_Type, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(enableBossInfo(int)));

    for (StringMap::const_iterator i = exitTypes.begin(); i != exitTypes.end(); i++) {
            ui->comboBox_Type->addItem(i->second, i->first);
    }

    this->spinBox_Level->setValue(exit->dest);
    ui->spinBox_Screen->setValue(exit->destScreen);
    ui->spinBox_X->setValue(exit->destX);
    ui->spinBox_Y->setValue(exit->destY);
    ui->comboBox_Type->setCurrentIndex(std::distance(exitTypes.begin(),
                                                     exitTypes.find(exit->type)));

    this->spinBox_BossLevel->setValue(exit->bossLevel);
    ui->spinBox_BossScreen->setValue(exit->bossScreen);
    ui->spinBox_BossX->setValue(exit->bossX);
    ui->spinBox_BossY->setValue(exit->bossY);
}

ExitEditWindow::~ExitEditWindow()
{
    delete ui;
    delete spinBox_Level;
    delete spinBox_BossLevel;
}

void ExitEditWindow::enableBossInfo(int index) {
    uint type = ui->comboBox_Type->itemData(index).toUInt();
    bool boss = type == 0x1F;

    ui->label_BossStuff->setEnabled(boss);
    ui->label_BossLevel->setEnabled(boss);
    this->spinBox_BossLevel->setEnabled(boss);
    ui->label_BossScreen->setEnabled(boss);
    ui->spinBox_BossScreen->setEnabled(boss);
    ui->label_BossX->setEnabled(boss);
    ui->spinBox_BossX->setEnabled(boss);
    ui->label_BossY->setEnabled(boss);
    ui->spinBox_BossY->setEnabled(boss);
}

void ExitEditWindow::accept() {
    this->exit->dest = this->spinBox_Level->value();
    this->exit->destScreen = ui->spinBox_Screen->value();
    this->exit->destX = ui->spinBox_X->value();
    this->exit->destY = ui->spinBox_Y->value();
    this->exit->type = ui->comboBox_Type->itemData(ui->comboBox_Type->currentIndex()).toUInt();

    this->exit->bossLevel = this->spinBox_BossLevel->value();
    this->exit->bossScreen = ui->spinBox_BossScreen->value();
    this->exit->bossX = ui->spinBox_BossX->value();
    this->exit->bossY = ui->spinBox_BossY->value();

    QDialog::accept();
}

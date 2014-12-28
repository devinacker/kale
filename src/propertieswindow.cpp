/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#include <QMessageBox>
#include <algorithm>
#include "propertieswindow.h"
#include "ui_propertieswindow.h"
#include "stuff.h"
#include "level.h"
#include "graphics.h"
#include "hexspinbox.h"
#include "tileset.h"
#include "tilesetview.h"

PropertiesWindow::PropertiesWindow(QWidget *parent, const QPixmap *tileset) :
    QDialog(parent, Qt::CustomizeWindowHint
            | Qt::WindowTitleHint
            | Qt::WindowCloseButtonHint
            | Qt::MSWindowsFixedSizeDialogHint
           ),
    ui(new Ui::PropertiesWindow),
    tileBox(new HexSpinBox(this, 2)),
    tilePalBox(new HexSpinBox(this, 2)),
    spriteBox(new HexSpinBox(this, 2)),
    spritePalBox(new HexSpinBox(this, 2)),
    doorTopBox(new HexSpinBox(this, 2)),
    doorBottomBox(new HexSpinBox(this, 2)),
    tileView(new TilesetView(this, tileset)),
    spritesView(new SpritesView(this))
{
    ui->setupUi(this);

    // add spinboxes
    QGridLayout *layout = ui->gridLayout_Room;
    layout->addWidget(tileBox,      2, 2, 1, 1);
    tileBox->setMaximum(NUM_TILESETS - 1);
    QWidget::setTabOrder(ui->slider_AnimSpeed, tileBox);
    layout->addWidget(tilePalBox,   2, 5, 1, 1);
    tilePalBox->setMaximum(255);
    QWidget::setTabOrder(tileBox, tilePalBox);
    layout->addWidget(spriteBox,    3, 2, 1, 1);
    spriteBox->setMaximum(255);
    // sprites use two consecutive banks at once
    spriteBox->setSingleStep(2);
    QWidget::setTabOrder(tilePalBox, spriteBox);
    layout->addWidget(spritePalBox, 3, 5, 1, 1);
    spritePalBox->setMaximum(49);
    QWidget::setTabOrder(spriteBox, spritePalBox);

    layout = ui->gridLayout_Extra;
    layout->addWidget(doorTopBox, 3, 1, 1, 1);
    doorTopBox->setMaximum(255);
    QWidget::setTabOrder(ui->spinBox_DoorY, doorTopBox);
    layout->addWidget(doorBottomBox, 3, 4, 1, 1);
    doorBottomBox->setMaximum(255);
    QWidget::setTabOrder(doorTopBox, doorBottomBox);

    // add tile & sprite views
    layout = ui->tilesetTabLayout;
    layout->addWidget(tileView, 0, 0, 1, 1);
    layout = ui->spritesTabLayout;
    layout->addWidget(spritesView, 0, 0, 1, 1);

    // set initial tab index
    ui->tabWidget_View->setCurrentIndex(0);
    ui->tabWidget_Properties->setCurrentIndex(0);

    // add music names to other dropdown
    for (StringMap::const_iterator i = musicNames.begin(); i != musicNames.end(); i++) {
        ui->comboBox_Music->addItem(i->second, i->first);
    }

    // set up signals to automatically apply changes
    QObject::connect(ui->comboBox_TileGFX, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(applyChange()));
    QObject::connect(ui->comboBox_TileGFX, SIGNAL(currentIndexChanged(int)),
                     tileView, SLOT(update()));
    QObject::connect(this->tileBox, SIGNAL(valueChanged(int)),
                     this, SLOT(applyChange()));
    QObject::connect(this->tileBox, SIGNAL(valueChanged(int)),
                     tileView, SLOT(update()));
    QObject::connect(this->tilePalBox, SIGNAL(valueChanged(int)),
                     this, SLOT(applyChange()));
    QObject::connect(this->tilePalBox, SIGNAL(valueChanged(int)),
                     tileView, SLOT(update()));
    QObject::connect(this->spriteBox, SIGNAL(valueChanged(int)),
                     this, SLOT(applyChange()));
    QObject::connect(this->spriteBox, SIGNAL(valueChanged(int)),
                     this->spritesView, SLOT(setBank(int)));
    QObject::connect(this->spritePalBox, SIGNAL(valueChanged(int)),
                     this, SLOT(applyChange()));
    QObject::connect(this->spritePalBox, SIGNAL(valueChanged(int)),
                     this->spritesView, SLOT(setPalette(int)));
    QObject::connect(ui->slider_AnimSpeed, SIGNAL(valueChanged(int)),
                     this, SLOT(applySpeed(int)));
    QObject::connect(ui->spinBox_Height, SIGNAL(valueChanged(int)),
                     this, SLOT(applyChange()));
    QObject::connect(ui->spinBox_Width, SIGNAL(valueChanged(int)),
                     this, SLOT(applyChange()));

    // set up signals to handle width/length constraints
    QObject::connect(ui->spinBox_Height, SIGNAL(valueChanged(int)),
                     this, SLOT(setMaxLevelWidth(int)));
    QObject::connect(ui->spinBox_Width, SIGNAL(valueChanged(int)),
                     this, SLOT(setMaxLevelHeight(int)));
}

PropertiesWindow::~PropertiesWindow()
{
    delete ui;
}

void PropertiesWindow::setMaxLevelWidth(int height) {
    ui->spinBox_Width->setMaximum(16 / height);
}

void PropertiesWindow::setMaxLevelHeight(int width) {
    ui->spinBox_Height->setMaximum(16 / width);
}

void PropertiesWindow::startEdit(leveldata_t *level) {
    this->level = NULL;

    // add graphics indices to dropdown
    ui->comboBox_TileGFX->clear();
    for (int i = 0; i < 256; i++)
        ui->comboBox_TileGFX->addItem(hexFormat(i, 2)
                                         + ": banks "
                                         + hexFormat(bankTable[0][i], 2) + ", "
                                         + hexFormat(bankTable[1][i], 2) + ", "
                                         + hexFormat(bankTable[2][i], 2) + "-"
                                         + hexFormat((bankTable[2][i] + 3) & 0xFF, 2));


    // set graphics values
    ui->comboBox_TileGFX->setCurrentIndex(level->header.tileIndex);
    this->tileBox       ->setValue(level->tileset);
    this->tilePalBox    ->setValue(level->header.tilePal);
    this->spriteBox     ->setValue(level->header.sprIndex);
    this->spritePalBox  ->setValue(level->header.sprPal);

    // set slider initial value to the opposite of the level speed
    // (slower speed / further right, except for zero)
    if (level->header.animSpeed)
        ui->slider_AnimSpeed->setValue(ui->slider_AnimSpeed->maximum()
                                       - level->header.animSpeed + 1);
    else
        ui->slider_AnimSpeed->setValue(0);

    // set height and width values
    ui->spinBox_Height->setValue(level->header.screensV);

    ui->spinBox_Width ->setValue(level->header.screensH);

    // set music value
    ui->comboBox_Music->setCurrentIndex(std::distance(musicNames.begin(),
                                                      musicNames.find(level->header.music)));

    // set no return value
    ui->checkBox_NoReturn->setCheckState(level->noReturn ? Qt::Checked : Qt::Unchecked);

    // disable extra tab for unpatched roms
    ui->extraTab->setEnabled(leveldata_t::hasExtra);

    // set extra properties
    ui->comboBox_Wind->setCurrentIndex(level->extra.wind);
    ui->spinBox_BossCount->setValue(level->extra.bossCount);
    ui->comboBox_BossAct->setCurrentIndex(level->extra.lock ? 1 : 0);
    ui->spinBox_DoorX->setValue(level->extra.doorX);
    ui->spinBox_DoorY->setValue(level->extra.doorY);
    this->doorTopBox->setValue(level->extra.doorTop);
    this->doorBottomBox->setValue(level->extra.doorBottom);
    ui->spinBox_ScreenLock->setValue(level->extra.lockPos);

    // save pointer
    this->level = level;
    // and original data, in case user cancels
    this->header  = level->header;
    this->tileset = level->tileset;

    this->exec();
}

void PropertiesWindow::applySpeed(int speed) {
    if (speed) {
        speed = ui->slider_AnimSpeed->maximum() - speed + 1;
        ui->label_FrameLength->setText(QString("%1 frame%2").arg(speed)
                                       .arg(speed > 1 ? "s" : ""));
    } else
        ui->label_FrameLength->setText("none");

    if (level) {
        level->header.animSpeed = speed;
        emit speedChanged(speed);
    }
}

void PropertiesWindow::applyChange() {
    if (!level) return;

    level->header.tileIndex = ui->comboBox_TileGFX->currentIndex();
    level->header.tilePal   = this->tilePalBox->value();
    level->tileset          = this->tileBox->value();
    level->header.sprIndex  = this->spriteBox->value();
    level->header.sprPal    = this->spritePalBox->value();

    // apply level size
    level->header.screensV = ui->spinBox_Height->value();
    level->header.screensH = ui->spinBox_Width ->value();

    emit changed();
}

void PropertiesWindow::accept() {
    // level graphics and size settings have already been applied by applyChange slot

    // apply music setting
    level->header.music = ui->comboBox_Music->itemData(ui->comboBox_Music->currentIndex()).toUInt();

    // apply return flag
    level->noReturn     = ui->checkBox_NoReturn->checkState() == Qt::Checked;

    // apply extra settings
    level->extra.wind = ui->comboBox_Wind->currentIndex();
    level->extra.bossCount = ui->spinBox_BossCount->value();
    level->extra.lock = ui->comboBox_BossAct->currentIndex() > 0;
    level->extra.doorX = ui->spinBox_DoorX->value();
    level->extra.doorY = ui->spinBox_DoorY->value();
    level->extra.doorTop = this->doorTopBox->value();
    level->extra.doorBottom = this->doorBottomBox->value();
    level->extra.lockPos = ui->spinBox_ScreenLock->value();

    level->modified = true;

    QDialog::accept();
}

// discard settings
void PropertiesWindow::reject() {
    // return to original settings
    level->header  = this->header;
    level->tileset = this->tileset;

    emit changed();
    QDialog::reject();
}

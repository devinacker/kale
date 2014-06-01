/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#include <QMessageBox>
#include <QPainter>
#include <algorithm>
#include "tileseteditwindow.h"
#include "ui_tileseteditwindow.h"
#include "stuff.h"
#include "level.h"
#include "graphics.h"
#include "hexspinbox.h"
#include "tileset.h"
#include "tilesetview.h"

TilesetEditWindow::TilesetEditWindow(QWidget *parent) :
    QDialog(parent, Qt::CustomizeWindowHint
            | Qt::WindowTitleHint
            | Qt::WindowCloseButtonHint
            | Qt::MSWindowsFixedSizeDialogHint
           ),
    ui(new Ui::TilesetEditWindow),
    tilesetBox(new HexSpinBox(this, 2)),
    tilePalBox(new HexSpinBox(this, 2)),
    subtractBox(new HexSpinBox(this, 2)),
    tileView(new TilesetView(this, &tilesetPixmap)),
    tile8View(new Tile8View(this, gfxBanks)),
    tilesetPixmap(256*TILE_SIZE, TILE_SIZE),
    animTimer(this), animFrame(0)
{
    ui->setupUi(this);

    // add spinboxes
    QGridLayout *layout = ui->gridLayout_Display;
    layout->addWidget(tilesetBox,      2, 2, 1, 1);
    tilesetBox->setMaximum(NUM_TILESETS - 1);
    QWidget::setTabOrder(ui->slider_AnimSpeed, tilesetBox);
    layout->addWidget(tilePalBox,   2, 5, 1, 1);
    tilePalBox->setMaximum(255);
    QWidget::setTabOrder(tilesetBox, tilePalBox);


    // add tile subtract box
    layout = ui->mainLayout;
    layout->addWidget(subtractBox, 2, 2, 1, 1);
    subtractBox->setMinimum(0);
    subtractBox->setMaximum(255);
    QWidget::setTabOrder(tilePalBox, subtractBox);

    // 8x8 tile selection boxes
    layout = ui->gridLayout_Tile;
    for (uint i = 0; i < 4; i++) {
        tileBoxes[i] = new HexSpinBox(this, 2);
        tileBoxes[i]->setMinimum(0);
        tileBoxes[i]->setMaximum(255);

        QObject::connect(tileBoxes[i], SIGNAL(valueChanged(int)),
                         this, SLOT(updateTile()));
    }
    layout->addWidget(tileBoxes[0], 0, 2, 1, 1);
    QWidget::setTabOrder(subtractBox, tileBoxes[0]);
    layout->addWidget(tileBoxes[1], 0, 4, 1, 1);
    QWidget::setTabOrder(tileBoxes[0], tileBoxes[1]);
    layout->addWidget(tileBoxes[2], 1, 2, 1, 1);
    QWidget::setTabOrder(tileBoxes[1], tileBoxes[2]);
    layout->addWidget(tileBoxes[3], 1, 4, 1, 1);
    QWidget::setTabOrder(tileBoxes[2], tileBoxes[3]);

    // add tile views
    layout = ui->tile16Layout;
    layout->addWidget(tileView);
    tileView->setMouseEnabled(true);
    layout = ui->tile8Layout;
    layout->addWidget(tile8View);


    // add tile behaviors to other dropdown
    for (StringMap::const_iterator i = tileTypes.begin(); i != tileTypes.end(); i++) {
        ui->comboBox_Behavior->addItem(i->second, i->first);
    }

    // set up signals to automatically apply changes
    QObject::connect(ui->comboBox_TileGFX, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(refreshPixmap()));
    QObject::connect(this->tilesetBox, SIGNAL(valueChanged(int)),
                     this, SLOT(setTileset(int)));
    QObject::connect(this->tilePalBox, SIGNAL(valueChanged(int)),
                     this, SLOT(refreshPixmap()));
    QObject::connect(ui->slider_AnimSpeed, SIGNAL(valueChanged(int)),
                     this, SLOT(applySpeed(int)));

    QObject::connect(ui->spinBox_Palette, SIGNAL(valueChanged(int)),
                     this, SLOT(updateTile()));
    QObject::connect(ui->spinBox_Palette, SIGNAL(valueChanged(int)),
                     tile8View, SLOT(setPalette(int)));
    QObject::connect(ui->comboBox_Behavior, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(updateTile()));

    QObject::connect(tileView, SIGNAL(tileSelected(int)),
                     this, SLOT(setTile(int)));

    QObject::connect(&animTimer, SIGNAL(timeout()),
                     this, SLOT(animate()));

    QObject::connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)),
                     this, SLOT(buttonClick(QAbstractButton*)));
}

TilesetEditWindow::~TilesetEditWindow()
{
    delete ui;
    delete tilesetBox;
    delete tilePalBox;
    delete subtractBox;
    delete tileView;
    delete tile8View;
    delete tileBoxes[0];
    delete tileBoxes[1];
    delete tileBoxes[2];
    delete tileBoxes[3];
}

void TilesetEditWindow::startEdit(const leveldata_t *level) {
    // add graphics indices to dropdown
    ui->comboBox_TileGFX->clear();
    for (int i = 0; i < 256; i++)
        ui->comboBox_TileGFX->addItem(QString::number(i, 16).rightJustified(2, QLatin1Char('0')).toUpper()
                                         + ": banks "
                                         + QString::number(bankTable[0][i], 16).rightJustified(2, QLatin1Char('0')).toUpper() + ", "
                                         + QString::number(bankTable[1][i], 16).rightJustified(2, QLatin1Char('0')).toUpper() + ", "
                                         + QString::number(bankTable[2][i], 16).rightJustified(2, QLatin1Char('0')).toUpper() + "-"
                                         + QString::number((bankTable[2][i] + 3) & 0xFF, 16).rightJustified(2, QLatin1Char('0')).toUpper());


    // set graphics values
    ui->comboBox_TileGFX->setCurrentIndex(level->header.tileIndex);
    this->tilesetBox    ->setValue(level->tileset);
    this->tilePalBox    ->setValue(level->header.tilePal);

    // set slider initial value to the opposite of the level speed
    // (slower speed / further right, except for zero)
    if (level->header.animSpeed)
        ui->slider_AnimSpeed->setValue(ui->slider_AnimSpeed->maximum()
                                       - level->header.animSpeed + 1);
    else
        ui->slider_AnimSpeed->setValue(0);

    // make editing copies of tilesets
    for (uint i = 0; i < NUM_TILESETS; i++)
        memcpy(tempTilesets[i], tilesets[i], 0x100 * sizeof(metatile_t));

    memcpy(tempTileSubtract, tileSubtract, NUM_TILESETS * sizeof(uint8_t));

    setTileset(level->tileset);
    setTile(0);

    this->exec();
}

void TilesetEditWindow::refreshPixmap() {
    // update CHR banks
    uint chr = ui->comboBox_TileGFX->currentIndex();
    uint pal = tilePalBox->value();

    gfxBanks[0] = getCHRBank(0, pal);
    gfxBanks[1] = getCHRBank(bankTable[0][chr], pal);
    gfxBanks[2] = getCHRBank(bankTable[1][chr], pal);
    gfxBanks[3] = getCHRBank(bankTable[2][chr] + animFrame, pal);

    QPainter painter(&tilesetPixmap);

    for (uint i = 0; i < 256; i++) {
        metatile_t thisTile = tempTilesets[tileset][i];
        uint palette = thisTile.palette;

        uint srcX = i * 16;

        QRect destRect(0, 0, 8, 8);
        QRect srcRect(0, 8 * palette, 8, 8);

        // upper left
        destRect.moveTopLeft(QPoint(srcX, 0));
        srcRect.moveLeft(8 * (thisTile.ul % 64));
        painter.drawImage(destRect, gfxBanks[thisTile.ul / 64], srcRect);

        // upper right
        destRect.moveTopLeft(QPoint(srcX + 8, 0));
        srcRect.moveLeft(8 * (thisTile.ur % 64));
        painter.drawImage(destRect, gfxBanks[thisTile.ur / 64], srcRect);

        // lower left
        destRect.moveTopLeft(QPoint(srcX, 8));
        srcRect.moveLeft(8 * (thisTile.ll % 64));
        painter.drawImage(destRect, gfxBanks[thisTile.ll / 64], srcRect);

        // lower right
        destRect.moveTopLeft(QPoint(srcX + 8, 8));
        srcRect.moveLeft(8 * (thisTile.lr % 64));
        painter.drawImage(destRect, gfxBanks[thisTile.lr / 64], srcRect);
    }
}

void TilesetEditWindow::applySpeed(int speed) {
    // set up tile animation
    if (speed) {
        speed = ui->slider_AnimSpeed->maximum() - speed + 1;

        // frame length (NTSC frames -> msec)
        uint timeout = speed * 16;

        ui->label_FrameLength->setText(QString("%1 frame%2").arg(speed)
                                       .arg(speed > 1 ? "s" : ""));
        animTimer.start(timeout);
    } else {
        ui->label_FrameLength->setText("none");
        animFrame = 0;
        animTimer.stop();
        refreshPixmap();
    }
}

// advance to next animation frame
void TilesetEditWindow::animate() {
    ++animFrame &= 3;
    refreshPixmap();
}

void TilesetEditWindow::setTileset(int tileset) {
    this->tileset = tileset;

    subtractBox->setValue(tempTileSubtract[tileset]);
    subtractBox->setEnabled(tileset < NUM_TILESETS_INGAME);

    refreshPixmap();
}

void TilesetEditWindow::setTile(int tile) {
    currentTile = tile;

    ui->groupBox_Tile->setTitle(QString("Tile %1 Properties")
                                .arg(QString::number(tile, 16).rightJustified(2, QLatin1Char('0')).toUpper()));

    // TODO: all of the stuff
    metatile_t thisTile = tempTilesets[tileset][tile];
    // set tile parts
    tileBoxes[0]->setValue(thisTile.ul);
    tileBoxes[1]->setValue(thisTile.ur);
    tileBoxes[2]->setValue(thisTile.ll);
    tileBoxes[3]->setValue(thisTile.lr);
    // set behavior value
    ui->comboBox_Behavior->setCurrentIndex(std::distance(tileTypes.begin(),
                                                         tileTypes.find(thisTile.action)));
    // and palette value
    ui->spinBox_Palette->setValue(thisTile.palette);
}

void TilesetEditWindow::updateTile() {
    metatile_t *thisTile = &tempTilesets[tileset][currentTile];

    // set tile parts
    thisTile->ul = tileBoxes[0]->value();
    thisTile->ur = tileBoxes[1]->value();
    thisTile->ll = tileBoxes[2]->value();
    thisTile->lr = tileBoxes[3]->value();

    // set behavior value
    thisTile->action = ui->comboBox_Behavior->itemData(ui->comboBox_Behavior->currentIndex()).toUInt();

    // set palette
    thisTile->palette = ui->spinBox_Palette->value();

    refreshPixmap();
}

void TilesetEditWindow::applyChange() {
    // save changes to tilesets
    for (uint i = 0; i < NUM_TILESETS; i++)
        memcpy(tilesets[i], tempTilesets[i], 0x100 * sizeof(metatile_t));

    memcpy(tileSubtract, tempTileSubtract, NUM_TILESETS * sizeof(uint8_t));

    emit changed();
}

// handle buttons without AcceptRole/RejectRole
void TilesetEditWindow::buttonClick(QAbstractButton *button) {
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole) {
        applyChange();
    }
}

void TilesetEditWindow::accept() {
    applyChange();

    QDialog::accept();
}

Tile8View::Tile8View(QWidget *parent, const QImage *banks) :
    TilesetView(parent, NULL),
    gfxBanks(banks),
    palette(0)
{}

void Tile8View::setPalette(int palette) {
    this->palette = palette;
    update();
}

void Tile8View::paintEvent(QPaintEvent *event) {
    // assign a painter to the widget
    QPainter painter(this);
    QRect rect = event->rect();

    uint tile = 0;
    for (int h = rect.top() / TILE_SIZE; tile < 256 && h <= rect.bottom() / TILE_SIZE; h++) {
        for (int w = rect.left() / TILE_SIZE; tile < 256 && w <= rect.right() / TILE_SIZE; w++) {
            QRect destRect(w * TILE_SIZE, h * TILE_SIZE, TILE_SIZE, TILE_SIZE);
            QRect srcRect ((tile % 64) * 8, palette * 8, 8, 8);
            painter.drawImage(destRect, gfxBanks[tile / 64], srcRect);

            tile++;
        }
    }

    if (currTile >= 0)
        painter.fillRect((currTile % 16) * TILE_SIZE, (currTile / 16) * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                         TilesetView::infoColor);
}

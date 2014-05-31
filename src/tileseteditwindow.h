/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#ifndef TILESETEDITWINDOW_H
#define TILESETEDITWINDOW_H

#include <QDialog>
#include <QTimer>
#include <QAbstractButton>
#include <cstdint>
#include "level.h"
#include "tileset.h"
#include "hexspinbox.h"
#include "tilesetview.h"

namespace Ui {
class TilesetEditWindow;
}

class TilesetEditWindow : public QDialog
{
    Q_OBJECT
    
public:
    explicit TilesetEditWindow(QWidget *parent);
    ~TilesetEditWindow();

    void startEdit(const leveldata_t *level);
    
public slots:
    void applySpeed(int);
    void animate();
    void setTile(int);
    void updateTile();
    void applyChange();
    void buttonClick(QAbstractButton*);

private:
    Ui::TilesetEditWindow *ui;

    HexSpinBox *tilesetBox, *tilePalBox;
    HexSpinBox *tileBoxes[4];
    TilesetView *tileView;

    QPixmap tilesetPixmap;
    QImage gfxBanks[4];
    uint tileset, currentTile;

    QTimer animTimer;
    uint animFrame;
    metatile_t tempTilesets[NUM_TILESETS][0x100];

private slots:
    void accept();
    void refreshPixmap();

signals:
    void changed();
    void speedChanged(int);
};

#endif // TILESETEDITWINDOW_H

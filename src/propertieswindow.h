/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#ifndef PROPERTIESWINDOW_H
#define PROPERTIESWINDOW_H

#include <QDialog>
#include <cstdint>
#include "level.h"
#include "hexspinbox.h"
#include "tilesetview.h"

namespace Ui {
class PropertiesWindow;
}

class PropertiesWindow : public QDialog
{
    Q_OBJECT
    
public:
    explicit PropertiesWindow(QWidget *parent, const QPixmap *tileset);
    ~PropertiesWindow();

    void startEdit(leveldata_t *level);
    
public slots:
    void applySpeed(int);
    void applyChange();
    void applySpriteColor();
    void setMaxLevelWidth(int);
    void setMaxLevelHeight(int);

private:
    Ui::PropertiesWindow *ui;

    leveldata_t *level;
    header_t header;
    uint8_t  tileset;

    HexSpinBox *tileBox, *tilePalBox, *spriteBox, *spritePalBox;
    TilesetView *tileView;
    SpritesView *spritesView;

private slots:
    void accept();
    void reject();

signals:
    void changed();
    void speedChanged(int);
};

#endif // PROPERTIESWINDOW_H

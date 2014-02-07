#include "tilesetview.h"
#include "level.h"
#include "stuff.h"
#include "tileset.h"
#include "tileeditwindow.h"
#include "ui_tileeditwindow.h"

TileEditWindow::TileEditWindow(QWidget *parent, leveldata_t *level, QRect rect, const QPixmap *tiles) :
    QDialog(parent, Qt::CustomizeWindowHint
            | Qt::WindowTitleHint
            | Qt::WindowCloseButtonHint
            | Qt::MSWindowsFixedSizeDialogHint),
    ui(new Ui::TileEditWindow),
    level(level),
    area(rect),
    view(new TilesetView(this, tiles))
{
    ui->setupUi(this);

    ui->verticalLayout->insertWidget(0, view);
    view->setMouseEnabled(true);

    setTileLabel(-1);

    QObject::connect(view, SIGNAL(tileHovered(int)),
                     this, SLOT(setTileLabel(int)));
    QObject::connect(view, SIGNAL(tileSelected(int)),
                     this, SLOT(accept(int)));
}

TileEditWindow::~TileEditWindow()
{
    delete ui;
    delete view;
}

void TileEditWindow::setTileLabel(int tile) {
    if (tile < 0)
        ui->label->setText("");
    else
        ui->label->setText(QString("Tile %1 (%2)").arg(tile).arg(tileType(tilesets[level->tileset][tile].action)));
}

void TileEditWindow::accept(int tile) {
    if (tile >= 0) {
        for (uint x = area.left(); x <= area.right(); x++)
            for (uint y = area.top(); y <= area.bottom(); y++)
                level->tiles[y][x] = tile;
    }

    QDialog::accept();
}

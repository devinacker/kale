#include "mapclear.h"
#include "romfile.h"
#include "level.h"
#include <cstdint>
#include <cstdio>

std::vector<QRect> mapClearData[7][16];

const romaddr_t ptrMapClearL = {0x12, 0x9C7E};
const romaddr_t ptrMapClearH = {0x12, 0x9CEE};
const uint      ptrMapClearB = 0x12;

void loadMapClearData(ROMFile& rom, uint map, uint width) {
    for (uint level = 0; level < 16; level++) {
        mapClearData[map][level].clear();

        uint8_t bytes[4] = {0};
        romaddr_t addr = rom.readShortPointer(ptrMapClearL, ptrMapClearH, ptrMapClearB,
                                              (map * 16) + level);
        if (!addr.addr) continue;

        do {
            rom.readBytes(addr, 4, bytes);

            addr.addr += 4;

            uint screen = bytes[0] & 0xF;
            uint x = (screen % width * SCREEN_WIDTH) + (bytes[1] & 0xF);
            uint y = (screen / width * SCREEN_HEIGHT) + (bytes[1] >> 4);

            mapClearData[map][level].push_back(QRect(x, y, bytes[2], bytes[3]));

        } while (bytes[0] < 0x80);
    }
}


void saveMapClearData(ROMFile&);

MapClearModel::MapClearModel(QObject *parent, std::vector<QRect> *rects) :
    QAbstractTableModel(parent),
    level(0),
    rects(rects)
{}

int MapClearModel::rowCount(const QModelIndex&) const {
    // return number of rects for this stage
    if (rects)
        return rects->size();

    return 0;
}

int MapClearModel::columnCount(const QModelIndex&) const {
    return 4;
}

QVariant MapClearModel::data(const QModelIndex& index, int role) const {
    switch (role) {
    case Qt::DisplayRole:
        if (!rects)
            return 0;

        int col = index.column();
        int row = index.row();
        QRect rect = rects->at(row);

        switch (col) {
        case columnX:
            return rect.x();

        case columnY:
            return rect.y();

        case columnWidth:
            return rect.width();

        case columnHeight:
            return rect.height();
        }

        break;
    }

    return QVariant();
}

QVariant MapClearModel::headerData(int section, Qt::Orientation orientation, int role) const {
    switch (role) {
    case Qt::DisplayRole:
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case columnX:
                return QString("X");

            case columnY:
                return QString("Y");

            case columnWidth:
                return QString("Width");

            case columnHeight:
                return QString("Height");
            }
        } else {
            return section + 1;
        }

        break;
    }

    return QVariant();
}

void MapClearModel::setLevel(uint level) {
    if (level < 7) {
        this->level = level;
        setLevelIndex(0);
    }
}

void MapClearModel::setLevelIndex(uint index) {
    if (index < 0x10) {
        beginResetModel();
        if (rects)
            *rects = mapClearData[level][index];
        endResetModel();
    }
}

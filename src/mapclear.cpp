#include "mapclear.h"
#include "romfile.h"
#include "level.h"
#include <cstdint>
#include <cstdio>
#include <QSpinBox>

std::vector<QRect> mapClearData[7][16];

const romaddr_t ptrMapClearL  = {0x12, 0x9C7E};
const romaddr_t ptrMapClearH  = {0x12, 0x9CEE};
const uint      ptrMapClearB  = 0x12;
const romaddr_t mapClearStart = {0x12, 0x9D5E};

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


void saveMapClearData(ROMFile& rom, const leveldata_t *levelData, uint num) {

    romaddr_t addr = mapClearStart;
    // get address to write clear data to based on how many rects were written
    // for previous levels
    for (uint map = 0; map < num; map++)
        for (uint level = 0; level < 0x10; level++)
            addr.addr += 4 * mapClearData[map][level].size();


    for (uint level = 0; level < 0x10; level++) {
        std::vector<QRect>& rects = mapClearData[num][level];

        if (!rects.size()) {
            rom.writeToShortPointer(ptrMapClearL, ptrMapClearH, {0, 0}, 0, NULL, num * 16 + level);
            continue;
        }

        rom.writeToShortPointer(ptrMapClearL, ptrMapClearH, addr, 0, NULL, num * 16 + level);

        uint numRects = rects.size();
        for (std::vector<QRect>::const_iterator i = rects.begin(); i != rects.end(); i++) {
            uint8_t bytes[4];

            // byte 0: screen
            bytes[0] = (i->y() / SCREEN_HEIGHT * levelData->header.screensH)
                     + (i->x() / SCREEN_WIDTH);
            // and last rect flag
            if (--numRects == 0)
                bytes[0] |= 0x80;

            // byte 1: Y/X coords
            bytes[1] = ((i->y() % SCREEN_HEIGHT) << 4) | (i->x() % SCREEN_WIDTH);

            // bytes 2-3: width/height
            bytes[2] = i->width();
            bytes[3] = i->height();

            // write it
            rom.writeBytes(addr, 4, bytes);
            addr.addr += 4;
        }
    }
}

MapClearDelegate::MapClearDelegate(QObject *parent) :
    QItemDelegate(parent)
{}

QWidget* MapClearDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem&, const QModelIndex& index) const {
    QSpinBox *editor = new QSpinBox(parent);
    int col = index.column();

    editor->setMinimum(0);
    switch (col) {
    case MapClearModel::columnX:
        editor->setMaximum(16 * SCREEN_WIDTH - 1);
        break;

    case MapClearModel::columnY:
        editor->setMaximum(16 * SCREEN_HEIGHT - 1);
        break;

    default:
        editor->setMaximum(255);
        break;
    }

    return editor;
}

void MapClearDelegate::setEditorData(QWidget *editor, const QModelIndex& index) const {
    int val = index.model()->data(index).toInt();

    QSpinBox *box = static_cast<QSpinBox*>(editor);
    box->setValue(val);
}

void MapClearDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex& index) const {
    QSpinBox *box = static_cast<QSpinBox*>(editor);
    box->interpretText();

    model->setData(index, box->value());
}

void MapClearDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem& option, const QModelIndex &) const {
    editor->setGeometry(option.rect);
}

MapClearModel::MapClearModel(QObject *parent) :
    QAbstractTableModel(parent),
    level(0),
    rects(NULL)
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

Qt::ItemFlags MapClearModel::flags(const QModelIndex&) const {
    return Qt::ItemIsEnabled
         | Qt::ItemIsSelectable
         | Qt::ItemIsEditable;
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

        default:
            return QVariant();
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

            default:
                return QVariant();
            }
        } else {
            return section + 1;
        }

        break;
    }

    return QVariant();
}

bool MapClearModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::EditRole) {
        int row = index.row();
        int col = index.column();

        QRect& rect = rects->at(row);

        bool ok;
        uint val = value.toUInt(&ok);

        if (!ok) return false;

        switch (col) {
        case columnX:
            rect.moveLeft(val);
            break;

        case columnY:
            rect.moveTop(val);
            break;

        case columnWidth:
            rect.setWidth(val);
            break;

        case columnHeight:
            rect.setHeight(val);
            break;

        default:
            return false;
        }

        emit dataChanged(index, index);
        return true;
    }

    return false;
}

bool MapClearModel::insertRow(int row, const QModelIndex &parent) {
    if (rects && row < rects->size()) {
        beginInsertRows(parent, row, row);
        rects->insert(rects->begin() + row, QRect());
        endInsertRows();
        return true;

    } else if (rects) {
        beginInsertRows(parent, row, row);
        rects->push_back(QRect());
        endInsertRows();
        return true;
    }

    return false;
}

bool MapClearModel::removeRow(int row, const QModelIndex &parent) {
    if (rects && row < rects->size()) {
        beginRemoveRows(parent, row, row);
        rects->erase(rects->begin() + row);
        endRemoveRows();
        return true;

    }

    return false;
}

void MapClearModel::setRects(std::vector<QRect> *newRects) {
    if (newRects) {
        beginResetModel();
        rects = newRects;
        endResetModel();
    }
}

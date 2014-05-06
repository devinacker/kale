#ifndef MAPCLEAR_H
#define MAPCLEAR_H

#include <vector>
#include <QRect>
#include <QAbstractTableModel>
#include <QItemDelegate>
#include "romfile.h"
#include "level.h"

#define MAX_CLEAR_RECTS 77

extern std::vector<QRect> mapClearData[7][16];

void loadMapClearData(ROMFile&, uint, uint);
void saveMapClearData(ROMFile&, const leveldata_t*, uint);

class MapClearDelegate : public QItemDelegate {
    Q_OBJECT

public:
    MapClearDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const;

    void setEditorData(QWidget*, const QModelIndex&) const;
    void setModelData(QWidget*, QAbstractItemModel*, const QModelIndex&) const;

    void updateEditorGeometry(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const;
};

class MapClearModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum {
        columnX,
        columnY,
        columnWidth,
        columnHeight
    };

    MapClearModel(QObject*);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    bool insertRow(int row, const QModelIndex &parent = QModelIndex());
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());

    void setRects(std::vector<QRect>*);

private:
    uint level;
    std::vector<QRect> *rects;
};

#endif // MAPCLEAR_H

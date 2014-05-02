#ifndef MAPCLEAR_H
#define MAPCLEAR_H

#include <vector>
#include <QRect>
#include <QAbstractTableModel>
#include "romfile.h"

extern std::vector<QRect> mapClearData[7][16];

void loadMapClearData(ROMFile&, uint, uint);
void saveMapClearData(ROMFile&);

class MapClearModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum {
        columnX,
        columnY,
        columnWidth,
        columnHeight
    };

    MapClearModel(QObject*, std::vector<QRect>*);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void setLevel(uint);
    void setLevelIndex(uint);

private:
    uint level;
    std::vector<QRect> *rects;
};

#endif // MAPCLEAR_H

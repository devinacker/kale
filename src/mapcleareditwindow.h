#ifndef MAPCLEAREDITWINDOW_H
#define MAPCLEAREDITWINDOW_H

#include <QDialog>
#include <QItemSelectionModel>
#include "mapclear.h"

namespace Ui {
class MapClearEditWindow;
}

class MapClearEditWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MapClearEditWindow(QWidget *parent = 0);
    ~MapClearEditWindow();

    void setLevel(uint);

public slots:
    int exec();

protected slots:
    void setLevelIndex(int);
    void updateRects();
    void addRect();
    void deleteRects();
    void selectionChanged();

private:
    Ui::MapClearEditWindow *ui;
    MapClearModel *model;
    MapClearDelegate *delegate;
    uint level, levelIndex;
    std::vector<QRect> rects[0x10];

    uint rectCount;

private slots:
    void accept();
    void reject();

signals:
    void clearRectsChanged(const std::vector<QRect>*);
};

#endif // MAPCLEAREDITWINDOW_H

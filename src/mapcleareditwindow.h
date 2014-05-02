#ifndef MAPCLEAREDITWINDOW_H
#define MAPCLEAREDITWINDOW_H

#include <QDialog>
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

private:
    Ui::MapClearEditWindow *ui;
    MapClearModel *model;
    uint level;
    std::vector<QRect> rects;

private slots:
    void accept();
    void reject();

signals:
    void clearRectsChanged(const std::vector<QRect>*);
};

#endif // MAPCLEAREDITWINDOW_H

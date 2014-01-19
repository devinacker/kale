/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#ifndef PROPERTIESWINDOW_H
#define PROPERTIESWINDOW_H

#include <QDialog>
#include <cstdint>
#include "level.h"

namespace Ui {
class PropertiesWindow;
}

class PropertiesWindow : public QDialog
{
    Q_OBJECT
    
public:
    explicit PropertiesWindow(QWidget *parent = 0);
    ~PropertiesWindow();

    void startEdit(leveldata_t *level);
    
public slots:
    void applySpeed(int);
    void applyChange();
    void setMaxLevelWidth(int);
    void setMaxLevelLength(int);

private:
    Ui::PropertiesWindow *ui;

    leveldata_t *level;
    header_t header;
    uint8_t  tileset;

private slots:
    void accept();
    void reject();

signals:
    void changed();
    void speedChanged(int);
};

#endif // PROPERTIESWINDOW_H

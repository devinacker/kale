#ifndef PALETTEEDITWINDOW_H
#define PALETTEEDITWINDOW_H

#include <QDialog>
#include <QAbstractButton>
#include <QAbstractListModel>
#include <cstdint>
#include "hexspinbox.h"

namespace Ui {
class PaletteEditWindow;
}

class PaletteModel : public QAbstractListModel {
    Q_OBJECT

public:
    PaletteModel(QObject*, uint8_t (&pal)[10][256], uint8_t (&sprPal)[50][6]);
    int rowCount(const QModelIndex &parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    void setPalette(bool, uint);

private:
    bool spritePal;
    uint palette;
    uint8_t (&tempPalettes)[10][256];
    uint8_t (&tempSprPalettes)[50][6];
};

class PaletteEditWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PaletteEditWindow(QWidget *parent = 0);
    ~PaletteEditWindow();

    void applyChange();

signals:
    void changed();

public slots:
    void startEdit(uint, uint);
    void selectPalette(int pal);
    void selectPalType();
    void buttonClick(QAbstractButton*);

private slots:
    void accept();

private:
    Ui::PaletteEditWindow *ui;
    PaletteModel *model;
    HexSpinBox *spinBox;

    uint currPal, currSprPal;

    // working copies of the palettes in graphics.cpp
    uint8_t tempPalettes[10][256];
    uint8_t tempSprPalettes[50][6];
};

#endif // PALETTEEDITWINDOW_H

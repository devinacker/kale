#include "paletteeditwindow.h"
#include "ui_paletteeditwindow.h"
#include "graphics.h"

PaletteEditWindow::PaletteEditWindow(QWidget *parent) :
    QDialog(parent, Qt::CustomizeWindowHint
            | Qt::WindowTitleHint
            | Qt::WindowCloseButtonHint
            | Qt::MSWindowsFixedSizeDialogHint
           ),
    ui(new Ui::PaletteEditWindow),
    model(new PaletteModel(this, tempPalettes, tempSprPalettes)),
    spinBox(new HexSpinBox(this, 2))
{
    ui->setupUi(this);

    ui->gridLayout->addWidget(spinBox, 0, 4);
    QWidget::setTabOrder(ui->radioButton_Sprite, spinBox);

    ui->listView->setIconSize(QSize(32, 32));
    ui->listView->setModel(model);

    QObject::connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)),
                     this, SLOT(buttonClick(QAbstractButton*)));

    QObject::connect(ui->radioButton_BG, SIGNAL(clicked()),
                     this, SLOT(selectPalType()));
    QObject::connect(ui->radioButton_Sprite, SIGNAL(clicked()),
                     this, SLOT(selectPalType()));
    QObject::connect(spinBox, SIGNAL(valueChanged(int)),
                     this, SLOT(selectPalette(int)));
}

PaletteEditWindow::~PaletteEditWindow()
{
    delete ui;
    delete model;
    delete spinBox;
}

void PaletteEditWindow::startEdit(uint pal, uint sprPal) {
    // make working copies of palettes
    // background palettes
    for (uint i = 0; i < BG_PAL_SIZE; i++)
        memcpy(tempPalettes[i], palettes[i], BG_PAL_NUM);

    // sprite palettes
    for (uint i = 0; i < SPR_PAL_NUM; i++)
        memcpy(tempSprPalettes[i], sprPalettes[i], SPR_PAL_SIZE);

    currPal = pal;
    currSprPal = sprPal;

    selectPalType();
    this->exec();
}

void PaletteEditWindow::selectPalType() {
    if (ui->radioButton_BG->isChecked()) {
        spinBox->setMaximum(BG_PAL_NUM - 1);
        spinBox->setValue(currPal);

        model->setPalette(false, currPal);
    } else {
        spinBox->setMaximum(SPR_PAL_NUM - 1);
        spinBox->setValue(currSprPal);

        model->setPalette(true, currSprPal);
    }
}

void PaletteEditWindow::selectPalette(int pal) {
    if (ui->radioButton_BG->isChecked()) {
        currPal = pal;
        model->setPalette(false, pal);
    } else {
        currSprPal = pal;
        model->setPalette(true, pal);
    }
}

void PaletteEditWindow::applyChange() {
    // apply changes to palettes
    // background palettes
    for (uint i = 0; i < BG_PAL_SIZE; i++)
        memcpy(palettes[i], tempPalettes[i], BG_PAL_NUM);

    // sprite palettes
    for (uint i = 0; i < SPR_PAL_NUM; i++)
        memcpy(sprPalettes[i], tempSprPalettes[i], SPR_PAL_SIZE);

    emit changed();
}

// handle buttons without AcceptRole/RejectRole
void PaletteEditWindow::buttonClick(QAbstractButton *button) {
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole) {
        applyChange();
    }
}

void PaletteEditWindow::accept() {
    applyChange();

    QDialog::accept();
}

PaletteModel::PaletteModel(QObject *parent, uint8_t (&pal)[BG_PAL_SIZE][BG_PAL_NUM],
                                            uint8_t (&sprPal)[SPR_PAL_NUM][SPR_PAL_SIZE]) :
    QAbstractListModel(parent),
    spritePal(false),
    palette(0),
    tempPalettes(pal),
    tempSprPalettes(sprPal)
{}

int PaletteModel::rowCount(const QModelIndex &) const {
    if (spritePal)
        return SPR_PAL_SIZE;

    return BG_PAL_SIZE;
}

Qt::ItemFlags PaletteModel::flags(const QModelIndex &) const {
    return Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

QVariant PaletteModel::data(const QModelIndex &index, int role) const {
    int row = index.row();
    uint color;

    if (spritePal)
        color = tempSprPalettes[palette][row];
    else
        color = tempPalettes[row][palette];

    switch (role) {
    case Qt::BackgroundRole:
        return QBrush(QColor(nesPalette[color]));

    case Qt::SizeHintRole:
        return QSize(32, 32);

    case Qt::DisplayRole:
        return QString::number(color, 16).rightJustified(2, QLatin1Char('0')).toUpper();
    }

    return QVariant();

}

bool PaletteModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::EditRole) {
        int row = index.row();
        bool ok;
        QString text = value.toString();
        uint val = text.toUInt(&ok, 16);

        if (ok && val < 0x40) {
            if (spritePal)
                tempSprPalettes[palette][row] = val;
            else
                tempPalettes[row][palette] = val;

            emit dataChanged(index, index);
        }

        return ok;
    }

    return false;
}

void PaletteModel::setPalette(bool spritePal, uint palette) {
    beginResetModel();
    this->spritePal = spritePal;
    this->palette = palette;
    endResetModel();
}

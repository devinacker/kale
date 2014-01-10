/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#include "coursewindow.h"
#include "ui_coursewindow.h"

#include <QSpinBox>

#include "level.h"
#include "romfile.h"

// TODO: spin this own to its own file if it's ever used elsewhere
class HexSpinBox: public QSpinBox {
public:
    HexSpinBox(QWidget *parent = 0, uint digits = 0):
        QSpinBox(parent) {
        this->digits = digits;
    }

protected:
    int valueFromText(const QString &text) const {
        return text.toInt(0, 16);
    }

    QString textFromValue(int val) const {
        return QString::number(val, 16).rightJustified(this->digits, QLatin1Char('0')).toUpper();
    }

    QValidator::State validate(QString &input, int &pos) const {
        bool ok;
        int res = input.toInt(&ok, 16);
        if (ok && res <= this->maximum() && res >= this->minimum())
            return QValidator::Acceptable;

        return QValidator::Invalid;
    }

private:
    uint digits;
};

CourseWindow::CourseWindow(QWidget *parent) :
    QDialog(parent, Qt::CustomizeWindowHint
            | Qt::WindowTitleHint
            | Qt::WindowCloseButtonHint
            | Qt::MSWindowsFixedSizeDialogHint
           ),
    ui(new Ui::CourseWindow),
    spinBox(new HexSpinBox(this, 3))
{
    ui->setupUi(this);

    spinBox->setMaximum(numLevels - 1);
    ui->horizontalLayout->addWidget(spinBox);
    // prevent window resizing
    this->layout()->setSizeConstraint(QLayout::SetFixedSize);
}

CourseWindow::~CourseWindow()
{
    delete ui;
    delete spinBox;
}

// TODO: subclass spinbox and show value in hex
int CourseWindow::select(uint level) {
    spinBox->setValue(level);

    if (this->exec())
        return spinBox->value();

    return -1;
}

#include "hexspinbox.h"
#include "stuff.h"

HexSpinBox::HexSpinBox(QWidget *parent, uint digits = 0):
    QSpinBox(parent) {

    this->digits = digits;
}

int HexSpinBox::valueFromText(const QString &text) const {
    return text.toInt(0, 16);
}

QString HexSpinBox::textFromValue(int val) const {
    return hexFormat(val, this->digits);
}

QValidator::State HexSpinBox::validate(QString &input, int& /* pos */) const {
    bool ok;
    int res = input.toInt(&ok, 16);
    if ((ok && res <= this->maximum() && res >= this->minimum())
            || input.isEmpty())
        return QValidator::Acceptable;

    return QValidator::Invalid;
}

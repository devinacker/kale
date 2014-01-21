#include "hexspinbox.h"

HexSpinBox::HexSpinBox(QWidget *parent = 0, uint digits = 0):
    QSpinBox(parent) {

    this->digits = digits;
}

int HexSpinBox::valueFromText(const QString &text) const {
    return text.toInt(0, 16);
}

QString HexSpinBox::textFromValue(int val) const {
    return QString::number(val, 16).rightJustified(this->digits, QLatin1Char('0')).toUpper();
}

QValidator::State HexSpinBox::validate(QString &input, int &pos) const {
    bool ok;
    int res = input.toInt(&ok, 16);
    if (ok && res <= this->maximum() && res >= this->minimum())
        return QValidator::Acceptable;

    return QValidator::Invalid;
}

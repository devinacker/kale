#ifndef HEXSPINBOX_H
#define HEXSPINBOX_H
#include <QSpinBox>

class HexSpinBox: public QSpinBox {
public:
    HexSpinBox(QWidget *parent, uint digits);

protected:
    int valueFromText(const QString &text) const;
    QString textFromValue(int val) const;
    QValidator::State validate(QString &input, int &pos) const;

private:
    uint digits;
};

#endif // HEXSPINBOX_H

/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#include "coursewindow.h"
#include "ui_coursewindow.h"

#include "level.h"
#include "romfile.h"
#include "hexspinbox.h"

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

    spinBox->setMaximum(NUM_LEVELS - 1);
    ui->horizontalLayout->addWidget(spinBox);
    // prevent window resizing
    this->layout()->setSizeConstraint(QLayout::SetFixedSize);
}

CourseWindow::~CourseWindow()
{
    delete ui;
    delete spinBox;
}

int CourseWindow::select(uint level) {
    spinBox->setValue(level);

    if (this->exec())
        return spinBox->value();

    return -1;
}

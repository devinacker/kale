/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#ifndef COURSEWINDOW_H
#define COURSEWINDOW_H

#include <QDialog>
#include "level.h"
#include "romfile.h"
#include "hexspinbox.h"

namespace Ui {
class CourseWindow;
}

class CourseWindow : public QDialog
{
    Q_OBJECT
    
public:
    explicit CourseWindow(QWidget *parent = 0);
    ~CourseWindow();

    int select(uint level);
    
private:
    Ui::CourseWindow *ui;
    HexSpinBox *spinBox;
};

#endif // COURSEWINDOW_H

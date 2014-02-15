#include <algorithm>
#include "level.h"
#include "stuff.h"

#include "spriteeditwindow.h"
#include "ui_spriteeditwindow.h"

SpriteEditWindow::SpriteEditWindow(QWidget *parent, sprite_t *sprite) :
    QDialog(parent, Qt::CustomizeWindowHint
            | Qt::WindowTitleHint
            | Qt::WindowCloseButtonHint
            | Qt::MSWindowsFixedSizeDialogHint),
    ui(new Ui::SpriteEditWindow)
{
    ui->setupUi(this);

    this->sprite = sprite;

    for (StringMap::const_iterator i = spriteTypes.begin(); i != spriteTypes.end(); i++) {
            ui->comboBox->addItem(i->second, i->first);
    }

    ui->comboBox->setCurrentIndex(std::distance(spriteTypes.begin(),
                                                spriteTypes.find(sprite->type)));

}

SpriteEditWindow::~SpriteEditWindow()
{
    delete ui;
}

void SpriteEditWindow::accept() {
    this->sprite->type = ui->comboBox->itemData(ui->comboBox->currentIndex()).toUInt();

    QDialog::accept();
}

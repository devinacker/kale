#include "mapcleareditwindow.h"
#include "ui_mapcleareditwindow.h"

MapClearEditWindow::MapClearEditWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapClearEditWindow),
    model(new MapClearModel(this, &rects)),
    level(0)
{
    ui->setupUi(this);
    ui->tableView->setModel(model);

    // set combobox stuff
    ui->comboBox->addItem("Start", 0);

    for (uint i = 0; i < 7; i++) {
        ui->comboBox->addItem(QString("Level %1 Clear").arg(i + 1), i + 1);
        ui->comboBox->addItem(QString("Level %1 Switch").arg(i + 1), i + 8);
    }

    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setLevelIndex(int)));
}

MapClearEditWindow::~MapClearEditWindow()
{
    delete ui;
    delete model;
}

int MapClearEditWindow::exec() {
    ui->comboBox->setCurrentIndex(0);
    this->setLevelIndex(0);

    return QDialog::exec();
}

void MapClearEditWindow::setLevel(uint level) {
    this->level = level;
    model->setLevel(level);
}

void MapClearEditWindow::setLevelIndex(int index) {
    model->setLevelIndex(ui->comboBox->itemData(index).toUInt());
    emit clearRectsChanged(&rects);
}

void MapClearEditWindow::accept() {
    // TODO: update data

    emit clearRectsChanged(NULL);
    QDialog::accept();
}

void MapClearEditWindow::reject() {

    emit clearRectsChanged(NULL);
    QDialog::reject();
}

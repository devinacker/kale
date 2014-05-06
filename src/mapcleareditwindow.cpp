#include "mapcleareditwindow.h"
#include "ui_mapcleareditwindow.h"
#include "mapclear.h"

MapClearEditWindow::MapClearEditWindow(QWidget *parent) :
    QDialog(parent, Qt::CustomizeWindowHint
            | Qt::WindowTitleHint
            | Qt::WindowCloseButtonHint),
    ui(new Ui::MapClearEditWindow),
    model(new MapClearModel(this)),
    delegate(new MapClearDelegate(this)),
    level(0)
{
    ui->setupUi(this);
    ui->tableView->setModel(model);
    ui->tableView->setItemDelegate(delegate);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // set combobox stuff
    ui->comboBox->addItem("Start", 0);

    for (uint i = 0; i < 7; i++) {
        ui->comboBox->addItem(QString("Level %1 Clear").arg(i + 1), i + 1);
        ui->comboBox->addItem(QString("Level %1 Switch").arg(i + 1), i + 8);
    }

    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setLevelIndex(int)));
    connect(ui->tableView->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(updateRects()));

    connect(ui->button_Add, SIGNAL(clicked()),
            this, SLOT(addRect()));
    connect(ui->button_Del, SIGNAL(clicked()),
            this, SLOT(deleteRects()));
    connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(selectionChanged()));
}

MapClearEditWindow::~MapClearEditWindow()
{
    delete ui;
    delete model;
    delete delegate;
}

int MapClearEditWindow::exec() {
    // get current rect count
    rectCount = 0;
    for (uint i = 0; i < 7; i++)
        for (uint j = 0; j < 0x10; j++)
            rectCount += mapClearData[i][j].size();

    ui->comboBox->setCurrentIndex(0);
    this->setLevelIndex(0);
    ui->button_Del->setEnabled(false);

    return QDialog::exec();
}

void MapClearEditWindow::setLevel(uint level) {
    this->level = level;

    for (uint i = 0; i < 0x10; i++)
        this->rects[i] = mapClearData[level][i];
}

void MapClearEditWindow::setLevelIndex(int index) {
    levelIndex = ui->comboBox->itemData(index).toUInt();
    model->setRects(&rects[levelIndex]);
    updateRects();
}

void MapClearEditWindow::updateRects() {
    emit clearRectsChanged(&rects[levelIndex]);

    ui->label_Rects->setText(QString("%1 of %2 rects")
                             .arg(rectCount).arg(MAX_CLEAR_RECTS));
    ui->button_Add->setEnabled(rectCount < MAX_CLEAR_RECTS);
}

void MapClearEditWindow::addRect() {
    if (rectCount < MAX_CLEAR_RECTS && model->insertRow(0)) {
        rectCount++;
        updateRects();
    }
}

void MapClearEditWindow::deleteRects() {
    QModelIndexList list;

    // selection groups? what's that?
    while (list = ui->tableView->selectionModel()->selectedRows(), list.size()) {
        if (model->removeRow(list[0].row()))
            rectCount--;
    }

    updateRects();
}

void MapClearEditWindow::selectionChanged() {
    ui->button_Del->setEnabled(ui->tableView->selectionModel()->selectedRows().size() > 0);
}

void MapClearEditWindow::accept() {
    for (uint i = 0; i < 0x10; i++)
        mapClearData[this->level][i] = this->rects[i];

    emit clearRectsChanged(NULL);
    QDialog::accept();
}

void MapClearEditWindow::reject() {

    emit clearRectsChanged(NULL);
    QDialog::reject();
}

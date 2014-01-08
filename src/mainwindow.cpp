/*
  mainwindow.cpp

  It's the main window. Surprise!
  This class also handles opening/closing ROM files and contains the loaded levels and course
  data, which is passed to the other windows, as well as loading/saving individual course files.

  This code is released under the terms of the MIT license.
  See COPYING.txt for details.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>

#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include "romfile.h"
#include "level.h"
#include "version.h"

#ifdef _WIN32
#include <windows.h>
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings(new QSettings("settings.ini", QSettings::IniFormat, this)),

    fileOpen(false),
    unsaved(false),
    saving(false),
    level(0),

    levelLabel(new QLabel()),
    scene(new MapScene(this, &currentLevel))
{
    ui->setupUi(this);

    for (int i = 0; i < 224; i++)
        levels[i] = NULL;

    currentLevel.header.screensH = 0;
    currentLevel.header.screensV = 0;
    currentLevel.modifiedRecently = false;

    fileName   = settings->value("MainWindow/fileName", "").toString();

    ui->graphicsView->setScene(scene);
    // enable mouse tracking for graphics view
    ui->graphicsView->setMouseTracking(true);

    // remove margins around map view and other stuff
    this->centralWidget()->layout()->setContentsMargins(0,0,0,0);

    setupSignals();
    setupActions();
    getSettings();
    setOpenFileActions(false);
    updateTitle();

#ifdef _WIN32
    // lousy attempt to get the right-sized icons used by Windows, since Qt will only
    // resize one icon in the icon resource (and not the one i want it to use for the
    // titlebar, anyway), at least in 4.7 (and 4.8?)
    // let's assume the small icon is already handled by Qt and so we only need to
    // care about the big one (For alt-tabbing, etc.)
    // icon resource 0 is defined in windows.rc
    SendMessage((HWND)this->winId(),
                WM_SETICON, ICON_BIG,
                (LPARAM)LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(0)));
#endif
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
    delete levelLabel;
    delete scene;
    delete settings;
}

void MainWindow::setupSignals() {
    // file menu
    QObject::connect(ui->action_Open_ROM, SIGNAL(triggered()),
                     this, SLOT(openFile()));
    QObject::connect(ui->action_Save_ROM, SIGNAL(triggered()),
                     this, SLOT(saveFile()));
    QObject::connect(ui->action_Save_ROM_As, SIGNAL(triggered()),
                     this, SLOT(saveFileAs()));
    QObject::connect(ui->action_Close_ROM, SIGNAL(triggered()),
                     this, SLOT(closeFile()));

    QObject::connect(ui->action_Exit, SIGNAL(triggered()),
                     this, SLOT(close()));

    // edit menu
    QObject::connect(ui->action_Undo, SIGNAL(triggered()),
                     scene, SLOT(undo()));
    QObject::connect(ui->action_Redo, SIGNAL(triggered()),
                     scene, SLOT(redo()));

    QObject::connect(ui->action_Cut, SIGNAL(triggered()),
                     scene, SLOT(cut()));
    QObject::connect(ui->action_Copy, SIGNAL(triggered()),
                     scene, SLOT(copy()));
    QObject::connect(ui->action_Paste, SIGNAL(triggered()),
                     scene, SLOT(paste()));
    QObject::connect(ui->action_Delete, SIGNAL(triggered()),
                     scene, SLOT(deleteTiles()));
    QObject::connect(ui->action_Edit_Tiles, SIGNAL(triggered()),
                     scene, SLOT(editTiles()));

    QObject::connect(scene, SIGNAL(edited()),
                     this, SLOT(setUnsaved()));
    QObject::connect(scene, SIGNAL(edited()),
                     this, SLOT(setUndoRedoActions()));

    // level menu
    QObject::connect(ui->action_Save_Level, SIGNAL(triggered()),
                     this, SLOT(saveCurrentLevel()));
    /* TODO: add this feature to MapScene
    QObject::connect(ui->action_Save_Level_to_Image, SIGNAL(triggered()),
                     previewWin, SLOT(savePreview()));
    */

    QObject::connect(ui->action_Load_Course_from_File, SIGNAL(triggered()),
                     this, SLOT(loadCourseFromFile()));
    QObject::connect(ui->action_Save_Course_to_File, SIGNAL(triggered()),
                     this, SLOT(saveCourseToFile()));

    QObject::connect(ui->action_Level_Properties, SIGNAL(triggered()),
                     this, SLOT(levelProperties()));

    QObject::connect(ui->action_Select_Course, SIGNAL(triggered()),
                     this, SLOT(selectCourse()));
    QObject::connect(ui->action_Previous_Level, SIGNAL(triggered()),
                     this, SLOT(prevLevel()));
    QObject::connect(ui->action_Next_Level, SIGNAL(triggered()),
                     this, SLOT(nextLevel()));

    // help menu
    QObject::connect(ui->action_Contents, SIGNAL(triggered()),
                     this, SLOT(showHelp()));
    QObject::connect(ui->action_About, SIGNAL(triggered()),
                     this, SLOT(showAbout()));


    // debug menu
    QObject::connect(ui->action_Dump_Level, SIGNAL(triggered()),
                     this, SLOT(dumpLevel()));

    // other window-related stuff
    // receive status bar messages from scene
    QObject::connect(scene, SIGNAL(statusMessage(QString)),
                     ui->statusBar, SLOT(showMessage(QString)));
}

void MainWindow::setupActions() {
    // from file menu
    ui->toolBar->addAction(ui->action_Open_ROM);
    ui->toolBar->addAction(ui->action_Save_ROM);
    ui->toolBar->addSeparator();

    // from edit menu
    ui->toolBar->addAction(ui->action_Undo);
    ui->toolBar->addAction(ui->action_Redo);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->action_Edit_Tiles);
    ui->toolBar->addAction(ui->action_Level_Properties);
    ui->toolBar->addSeparator();

    // from level menu
    ui->toolBar->addAction(ui->action_Select_Course);
    ui->toolBar->addAction(ui->action_Previous_Level);
    ui->toolBar->addAction(ui->action_Next_Level);
    ui->toolBar->addWidget(levelLabel);
}

void MainWindow::getSettings() {
    // set up windows positions/dimensions
    if (settings->contains("MainWindow/geometry"))
        this      ->setGeometry(settings->value("MainWindow/geometry").toRect());
    if (settings->value("MainWindow/maximized", false).toBool())
        this      ->showMaximized();

    // display friendly message
    status(tr("Welcome to KALE, version %1.")
           .arg(INFO_VERS));

#ifdef QT_NO_DEBUG
    ui->menuDebug->menuAction()->setVisible(settings->value("MainWindow/debug", false).toBool());
#endif
}

void MainWindow::saveSettings() {
    // save window settings
    settings->setValue("MainWindow/fileName", fileName);
    settings->setValue("MainWindow/maximized", this->isMaximized());
    if (!this->isMaximized())
        settings->setValue("MainWindow/geometry", this->geometry());
}

/*
  friendly status bar function
*/
void MainWindow::status(const QString& msg) {
    ui->statusBar->showMessage(msg);
}

void MainWindow::setUnsaved() {
    unsaved = true;
}

/*
  actions that are disabled when a file is not open
*/
void MainWindow::setOpenFileActions(bool val) {
    ui->action_Dump_Level         ->setEnabled(val);
    ui->action_Select_Course      ->setEnabled(val);
    ui->action_Save_Level_to_Image->setEnabled(val);
    setEditActions(val);
    setLevelChangeActions(val);
    // setEditActions may disable this
    ui->action_Open_ROM->setEnabled(true);
}

/*
  actions that are disabled while saving the file
*/
void MainWindow::setEditActions(bool val) {
    setUndoRedoActions(val);
    ui->action_Cut             ->setEnabled(val);
    ui->action_Copy            ->setEnabled(val);
    ui->action_Paste           ->setEnabled(val);
    ui->action_Delete          ->setEnabled(val);
    ui->action_Close_ROM       ->setEnabled(val);
    ui->action_Open_ROM        ->setEnabled(val);
    ui->action_Save_ROM        ->setEnabled(val);
    ui->action_Save_ROM_As     ->setEnabled(val);
    ui->action_Save_Level      ->setEnabled(val);
    ui->action_Edit_Tiles      ->setEnabled(val);
    ui->action_Level_Properties->setEnabled(val);
    ui->action_Load_Course_from_File->setEnabled(val);
    ui->action_Save_Course_to_File->setEnabled(val);
}

/*
 *actions that depend on the state of the undo stack
 */
void MainWindow::setUndoRedoActions(bool val) {
    ui->action_Undo->setEnabled(val && scene->canUndo());
    ui->action_Redo->setEnabled(val && scene->canRedo());
}

/*
  actions that depend on the current level number
*/
void MainWindow::setLevelChangeActions(bool val) {
    ui->action_Previous_Level ->setEnabled(val && level > 0);
    ui->action_Next_Level     ->setEnabled(val && level < numLevels - 1);
}

/*
  put the current file name in the title bar
*/
void MainWindow::updateTitle() {
    if (fileOpen)
        this->setWindowTitle(QString("%1 - %2").arg(INFO_TITLE)
                             .arg(QString(fileName).split("/").last()));
    else
        this->setWindowTitle(INFO_TITLE);
}

/*
  main window close
*/
void MainWindow::closeEvent(QCloseEvent *event) {
    if (!saving && closeFile() != -1)
        event->accept();
    else
        event->ignore();
}

/*
  File menu item slots
*/
void MainWindow::openFile() {

    // open file dialog
    QString newFileName = QFileDialog::getOpenFileName(this,
                                 tr("Open ROM"),
                                 fileName,
                                 tr("NES ROM images (*.nes);;All files (*.*)"));

    if (!newFileName.isNull() && !closeFile()) {
        status(tr("Opening file %1").arg(newFileName));

        // open file
        rom.setFileName(newFileName);
        if (rom.openROM(QIODevice::ReadOnly)) {

            fileName = newFileName;
            unsaved  = false;

            fileOpen = true;

            for (int i = 0; i < numLevels; i++) {
                levels[i] = loadLevel(rom, i);

                // if the user aborted level load, give up and close the ROM
                if (!levels[i]) {
                    closeFile();
                    return;
                }
            }

            int ver = rom.getVersion();

            // show first level
            setLevel(0);
            setOpenFileActions(true);
            updateTitle();

            rom.close();

        } else {
            // if file open fails, display an error
            QMessageBox::warning(this,
                                 tr("Error"),
                                 tr("Unable to open %1.")
                                 .arg(newFileName),
                                 QMessageBox::Ok);
        }
    }
}

void MainWindow::saveFile() {
    if (!fileOpen || checkSaveLevel() == QMessageBox::Cancel)
        return;

    // If there is a problem opening the original file for saving
    // (i.e. it was moved or deleted), let the user select a different one
    while (!QFile::exists(fileName)
           || (QFile::exists(fileName) && !rom.openROM(QIODevice::ReadWrite))) {
        QMessageBox::critical(this, tr("Save File"),
                              tr("Unable to open\n%1\nfor saving. Please select a different ROM.")
                              .arg(fileName),
                              QMessageBox::Ok);

        QString newFileName = QFileDialog::getSaveFileName(this,
                                     tr("Save ROM"),
                                     fileName,
                                     tr("NES ROM images (*.nes);;All files (*.*)"));

        // if the user pressed cancel, then don't save after all
        if (newFileName.isNull())
            return;
        // otherwise try again
        else {
            fileName = newFileName;
            rom.setFileName(fileName);
        }
    }

    status(tr("Saving to file ") + fileName);

    // disable saving/editing while already saving
    setEditActions(false);
    saving = true;

    // save levels to ROM
    /*
    int addr = newDataAddress[rom.getVersion()];

    for (int i = 0; i < numLevels[game]; i++) {
        if (levels[i]->modified) {
            addr = saveLevel(rom, i, levels[i], addr);

            status(tr("Saved level %1-%2").arg((i / 8) + 1).arg((i % 8) + 1));
            QCoreApplication::processEvents();
        }
    }

    // Pad the current ROM bank to 32kb to make sure it is
    // mapped correctly
    int spaceLeft = BANK_SIZE - (addr % BANK_SIZE);

    if (spaceLeft)
        rom.writeByte(addr + spaceLeft - 1, 0);

    */
    status(tr("Saved %1").arg(fileName));
    updateTitle();

    unsaved = false;
    rom.close();

    // re-enable editing
    setEditActions(true);
    saving = false;
}

void MainWindow::saveFileAs() {
    // get a new save location
    QString newFileName = QFileDialog::getSaveFileName(this,
                                 tr("Save ROM"),
                                 fileName,
                                 tr("NES ROM images (*.nes);;All files (*.*)"));

    if (newFileName.isNull() == false) {
        // copy old file onto new file
        if (QFile::exists(newFileName) && !QFile::remove(newFileName)) {
            QMessageBox::critical(this, tr("Save File As"),
                                  tr("Unable to update destination file.\n\nMake sure it is not open in another program, and then try again."),
                                  QMessageBox::Ok);
            return;
        }
        QFile::copy(fileName, newFileName);

        // set file name to new one, then save file
        fileName = newFileName;
        rom.setFileName(fileName);
        saveFile();
    }
}

/*
  Close the currently open file, prompting the user to save changes
  if necessary.
  Return values:
    -1: user cancels file close (file remains open)
     0: file closed successfully (or was already closed)
*/
int MainWindow::closeFile() {
    if (!fileOpen)
        return 0;

    if (checkSaveROM() == QMessageBox::Cancel)
        return -1;

    // deallocate all level data
    for (uint i = 0; i < numLevels; i++) {
        if (levels[i])
            free(levels[i]);

        levels[i] = NULL;
    }

    // clear level displays
    currentLevel.header.screensH = 0;
    currentLevel.header.screensV = 0;
    currentLevel.modifiedRecently = false;

    scene->cancelSelection();
    scene->refresh();
    scene->clearStack();

    levelLabel->setText("");
    setOpenFileActions(false);
    fileOpen = false;
    updateTitle();

    return 0;
}

/*
  Level menu item slots
*/
void MainWindow::loadCourseFromFile() {
    if (!fileOpen) return;
    //...
}

void MainWindow::saveCourseToFile() {
    if (!fileOpen || checkSaveLevel() == QMessageBox::Cancel) return;
    //...
}

void MainWindow::levelProperties() {
    if (currentLevel.header.screensH == 0) return;

    /*
    PropertiesWindow win(this);
    win.startEdit(&currentLevel,
                   &background[course % 8],
                   &palette[course],
                   &waterPalette[course]);

    // update 2D and 3D displays
    scene->refresh();
    */
}

void MainWindow::selectCourse() {
    /*
    CourseWindow win(this);

    int newLevel = win.select(level, rom.getGame());
    if (newLevel != level)
        setLevel(newLevel);
    */
}

void MainWindow::prevLevel() {
    if (level) setLevel(level - 1);
}
void MainWindow::nextLevel() {
    if (level < numLevels) setLevel(level + 1);
}

/*
  Help menu item slots
*/
void MainWindow::showHelp() const {
    /*
    QDesktopServices::openUrl(QUrl(QCoreApplication::applicationDirPath()
                                   + "/docs/index.htm"));
    */
}

void MainWindow::showAbout() {
    QMessageBox::information(this,
                             tr("About"),
                             tr("%1 version %2\nby Devin Acker (Revenant)")
                             .arg(INFO_TITLE).arg(INFO_VERS),
                             QMessageBox::Ok,
                             QMessageBox::Ok);
}

/*
  Display a particular level in the window
  Called when loading a ROM and also changing levels.
*/

void MainWindow::setLevel(uint level) {
    // loadLevel only returns null so don't do this yet
    //qDebug("MainWindow::setLevel not implemented");
    //return;

    if (level > numLevels || !fileOpen)
        return;

    // save changes to the level?
    if (checkSaveLevel() == QMessageBox::Cancel) return;

    this->level = level;
    currentLevel = *(levels[level]);

    // update button enabled states
    setLevelChangeActions(true);

    // set up the graphics view
    scene->cancelSelection();
    scene->refresh();
    scene->clearStack();
    setUndoRedoActions(false);
    ui->graphicsView->update();

    // display the level name in the toolbar label
    levelLabel->setText(QString(" Level %1")
                        .arg(level));
}

void MainWindow::saveCurrentLevel() {
    if (!fileOpen || currentLevel.modifiedRecently == false)
        return;

    scene->setClean();
    currentLevel.modifiedRecently = false;
    unsaved = true;

    *(levels[level]) = currentLevel;

    status(tr("Level saved."));
}

/*
  Prompts the user to save or not save the level.
  If "yes" is selected, the level will be saved.
  (note: no changes will be made to the ROM until the ROM itself is saved)
  Returns the button pressed by the user.
*/
QMessageBox::StandardButton MainWindow::checkSaveLevel() {
    // if the level has not been recently modified, don't do anything
    if (!fileOpen || currentLevel.modifiedRecently == false)
        return QMessageBox::No;

    QMessageBox::StandardButton button =
            QMessageBox::question(this, tr("Save Level"),
                                  tr("Save changes to current level?"),
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    // yes = save current level to group of all levels
    if (button == QMessageBox::Yes) {
        saveCurrentLevel();
    }

    return button;
}

/*
  Prompts the user to save or not save the entire ROM.
  Called when exiting the program or closing the ROM.
  If "yes" is selected, the ROM will be saved.
 */
QMessageBox::StandardButton MainWindow::checkSaveROM() {
    // if the ROM has not been recently modified, don't do anything
    if (!unsaved || !fileOpen)
        return QMessageBox::No;

    QMessageBox::StandardButton button =
            QMessageBox::question(this, tr("Save ROM"),
                                  tr("Save changes to ") + fileName + tr("?"),
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    // yes = save current level to group of all levels
    if (button == QMessageBox::Yes) {
        saveFile();
    }

    return button;
}

void MainWindow::dumpLevel() const {

}

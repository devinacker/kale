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

#include <cstdio>
#include <cstdlib>

#include "romfile.h"
#include "level.h"
#include "tileset.h"
#include "graphics.h"
#include "mapclear.h"
#include "coursewindow.h"
#include "version.h"

#ifdef _WIN32
#include <windows.h>
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    levelLabel(new QLabel()),
    selectGroup(new QActionGroup(this)),

    settings(new QSettings("settings.ini", QSettings::IniFormat, this)),

    fileOpen(false),
    unsaved(false),
    saving(false),
    level(0),

    scene(new MapScene(this, &currentLevel)),
    propWindow(new PropertiesWindow(this, scene->getPixmap())),
    clearWindow(new MapClearEditWindow(this)),
    tilesetWindow(new TilesetEditWindow(this)),
    paletteWindow(new PaletteEditWindow(this))
{
    ui->setupUi(this);
    selectGroup->addAction(ui->action_Select_Tiles);
    selectGroup->addAction(ui->action_Select_Sprites);
    selectGroup->addAction(ui->action_Select_Exits);
    selectGroup->setExclusive(true);
    ui->action_Select_Tiles->setChecked(true);
    enableSelectTiles(true);

    for (int i = 0; i < NUM_LEVELS; i++)
        levels[i] = NULL;

    currentLevel.header.screensH = 0;
    currentLevel.header.screensV = 0;
    currentLevel.modifiedRecently = false;

    fileName   = settings->value("MainWindow/fileName", "").toString();

    ui->graphicsView->setScene(scene);
    // enable mouse tracking for graphics view
    ui->graphicsView->setMouseTracking(true);
    ui->graphicsView->setBackgroundRole(QPalette::Mid);

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
    delete selectGroup;
    delete scene;
    delete settings;
    delete propWindow;
    delete clearWindow;
    delete tilesetWindow;
    delete paletteWindow;
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
                     scene, SLOT(deleteStuff()));
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
    /*
    QObject::connect(ui->action_Load_Course_from_File, SIGNAL(triggered()),
                     this, SLOT(loadCourseFromFile()));
    QObject::connect(ui->action_Save_Course_to_File, SIGNAL(triggered()),
                     this, SLOT(saveCourseToFile()));
    */
    QObject::connect(ui->action_Select_Tiles, SIGNAL(toggled(bool)),
                     this, SLOT(enableSelectTiles(bool)));
    QObject::connect(ui->action_Select_Sprites, SIGNAL(toggled(bool)),
                     this, SLOT(enableSelectSprites(bool)));
    QObject::connect(ui->action_Select_Exits, SIGNAL(toggled(bool)),
                     this, SLOT(enableSelectExits(bool)));

    QObject::connect(ui->action_Level_Properties, SIGNAL(triggered()),
                     this, SLOT(levelProperties()));
    QObject::connect(ui->action_Edit_Map_Clear_Data, SIGNAL(triggered()),
                     this, SLOT(editMapClearData()));
    QObject::connect(ui->action_Edit_Tilesets, SIGNAL(triggered()),
                     this, SLOT(editTilesets()));
    QObject::connect(ui->action_Edit_Palettes, SIGNAL(triggered()),
                     this, SLOT(editPalettes()));

    QObject::connect(ui->action_Select_Level, SIGNAL(triggered()),
                     this, SLOT(selectLevel()));
    QObject::connect(ui->action_Previous_Level, SIGNAL(triggered()),
                     this, SLOT(prevLevel()));
    QObject::connect(ui->action_Next_Level, SIGNAL(triggered()),
                     this, SLOT(nextLevel()));

    // view menu
    QObject::connect(ui->action_Double_Size, SIGNAL(toggled(bool)),
                     this, SLOT(setDoubleSize(bool)));
    QObject::connect(ui->action_Show_Screen_Boundaries, SIGNAL(toggled(bool)),
                     scene, SLOT(setShowBounds(bool)));
    QObject::connect(ui->action_See_Through_Breakable_Tiles, SIGNAL(toggled(bool)),
                     scene, SLOT(setSeeThrough(bool)));

    // help menu
    QObject::connect(ui->action_Contents, SIGNAL(triggered()),
                     this, SLOT(showHelp()));
    QObject::connect(ui->action_About, SIGNAL(triggered()),
                     this, SLOT(showAbout()));

    // other window-related stuff
    // receive status bar messages from scene
    QObject::connect(scene, SIGNAL(statusMessage(QString)),
                     ui->statusBar, SLOT(showMessage(QString)));

    // apply changes immediately from properties window
    QObject::connect(propWindow, SIGNAL(changed()),
                     scene, SLOT(refresh()));
    QObject::connect(propWindow, SIGNAL(speedChanged(int)),
                     scene, SLOT(setAnimSpeed(int)));
    QObject::connect(propWindow, SIGNAL(speedChanged(int)),
                     scene, SLOT(update()));

    // update map when tileset changes are applied
    QObject::connect(tilesetWindow, SIGNAL(changed()),
                     scene, SLOT(refresh()));

    // update map when palette changes are applied
    QObject::connect(paletteWindow, SIGNAL(changed()),
                     scene, SLOT(refresh()));

    // display map clear rects when editing them
    QObject::connect(clearWindow, SIGNAL(clearRectsChanged(const std::vector<QRect>*)),
                     scene, SLOT(setClearRects(const std::vector<QRect>*)));
}

void MainWindow::setupActions() {
    // from file menu
    ui->toolBar->addAction(ui->action_Open_ROM);
    ui->toolBar->addAction(ui->action_Save_ROM);
    ui->toolBar->addSeparator();

    // from edit/level menu etc.
    ui->toolBar->addAction(ui->action_Undo);
    ui->toolBar->addAction(ui->action_Redo);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->action_Select_Tiles);
    ui->toolBar->addAction(ui->action_Select_Sprites);
    ui->toolBar->addAction(ui->action_Select_Exits);
    ui->toolBar->addAction(ui->action_Edit_Tiles);
    ui->toolBar->addAction(ui->action_Level_Properties);
    ui->toolBar->addSeparator();

    ui->toolBar->addAction(ui->action_Edit_Map_Clear_Data);
    ui->toolBar->addAction(ui->action_Edit_Tilesets);
    ui->toolBar->addAction(ui->action_Edit_Palettes);
    ui->toolBar->addSeparator();

    // from view menu
    ui->toolBar->addAction(ui->action_Double_Size);
    ui->toolBar->addAction(ui->action_Show_Screen_Boundaries);
    ui->toolBar->addAction(ui->action_See_Through_Breakable_Tiles);
    ui->toolBar->addSeparator();

    // from level menu
    ui->toolBar->addAction(ui->action_Select_Level);
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
    ui->action_Select_Level       ->setEnabled(val);
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
    ui->action_Select_Tiles    ->setEnabled(val);
    ui->action_Select_Exits    ->setEnabled(val);
    ui->action_Select_Sprites  ->setEnabled(val);
    ui->action_Level_Properties->setEnabled(val);
    ui->action_Load_Course_from_File->setEnabled(val);
    ui->action_Save_Course_to_File->setEnabled(val);

    ui->action_Edit_Tilesets   ->setEnabled(val);
    ui->action_Edit_Palettes   ->setEnabled(val);
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
    ui->action_Next_Level     ->setEnabled(val && level < NUM_LEVELS - 1);
    // map clear data only available for overworlds (000-006)
    ui->action_Edit_Map_Clear_Data->setEnabled(val && level < 7);
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

            for (uint i = 0; i < NUM_LEVELS; i++) {
                levels[i] = loadLevel(rom, i);

                // if the user aborted level load, give up and close the ROM
                if (!levels[i]) {
                    closeFile();
                    return;
                }
            }

            loadCHRBanks(rom);
            loadTilesets(rom);

            // get information about progressively revealing the overworld
            for (uint i = 0; i < 7; i++)
                loadMapClearData(rom, i, levels[i]->header.screensH);

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

    std::list<DataChunk> chunks;
    romaddr_t nextAddr = {0x00, 0x0A00};
    const uint lastBank = 0x12;
    // calculated from amount of space between first door and the tile subtraction table
    const uint maxExits = 0x203;

    // check number of total exits and panic if there are too many
    uint numExits = 0;
    for (uint i = 0; i < NUM_LEVELS; i++) {
        numExits += levels[i]->exits.size();
    }
    if (numExits > maxExits) {
        QMessageBox::critical(this, tr("Error saving file"),
                              tr("Not enough available exits in ROM (%1 exits available, %2 exits used).")
                              .arg(maxExits).arg(numExits),
                              QMessageBox::Ok);

        rom.close();

        // re-enable editing
        setEditActions(true);
        saving = false;

        return;
    }

    // 0x12 banks available, first one has the first 0xA00 bytes used by palettes)
    const uint freeSpace = (BANK_SIZE * lastBank) - nextAddr.addr;
    uint usedSpace = 0;

    // pack level and sprite data
    for (uint i = 0; i < NUM_LEVELS; i++) {
        chunks.push_back(packLevel(levels[i], i));
        usedSpace += chunks.back().size;

        chunks.push_back(packSprites(levels[i], i));
        usedSpace += chunks.back().size;

        QCoreApplication::processEvents();
    }
    // pack tilesets
    for (uint i = 0; i < NUM_TILESETS; i++) {
        chunks.push_back(packTileset(i));
        usedSpace += chunks.back().size;

        QCoreApplication::processEvents();
    }
    // dummy-ish entry representing all three CHR bank tables consecutively
    // (they must be stored in the same PRG bank, and the uncompressed data is already
    //  stored elsewhere)
    chunks.push_back(DataChunk(NULL, 0x300, DataChunk::banks, 0));
    usedSpace += chunks.back().size;

    // panic if there's too much space
    if (usedSpace > freeSpace) {
        QMessageBox::critical(this, tr("Error saving file"),
                              tr("Not enough free space in ROM (%1 bytes available, %2 bytes used).")
                              .arg(freeSpace).arg(usedSpace),
                              QMessageBox::Ok);

        rom.close();

        // re-enable editing
        setEditActions(true);
        saving = false;

        return;
    }

    // sort packed chunks
    chunks.sort();

    // panic if something is bigger than it can/should be
    if (chunks.back().size > BANK_SIZE) {
        DataChunk& chunk = chunks.back();

        QMessageBox::critical(this, tr("Error saving file"),
                              tr("Something exceeded 0x2000 bytes somehow (type %1, num %2, size %3).")
                              .arg(chunk.type).arg(chunk.num).arg(chunk.size),
                              QMessageBox::Ok);

        rom.close();

        // re-enable editing
        setEditActions(true);
        saving = false;

        return;
    }

    while (chunks.size()) {
        // find biggest chunk that will fit in the current ROM bank
        uint space = BANK_SIZE - (nextAddr.addr % BANK_SIZE);

        for (std::list<DataChunk>::reverse_iterator i = chunks.rbegin(); i != chunks.rend(); i++) {
            if (space >= i->size) {
                DataChunk& chunk = *i;

                switch (chunk.type) {
                case DataChunk::level:
                    // save level data
                    saveLevel(rom, chunk, levels[chunk.num], nextAddr);
                    break;
                case DataChunk::enemy:
                    // save enemy/sprite data
                    saveSprites(rom, chunk, nextAddr);
                    break;
                case DataChunk::tileset:
                    // save tileset data
                    saveTileset(rom, chunk, nextAddr);
                    break;
                case DataChunk::banks:
                    saveBankTables(rom, nextAddr);
                    break;
                }

                nextAddr.addr += chunk.size;
                // will the smallest available next chunk still fit in this bank?
                if (space < chunk.size + chunks.front().size) {
                    nextAddr.bank++;
                    nextAddr.addr = 0;
                }

                chunks.erase((++i).base());
                break;
            }
        }

        QCoreApplication::processEvents();
    }

    // save all level exits (in level order instead of by size so pointers can be
    // calculated correctly)
    for (uint i = 0; i < NUM_LEVELS; i++) {
        saveExits(rom, levels[i], i);
    }

    // save map clear data for overworlds
    for (uint i = 0; i < 7; i++) {
        saveMapClearData(rom, levels[i], i);
    }

    // save palettes
    savePalettes(rom);

    status(tr("Saved %1").arg(fileName));

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
        updateTitle();
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
    for (uint i = 0; i < NUM_LEVELS; i++) {
        delete levels[i];
        levels[i] = NULL;
    }

    freeCHRBanks();

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

void MainWindow::enableSelectTiles(bool on) {
    scene->enableSelectTiles(on);
    scene->update();
    ui->graphicsView->setDragMode(QGraphicsView::NoDrag);
}

void MainWindow::enableSelectSprites(bool on) {
    scene->enableSelectSprites(on);
    if (on)
        ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
}

void MainWindow::enableSelectExits(bool on) {
    scene->enableSelectExits(on);
    if (on)
        ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
}

void MainWindow::levelProperties() {
    if (currentLevel.header.screensH == 0) return;

    propWindow->startEdit(&currentLevel);
}

void MainWindow::editMapClearData() {
    if (level >= 7) return;

    clearWindow->setLevel(level);
    if (clearWindow->exec())
        setUnsaved();
}

void MainWindow::editTilesets() {
    tilesetWindow->startEdit(&currentLevel);
}

void MainWindow::editPalettes() {
    paletteWindow->startEdit(currentLevel.header.tilePal,
                             currentLevel.header.sprPal);
}

void MainWindow::selectLevel() {
    CourseWindow win(this);

    int newLevel = win.select(level);
    if (newLevel != -1)
        setLevel(newLevel);
}

void MainWindow::prevLevel() {
    if (level) setLevel(level - 1);
}
void MainWindow::nextLevel() {
    if (level < NUM_LEVELS) setLevel(level + 1);
}

/*
 *Change the graphics view scale factor
 */
void MainWindow::setDoubleSize(bool on) {
    ui->graphicsView->resetTransform();
    if (on)
        ui->graphicsView->scale(2.0, 2.0);
}

/*
  Help menu item slots
*/
void MainWindow::showHelp() const {
    QDesktopServices::openUrl(QUrl(QCoreApplication::applicationDirPath()
                                   + "/docs/index.htm"));
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
    if (level > NUM_LEVELS || !fileOpen)
        return;

    // save changes to the level?
    if (checkSaveLevel() == QMessageBox::Cancel) return;

    this->level = level;
    currentLevel = *(levels[level]);

    // make deep copies of sprite and exit stuff
    currentLevel.exits.clear();
    currentLevel.sprites.clear();
    for (std::list<exit_t*>::const_iterator i = levels[level]->exits.begin();
         i != levels[level]->exits.end(); i++) {

        exit_t *exit = new exit_t;
        *exit = *(*i);
        currentLevel.exits.push_back(exit);
    }
    for (std::list<sprite_t*>::const_iterator i = levels[level]->sprites.begin();
         i != levels[level]->sprites.end(); i++) {

        sprite_t *sprite = new sprite_t;
        *sprite = *(*i);
        currentLevel.sprites.push_back(sprite);
    }

    // update button enabled states
    setLevelChangeActions(true);

    // set up the graphics view
    scene->cancelSelection();
    scene->refresh();
    scene->clearStack();
    setUndoRedoActions(false);
    ui->graphicsView->update();

    // display the room number in the toolbar label
    levelLabel->setText(QString("  Room ")
                      + QString::number(level, 16)
                        .rightJustified(3, QLatin1Char('0')).toUpper());
}

void MainWindow::saveCurrentLevel() {
    if (!fileOpen)
        return;

    scene->setClean();
    currentLevel.modifiedRecently = false;
    unsaved = true;

    leveldata_t *thisLevel = levels[level];

    // delete old level's sprites & exits
    for (std::list<exit_t*>::const_iterator i = thisLevel->exits.begin();
         i != thisLevel->exits.end(); i++) {

        delete *i;
    }
    for (std::list<sprite_t*>::const_iterator i = thisLevel->sprites.begin();
         i != thisLevel->sprites.end(); i++) {

        delete *i;
    }

    *thisLevel = currentLevel;

    // deep copy new sprites and exits
    thisLevel->exits.clear();
    thisLevel->sprites.clear();
    for (std::list<exit_t*>::const_iterator i = currentLevel.exits.begin();
         i != currentLevel.exits.end(); i++) {

        exit_t *exit = new exit_t;
        *exit = *(*i);
        thisLevel->exits.push_back(exit);
    }
    for (std::list<sprite_t*>::const_iterator i = currentLevel.sprites.begin();
         i != currentLevel.sprites.end(); i++) {

        sprite_t *sprite = new sprite_t;
        *sprite = *(*i);
        thisLevel->sprites.push_back(sprite);
    }

    status(tr("Room saved."));
}

/*
  Prompts the user to save or not save the level.
  If "yes" is selected, the level will be saved.
  (note: no changes will be made to the ROM until the ROM itself is saved)
  Returns the button pressed by the user.
*/
QMessageBox::StandardButton MainWindow::checkSaveLevel() {
    // if the level has not been recently modified, don't do anything
    if (!fileOpen || !currentLevel.header.screensH)
        return QMessageBox::No;

    // TODO: i'm temporarily modifying this so that "unsaved" levels get saved anyway
    // since sprite/exit changes don't create a dirty state yet
    if (!currentLevel.modifiedRecently) {
        saveCurrentLevel();
        return QMessageBox::Yes;
    }

    QMessageBox::StandardButton button =
            QMessageBox::question(this, tr("Save Room"),
                                  tr("Save changes to current room?"),
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
                                  tr("Save changes to %1?").arg(fileName),
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    // yes = save current level to group of all levels
    if (button == QMessageBox::Yes) {
        saveFile();
    }

    return button;
}

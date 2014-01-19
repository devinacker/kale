/*
    This code is released under the terms of the MIT license.
    See COPYING.txt for details.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QActionGroup>
#include <QSettings>

#include "romfile.h"
#include "mapscene.h"
#include "level.h"
#include "propertieswindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected slots:
    // file menu
    void openFile();
    void saveFile();
    void saveFileAs();
    int  closeFile();

    void setUnsaved();

    // level menu
    void loadCourseFromFile();
    void saveCourseToFile();

    void saveCurrentLevel();

    void enableSelectTiles(bool);
    void enableSelectSprites(bool);
    void enableSelectExits(bool);

    void levelProperties();

    void selectLevel();
    void prevLevel();
    void nextLevel();

    // help menu
    void showHelp() const;
    void showAbout();

    // display text on the statusbar
    void status(const QString &msg);

    // debug menu crap
    void dumpLevel() const;
    
    // toolbar updates
    void setOpenFileActions(bool val);
    void setEditActions(bool val);
    void setUndoRedoActions(bool val = true);
    void setLevelChangeActions(bool val);

protected:
    void closeEvent(QCloseEvent *);

private:
    Ui::MainWindow *ui;
    QLabel *levelLabel;
    QActionGroup *selectGroup;

    QSettings *settings;

    // Information about the currently open file
    QString fileName;
    ROMFile rom;
    bool    fileOpen, unsaved, saving;

    // The level data
    uint         level;
    leveldata_t* levels[NUM_LEVELS];
    leveldata_t  currentLevel;

    // renderin stuff
    MapScene *scene;
    PropertiesWindow *propWindow;

    // various funcs
    void setupSignals();
    void setupActions();
    void getSettings();
    void saveSettings();
    void updateTitle();
    void setLevel(uint);
    QMessageBox::StandardButton checkSaveLevel();
    QMessageBox::StandardButton checkSaveROM();
};

#endif // MAINWINDOW_H

#ifndef PATCHES_H
#define PATCHES_H

#include <QStringList>
#include "romfile.h"

class QWidget;

enum {
    versionUS0,
    versionUS1,
    versionJP,
    versionEU,
    versionCA,
    versionDE
};

extern const QStringList extraDataPatches;

int getGameVersion(QWidget *parent = 0);

bool applyPatch(ROMFile& file, QString path);

#endif // PATCHES_H

#include "patches.h"

#include <QFile>
#include <QDialog>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QtEndian>

#define PATCH_PATH ":patches/"
#define PATCH_EXTRA PATCH_PATH "maphacks-"

const QStringList extraDataPatches = {
    PATCH_EXTRA "us0.ips",
    PATCH_EXTRA "us1.ips",
    PATCH_EXTRA "jp.ips",
    PATCH_EXTRA "eu.ips",
    PATCH_EXTRA "fc.ips",
    PATCH_EXTRA "de.ips",
};

int getGameVersion(QWidget *parent) {
    QDialog dlg(parent);
    dlg.setWindowTitle(QWidget::tr("Select ROM Version"));
    QVBoxLayout layout;
    dlg.setLayout(&layout);
    QComboBox combo;
    combo.addItems({
       QWidget::tr("United States PRG0"),
       QWidget::tr("United States PRG1"),
       QWidget::tr("Japan"),
       QWidget::tr("Europe"),
       QWidget::tr("Canada"),
       QWidget::tr("Germany")
    });

    layout.addWidget(&combo);
    layout.addWidget(new QLabel(QWidget::tr("Be sure to select the correct region for your ROM.\n"
                                   "Applying a patch cannot be undone!")));
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout.addWidget(&buttons);
    QObject::connect(&buttons, SIGNAL(accepted()), &dlg, SLOT(accept()));
    QObject::connect(&buttons, SIGNAL(rejected()), &dlg, SLOT(reject()));

    if (dlg.exec()) {
        return combo.currentIndex();
    }

    return -1;
}

bool applyPatch(ROMFile &file, QString path) {
    QFile patch(path);
    if (!patch.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(0, QWidget::tr("Error Applying Patch"),
                              QWidget::tr("Unable to open %1.").arg(path),
                              QMessageBox::Ok);
        return false;
    }
    // ideally the target file should already be open (since the main window
    // will probably be made to handle any errors and allows the user to select
    // a new file, leaving it open afterwards), but we'll handle it quickly here
    // as well
    if (!file.open(QIODevice::ReadWrite)) {
        QMessageBox::critical(0, QWidget::tr("Error Applying Patch"),
                              QWidget::tr("Unable to open %1.").arg(file.fileName()),
                              QMessageBox::Ok);
        return false;
    }

    patch.seek(5);

    uchar buf[4] = {0};
    uint32_t addr;
    uint16_t size;
    char rle;
    while (true) {
        buf[0] = 0;
        patch.read((char*)buf+1, 3);
        addr = qFromBigEndian<uint32_t>(buf);
        if (addr == 0x454F46) break; // "EOF"
        file.seek(addr);

        patch.read((char*)buf, 2);
        size = qFromBigEndian<uint16_t>(buf);

        if (!size) { // RLE patch entry
            patch.read((char*)buf, 2);
            size = qFromBigEndian<uint16_t>(buf);
            patch.read(&rle, 1);

            for (uint i = 0; i < size; i++) {
                file.write(&rle, 1);
            }
        } else {
            file.write(patch.read(size));
        }
    }

    QMessageBox::information(0, QWidget::tr("Apply Patch"),
                          QWidget::tr("Patch applied successfully!").arg(file.fileName()),
                          QMessageBox::Ok);

    return true;
}

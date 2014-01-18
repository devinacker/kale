#include "graphics.h"
#include "romfile.h"
#include <QPixmap>

QImage *banks = 0;
uint numBanks = 0;

void loadCHRBanks(ROMFile& rom) {
    if (banks)
        delete[] banks;

    numBanks = rom.getNumCHRBanks();
    banks = new QImage[numBanks];
    for (uint i = 0; i < numBanks; i++)
        banks[i] = rom.readCHRBank(i);
}

// get single CHR bank with applied palette
// (TODO: the palette thing)
QImage getCHRBank(uint bank, uint pal) {
    if (banks) {
        QImage newBank(banks[bank % numBanks]);
        // test: save bank to image
        newBank.save(QString("bank%1.png").arg(bank));
        // apply palette here eventually
        return newBank;
    } else return QImage();
}

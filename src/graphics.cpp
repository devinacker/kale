#include "graphics.h"
#include "romfile.h"
#include <QPixmap>

QImage *banks = 0;
uint numBanks = 0;
uint8_t bankTable[3][256];

const romaddr_t bankLists = {0x05, 0xB418};

void loadCHRBanks(ROMFile& rom) {
    if (banks)
        delete[] banks;

    numBanks = rom.getNumCHRBanks();
    banks = new QImage[numBanks];
    for (uint i = 0; i < numBanks; i++)
        banks[i] = rom.readCHRBank(i);

    rom.readData(bankLists,       256, &bankTable[0][0]);
    rom.readData(bankLists + 256, 256, &bankTable[1][0]);
    rom.readData(bankLists + 512, 256, &bankTable[2][0]);
}

// get single CHR bank with applied palette
// (TODO: the palette thing)
QImage getCHRBank(uint bank, uint pal) {
    if (banks) {
        QImage newBank(banks[bank % numBanks]);

        // apply palette here eventually
        return newBank;
    } else return QImage();
}
